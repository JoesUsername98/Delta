#include <Delta++DB/market_db.h>

#include <sqlite3.h>

#include <expected>
#include <cstring>
#include <cstdio>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <chrono>

namespace DPP::DB::detail
{
    const char* marketSchemaSql();
}

namespace
{
    std::expected<std::chrono::sys_days, std::string> parseYmdToSysDays(std::string_view ymd)
    {
        int y = 0, m = 0, d = 0;
        if (std::sscanf(std::string(ymd).c_str(), "%d-%d-%d", &y, &m, &d) != 3)
            return std::unexpected("Invalid date (expected YYYY-MM-DD): " + std::string(ymd));
        using namespace std::chrono;
        const year_month_day ymdv{year{y}, month{static_cast<unsigned>(m)}, day{static_cast<unsigned>(d)}};
        if (!ymdv.ok())
            return std::unexpected("Invalid calendar date: " + std::string(ymd));
        return sys_days{ymdv};
    }

    std::expected<double, std::string> yearFractionAct365_25(std::string_view fromYmd, std::string_view toYmd)
    {
        const auto a = parseYmdToSysDays(fromYmd);
        if (!a.has_value())
            return std::unexpected(a.error());
        const auto b = parseYmdToSysDays(toYmd);
        if (!b.has_value())
            return std::unexpected(b.error());
        using namespace std::chrono;
        const auto dd = (b.value() - a.value()).count();
        return static_cast<double>(dd) / 365.25;
    }

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
        const int rc = sqlite3_open_v2(pathStr.c_str(), &raw, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
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
        return execSql(db, DPP::DB::detail::marketSchemaSql());
    }

    void bindOptionalDouble(sqlite3_stmt* stmt, int idx, const std::optional<double>& v)
    {
        if (v.has_value())
            sqlite3_bind_double(stmt, idx, *v);
        else
            sqlite3_bind_null(stmt, idx);
    }
}

namespace DPP::DB::Market
{
    std::expected<void, std::string> upsertTreasuryYieldRow(const std::filesystem::path& dbPath,
                                                            const TreasuryYieldRow& row)
    {
        auto dbResult = openDb(dbPath);
        if (!dbResult.has_value())
            return std::unexpected(dbResult.error());
        sqlite3* db = dbResult.value().get();

        auto schemaResult = initSchema(db);
        if (!schemaResult.has_value())
            return schemaResult;

        static constexpr const char* kUpsert = R"SQL(
INSERT INTO treasury_yields (
  date,
  yield_1_month, yield_3_month, yield_6_month,
  yield_1_year, yield_2_year, yield_3_year,
  yield_5_year, yield_7_year, yield_10_year,
  yield_20_year, yield_30_year
) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
ON CONFLICT(date) DO UPDATE SET
  yield_1_month  = excluded.yield_1_month,
  yield_3_month  = excluded.yield_3_month,
  yield_6_month  = excluded.yield_6_month,
  yield_1_year   = excluded.yield_1_year,
  yield_2_year   = excluded.yield_2_year,
  yield_3_year   = excluded.yield_3_year,
  yield_5_year   = excluded.yield_5_year,
  yield_7_year   = excluded.yield_7_year,
  yield_10_year  = excluded.yield_10_year,
  yield_20_year  = excluded.yield_20_year,
  yield_30_year  = excluded.yield_30_year
)SQL";

        sqlite3_stmt* stmtRaw = nullptr;
        int rc = sqlite3_prepare_v2(db, kUpsert, static_cast<int>(std::strlen(kUpsert)), &stmtRaw, nullptr);
        if (rc != SQLITE_OK)
            return std::unexpected(std::string("prepare: ") + sqlite3_errmsg(db));
        Sqlite3StmtPtr stmt(stmtRaw);

        rc = sqlite3_exec(db, "BEGIN IMMEDIATE", nullptr, nullptr, nullptr);
        if (rc != SQLITE_OK)
            return std::unexpected(std::string("BEGIN: ") + sqlite3_errmsg(db));

        sqlite3_bind_text(stmt.get(), 1, row.date.c_str(), static_cast<int>(row.date.size()), SQLITE_TRANSIENT);
        bindOptionalDouble(stmt.get(), 2, row.yield_1_month);
        bindOptionalDouble(stmt.get(), 3, row.yield_3_month);
        bindOptionalDouble(stmt.get(), 4, row.yield_6_month);
        bindOptionalDouble(stmt.get(), 5, row.yield_1_year);
        bindOptionalDouble(stmt.get(), 6, row.yield_2_year);
        bindOptionalDouble(stmt.get(), 7, row.yield_3_year);
        bindOptionalDouble(stmt.get(), 8, row.yield_5_year);
        bindOptionalDouble(stmt.get(), 9, row.yield_7_year);
        bindOptionalDouble(stmt.get(), 10, row.yield_10_year);
        bindOptionalDouble(stmt.get(), 11, row.yield_20_year);
        bindOptionalDouble(stmt.get(), 12, row.yield_30_year);

