#include <Delta++DB/reference_db.h>

#include <sqlite3.h>

#include <cstring>
#include <filesystem>
#include <memory>

namespace DPP::DB::detail
{
const char* referenceSchemaSql();
}

namespace
{
    struct Sqlite3Deleter
    {
        void operator()(sqlite3* p) const
        {
            if (p)
                sqlite3_close(p);
        }
    };

    using Sqlite3Ptr = std::unique_ptr<sqlite3, Sqlite3Deleter>;

    struct Sqlite3StmtDeleter
    {
        void operator()(sqlite3_stmt* p) const
        {
            if (p)
                sqlite3_finalize(p);
        }
    };

    using Sqlite3StmtPtr = std::unique_ptr<sqlite3_stmt, Sqlite3StmtDeleter>;

    std::expected<Sqlite3Ptr, std::string> openDb(const std::filesystem::path& dbPath)
    {
        std::error_code ec;
        const auto parent = dbPath.parent_path();
        if (!parent.empty())
            std::filesystem::create_directories(parent, ec);

        sqlite3* raw = nullptr;
        const std::string pathStr = dbPath.string();
        const int rc =
            sqlite3_open_v2(pathStr.c_str(), &raw, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
        if (rc != SQLITE_OK)
        {
            std::string err = raw ? sqlite3_errmsg(raw) : "sqlite3_open_v2 failed";
            if (raw)
                sqlite3_close(raw);
            return std::unexpected(std::move(err));
        }
        return Sqlite3Ptr(raw);
    }

    std::expected<void, std::string> execSql(sqlite3* db, const char* sql)
    {
        char* errMsg = nullptr;
        const int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK)
        {
            std::string err = errMsg ? errMsg : ("sqlite3_exec failed: " + std::to_string(rc));
            sqlite3_free(errMsg);
            return std::unexpected(std::move(err));
        }
        return {};
    }

    std::expected<void, std::string> initSchema(sqlite3* db)
    {
        return execSql(db, DPP::DB::detail::referenceSchemaSql());
    }
}

namespace DPP::DB
{
    std::expected<void, std::string> upsertOptionsContracts(const std::filesystem::path& dbPath,
                                                            const std::vector<OptionsContractRow>& rows)
    {
        auto dbResult = openDb(dbPath);
        if (!dbResult.has_value())
            return std::unexpected(dbResult.error());
        sqlite3* db = dbResult.value().get();

        auto schemaResult = initSchema(db);
        if (!schemaResult.has_value())
            return schemaResult;

        static constexpr const char* kUpsert = R"SQL(
INSERT INTO options_contracts (
  ticker, underlying_ticker, expiration_date, strike_price, contract_type, exercise_style
) VALUES (?, ?, ?, ?, ?, ?)
ON CONFLICT(ticker) DO UPDATE SET
  underlying_ticker = excluded.underlying_ticker,
  expiration_date = excluded.expiration_date,
  strike_price = excluded.strike_price,
  contract_type = excluded.contract_type,
  exercise_style = excluded.exercise_style
)SQL";

        sqlite3_stmt* stmtRaw = nullptr;
        int rc = sqlite3_prepare_v2(db, kUpsert, static_cast<int>(std::strlen(kUpsert)), &stmtRaw, nullptr);
        if (rc != SQLITE_OK)
            return std::unexpected(std::string("prepare: ") + sqlite3_errmsg(db));
        Sqlite3StmtPtr stmt(stmtRaw);

        rc = sqlite3_exec(db, "BEGIN IMMEDIATE", nullptr, nullptr, nullptr);
        if (rc != SQLITE_OK)
            return std::unexpected(std::string("BEGIN: ") + sqlite3_errmsg(db));

        for (const auto& row : rows)
        {
            sqlite3_reset(stmt.get());
            sqlite3_clear_bindings(stmt.get());

            sqlite3_bind_text(stmt.get(), 1, row.ticker.c_str(), static_cast<int>(row.ticker.size()), SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt.get(), 2, row.underlying_ticker.c_str(),
                              static_cast<int>(row.underlying_ticker.size()), SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt.get(), 3, row.expiration_date.c_str(),
                              static_cast<int>(row.expiration_date.size()), SQLITE_TRANSIENT);

            if (row.strike_price.has_value())
                sqlite3_bind_double(stmt.get(), 4, *row.strike_price);
            else
                sqlite3_bind_null(stmt.get(), 4);

            sqlite3_bind_text(stmt.get(), 5, row.contract_type.c_str(),
                              static_cast<int>(row.contract_type.size()), SQLITE_TRANSIENT);

            if (row.exercise_style.has_value() && !row.exercise_style->empty())
                sqlite3_bind_text(stmt.get(), 6, row.exercise_style->c_str(),
                                  static_cast<int>(row.exercise_style->size()), SQLITE_TRANSIENT);
            else
                sqlite3_bind_null(stmt.get(), 6);

            rc = sqlite3_step(stmt.get());
            if (rc != SQLITE_DONE)
            {
                sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
                return std::unexpected(std::string("step: ") + sqlite3_errmsg(db));
            }
        }

        rc = sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr);
        if (rc != SQLITE_OK)
            return std::unexpected(std::string("COMMIT: ") + sqlite3_errmsg(db));

        return {};
    }

    std::expected<std::vector<OptionsContractRow>, std::string> queryAllOptionsContracts(
        const std::filesystem::path& dbPath)
    {
        auto dbResult = openDb(dbPath);
        if (!dbResult.has_value())
            return std::unexpected(dbResult.error());
        sqlite3* db = dbResult.value().get();

        auto schemaResult = initSchema(db);
        if (!schemaResult.has_value())
            return std::unexpected(schemaResult.error());

        static constexpr const char* kSelect =
            "SELECT ticker, underlying_ticker, expiration_date, strike_price, contract_type, exercise_style "
            "FROM options_contracts ORDER BY ticker";

        sqlite3_stmt* stmtRaw = nullptr;
        int rc = sqlite3_prepare_v2(db, kSelect, static_cast<int>(std::strlen(kSelect)), &stmtRaw, nullptr);
        if (rc != SQLITE_OK)
            return std::unexpected(std::string("prepare: ") + sqlite3_errmsg(db));
        Sqlite3StmtPtr stmt(stmtRaw);

        std::vector<OptionsContractRow> out;
        while ((rc = sqlite3_step(stmt.get())) == SQLITE_ROW)
        {
            OptionsContractRow row;
            row.ticker = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 0));
            row.underlying_ticker = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 1));
            row.expiration_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 2));
            if (sqlite3_column_type(stmt.get(), 3) != SQLITE_NULL)
                row.strike_price = sqlite3_column_double(stmt.get(), 3);
            row.contract_type = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 4));
            if (sqlite3_column_type(stmt.get(), 5) != SQLITE_NULL)
                row.exercise_style = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 5));
            out.push_back(std::move(row));
        }
        if (rc != SQLITE_DONE)
            return std::unexpected(std::string("step: ") + sqlite3_errmsg(db));

        return out;
    }
}
