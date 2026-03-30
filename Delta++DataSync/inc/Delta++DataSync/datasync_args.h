#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace DPP::DataSync
{
    struct ParsedArgs
    {
        std::optional<std::string> job;
        std::vector<std::string> underlyingTickers;
        std::optional<std::filesystem::path> dbPath;
        std::optional<std::string> fromAsof;
        std::optional<std::string> toAsof;
    };

    void printUsage();

    /// Returns nullopt on parse failure or after printing help (caller should exit non-zero).
    std::optional<ParsedArgs> parseArgs(int argc, char** argv);
}
