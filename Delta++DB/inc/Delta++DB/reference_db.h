#pragma once

#include <Delta++MarketAPI/dtos.h>

#include <expected>
#include <filesystem>
#include <string>
#include <vector>

namespace DPP::DB
{
#if defined(DPP_REFERENCE_DB_PATH)
    inline std::filesystem::path defaultReferenceDbPath()
    {
        return std::filesystem::path(DPP_REFERENCE_DB_PATH);
    }
#else
    inline std::filesystem::path defaultReferenceDbPath()
    {
        return std::filesystem::path("data") / "referenceDB.sqlite";
    }
#endif

    std::expected<void, std::string> upsertOptionsContracts(const std::filesystem::path& dbPath,
                                                            const std::vector<OptionsContractRow>& rows);

    /// Returns rows with only the columns stored in the reference DB populated (others default / nullopt).
    std::expected<std::vector<OptionsContractRow>, std::string> queryAllOptionsContracts(
        const std::filesystem::path& dbPath);
}