        rc = sqlite3_step(stmt.get());
        if (rc != SQLITE_DONE)
        {
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            return std::unexpected(std::string("step: ") + sqlite3_errmsg(db));
        }

        rc = sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr);
        if (rc != SQLITE_OK)
            return std::unexpected(std::string("COMMIT: ") + sqlite3_errmsg(db));

        return {};
    }

    std::expected<std::optional<TreasuryYieldRow>, std::string> queryTreasuryYieldRow(
        const std::filesystem::path& dbPath, const std::string_view ymd)
    {
        auto dbResult = openDb(dbPath);
        if (!dbResult.has_value())
            return std::unexpected(dbResult.error());
        sqlite3* db = dbResult.value().get();

        auto schemaResult = initSchema(db);
        if (!schemaResult.has_value())
            return std::unexpected(schemaResult.error());

        static constexpr const char* kSelect = R"SQL(
SELECT
  date,
  yield_1_month, yield_3_month, yield_6_month,
  yield_1_year, yield_2_year, yield_3_year,
  yield_5_year, yield_7_year, yield_10_year,
  yield_20_year, yield_30_year
FROM treasury_yields
WHERE date = ?
)SQL";

        sqlite3_stmt* stmtRaw = nullptr;
        int rc = sqlite3_prepare_v2(db, kSelect, static_cast<int>(std::strlen(kSelect)), &stmtRaw, nullptr);
        if (rc != SQLITE_OK)
            return std::unexpected(std::string("prepare: ") + sqlite3_errmsg(db));
        Sqlite3StmtPtr stmt(stmtRaw);

        const std::string d(ymd);
        sqlite3_bind_text(stmt.get(), 1, d.c_str(), static_cast<int>(d.size()), SQLITE_TRANSIENT);

        rc = sqlite3_step(stmt.get());
        if (rc == SQLITE_DONE)
            return std::optional<TreasuryYieldRow>{};
        if (rc != SQLITE_ROW)
            return std::unexpected(std::string("step: ") + sqlite3_errmsg(db));

        TreasuryYieldRow row;
        row.date = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 0));
        auto readOpt = [&](int idx, std::optional<double>& out) {
            if (sqlite3_column_type(stmt.get(), idx) != SQLITE_NULL)
                out = sqlite3_column_double(stmt.get(), idx);
        };
        readOpt(1, row.yield_1_month);
        readOpt(2, row.yield_3_month);
        readOpt(3, row.yield_6_month);
        readOpt(4, row.yield_1_year);
        readOpt(5, row.yield_2_year);
        readOpt(6, row.yield_3_year);
        readOpt(7, row.yield_5_year);
        readOpt(8, row.yield_7_year);
        readOpt(9, row.yield_10_year);
        readOpt(10, row.yield_20_year);
        readOpt(11, row.yield_30_year);

        return std::optional<TreasuryYieldRow>(std::move(row));
    }

    std::expected<void, std::string> upsertOptionsEodQuotes(const std::filesystem::path& dbPath,
                                                            const std::vector<OptionsEodQuoteRow>& rows)
    {
        if (rows.empty())
            return {};

        auto dbResult = openDb(dbPath);
        if (!dbResult.has_value())
            return std::unexpected(dbResult.error());
        sqlite3* db = dbResult.value().get();

        auto schemaResult = initSchema(db);
        if (!schemaResult.has_value())
            return schemaResult;

        static constexpr const char* kUpsert = R"SQL(
INSERT INTO options_eod_quotes (
  quote_date, expiration_date, strike_price, underlying_ticker, contract_type, bid, ask
) VALUES (?, ?, ?, ?, ?, ?, ?)
ON CONFLICT(quote_date, expiration_date, strike_price, underlying_ticker, contract_type) DO UPDATE SET
  bid = excluded.bid,
  ask = excluded.ask
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

            sqlite3_bind_text(stmt.get(), 1, row.quote_date.c_str(),
                              static_cast<int>(row.quote_date.size()), SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt.get(), 2, row.expiration_date.c_str(),
                              static_cast<int>(row.expiration_date.size()), SQLITE_TRANSIENT);
            sqlite3_bind_double(stmt.get(), 3, row.strike_price);
            sqlite3_bind_text(stmt.get(), 4, row.underlying_ticker.c_str(),
                              static_cast<int>(row.underlying_ticker.size()), SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt.get(), 5, row.contract_type.c_str(),
                              static_cast<int>(row.contract_type.size()), SQLITE_TRANSIENT);
            bindOptionalDouble(stmt.get(), 6, row.bid);
            bindOptionalDouble(stmt.get(), 7, row.ask);

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

    std::expected<std::optional<OptionsEodQuoteRow>, std::string> queryOptionsEodQuote(
        const std::filesystem::path& dbPath,
        const std::string_view quoteDate,
        const std::string_view expirationDate,
        const double strikePrice,
        const std::string_view underlyingTicker,
        const std::string_view contractType)
    {
        auto dbResult = openDb(dbPath);
        if (!dbResult.has_value())
            return std::unexpected(dbResult.error());
        sqlite3* db = dbResult.value().get();

        auto schemaResult = initSchema(db);
        if (!schemaResult.has_value())
            return std::unexpected(schemaResult.error());

        static constexpr const char* kSelect = R"SQL(
SELECT quote_date, expiration_date, strike_price, underlying_ticker, contract_type, bid, ask
FROM options_eod_quotes
WHERE quote_date = ?
  AND expiration_date = ?
  AND strike_price = ?
  AND underlying_ticker = ?
  AND contract_type = ?
)SQL";

        sqlite3_stmt* stmtRaw = nullptr;
        int rc = sqlite3_prepare_v2(db, kSelect, static_cast<int>(std::strlen(kSelect)), &stmtRaw, nullptr);
        if (rc != SQLITE_OK)
            return std::unexpected(std::string("prepare: ") + sqlite3_errmsg(db));
        Sqlite3StmtPtr stmt(stmtRaw);

        const std::string qd(quoteDate);
        const std::string ed(expirationDate);
        const std::string ut(underlyingTicker);
        const std::string ct(contractType);
        sqlite3_bind_text(stmt.get(), 1, qd.c_str(), static_cast<int>(qd.size()), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt.get(), 2, ed.c_str(), static_cast<int>(ed.size()), SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt.get(), 3, strikePrice);
        sqlite3_bind_text(stmt.get(), 4, ut.c_str(), static_cast<int>(ut.size()), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt.get(), 5, ct.c_str(), static_cast<int>(ct.size()), SQLITE_TRANSIENT);

        rc = sqlite3_step(stmt.get());
        if (rc == SQLITE_DONE)
            return std::optional<OptionsEodQuoteRow>{};
        if (rc != SQLITE_ROW)
            return std::unexpected(std::string("step: ") + sqlite3_errmsg(db));

        OptionsEodQuoteRow row;
        row.quote_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 0));
        row.expiration_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 1));
        row.strike_price = sqlite3_column_double(stmt.get(), 2);
        row.underlying_ticker = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 3));
        row.contract_type = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 4));
        if (sqlite3_column_type(stmt.get(), 5) != SQLITE_NULL)
            row.bid = sqlite3_column_double(stmt.get(), 5);
        if (sqlite3_column_type(stmt.get(), 6) != SQLITE_NULL)
            row.ask = sqlite3_column_double(stmt.get(), 6);

        return std::optional<OptionsEodQuoteRow>(std::move(row));
    }

    std::expected<std::vector<std::string>, std::string>
    queryDistinctUnderlyingsForDate(const std::filesystem::path& dbPath, const std::string_view quoteDate)
    {
        auto dbResult = openDb(dbPath);
        if (!dbResult.has_value())
            return std::unexpected(dbResult.error());
        sqlite3* db = dbResult.value().get();

        auto schemaResult = initSchema(db);
        if (!schemaResult.has_value())
            return std::unexpected(schemaResult.error());

        static constexpr const char* kSql = R"SQL(
SELECT DISTINCT underlying_ticker
FROM options_eod_quotes
WHERE quote_date = ?
ORDER BY underlying_ticker
)SQL";

        sqlite3_stmt* stmtRaw = nullptr;
        int rc = sqlite3_prepare_v2(db, kSql, static_cast<int>(std::strlen(kSql)), &stmtRaw, nullptr);
        if (rc != SQLITE_OK)
            return std::unexpected(std::string("prepare: ") + sqlite3_errmsg(db));
        Sqlite3StmtPtr stmt(stmtRaw);

        const std::string qd(quoteDate);
        sqlite3_bind_text(stmt.get(), 1, qd.c_str(), static_cast<int>(qd.size()), SQLITE_TRANSIENT);

        std::vector<std::string> out;
        while ((rc = sqlite3_step(stmt.get())) == SQLITE_ROW)
        {
            const char* s = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 0));
            if (s && *s)
                out.emplace_back(s);
        }
        if (rc != SQLITE_DONE)
            return std::unexpected(std::string("step: ") + sqlite3_errmsg(db));

        return out;
    }

    std::expected<std::optional<double>, std::string>
    queryEquityLast(const std::filesystem::path& dbPath, const std::string_view quoteDate, const std::string_view ticker)
    {
        auto dbResult = openDb(dbPath);
        if (!dbResult.has_value())
            return std::unexpected(dbResult.error());
        sqlite3* db = dbResult.value().get();

        auto schemaResult = initSchema(db);
        if (!schemaResult.has_value())
            return std::unexpected(schemaResult.error());

        static constexpr const char* kSql = R"SQL(
SELECT last
FROM equities
WHERE quote_date = ?
  AND ticker = ?
LIMIT 1
)SQL";

        sqlite3_stmt* stmtRaw = nullptr;
        int rc = sqlite3_prepare_v2(db, kSql, static_cast<int>(std::strlen(kSql)), &stmtRaw, nullptr);
        if (rc != SQLITE_OK)
            return std::unexpected(std::string("prepare: ") + sqlite3_errmsg(db));
        Sqlite3StmtPtr stmt(stmtRaw);

        const std::string qd(quoteDate);
        const std::string tk(ticker);
        sqlite3_bind_text(stmt.get(), 1, qd.c_str(), static_cast<int>(qd.size()), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt.get(), 2, tk.c_str(), static_cast<int>(tk.size()), SQLITE_TRANSIENT);

        rc = sqlite3_step(stmt.get());
        if (rc == SQLITE_DONE)
            return std::optional<double>{};
        if (rc != SQLITE_ROW)
            return std::unexpected(std::string("step: ") + sqlite3_errmsg(db));
        if (sqlite3_column_type(stmt.get(), 0) == SQLITE_NULL)
            return std::optional<double>{};
        return std::optional<double>{sqlite3_column_double(stmt.get(), 0)};
    }

    std::expected<std::vector<CallMidPoint>, std::string>
    queryCallMidsForDateUnderlying(const std::filesystem::path& dbPath,
                                   const std::string_view quoteDate,
                                   const std::string_view underlyingTicker)
    {
        auto dbResult = openDb(dbPath);
        if (!dbResult.has_value())
            return std::unexpected(dbResult.error());
        sqlite3* db = dbResult.value().get();

        auto schemaResult = initSchema(db);
        if (!schemaResult.has_value())
            return std::unexpected(schemaResult.error());

        static constexpr const char* kSql = R"SQL(
SELECT expiration_date, strike_price, bid, ask
FROM options_eod_quotes
WHERE quote_date = ?
  AND underlying_ticker = ?
  AND contract_type = 'call'
  AND bid IS NOT NULL
  AND ask IS NOT NULL
)SQL";

        sqlite3_stmt* stmtRaw = nullptr;
        int rc = sqlite3_prepare_v2(db, kSql, static_cast<int>(std::strlen(kSql)), &stmtRaw, nullptr);
        if (rc != SQLITE_OK)
            return std::unexpected(std::string("prepare: ") + sqlite3_errmsg(db));
        Sqlite3StmtPtr stmt(stmtRaw);

        const std::string qd(quoteDate);
        const std::string ut(underlyingTicker);
        sqlite3_bind_text(stmt.get(), 1, qd.c_str(), static_cast<int>(qd.size()), SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt.get(), 2, ut.c_str(), static_cast<int>(ut.size()), SQLITE_TRANSIENT);

        std::vector<CallMidPoint> out;
        while ((rc = sqlite3_step(stmt.get())) == SQLITE_ROW)
        {
            const char* exp = reinterpret_cast<const char*>(sqlite3_column_text(stmt.get(), 0));
            const double strike = sqlite3_column_double(stmt.get(), 1);
            const double bid = sqlite3_column_double(stmt.get(), 2);
            const double ask = sqlite3_column_double(stmt.get(), 3);

            if (!exp || !*exp)
                continue;

            const auto yf = yearFractionAct365_25(quoteDate, exp);
            if (!yf.has_value())
                continue;
            const double mid = 0.5 * (bid + ask);
            out.push_back({yf.value(), strike, mid});
        }
        if (rc != SQLITE_DONE)
            return std::unexpected(std::string("step: ") + sqlite3_errmsg(db));

        return out;
    }
}

