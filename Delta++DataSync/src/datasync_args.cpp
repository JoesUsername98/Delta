#include <Delta++DataSync/datasync_args.h>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <string_view>

namespace DPP::DataSync
{
    namespace
    {
        std::string trim(std::string s)
        {
            auto notSpace = [](unsigned char c) { return !std::isspace(c); };
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
            s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
            return s;
        }

        std::vector<std::string> splitCommaList(std::string_view s)
        {
            std::vector<std::string> out;
            size_t pos = 0;
            while (pos <= s.size())
            {
                const size_t comma = s.find(',', pos);
                const size_t end = (comma == std::string_view::npos) ? s.size() : comma;
                auto item = trim(std::string(s.substr(pos, end - pos)));
                if (!item.empty())
                    out.push_back(std::move(item));
                if (comma == std::string_view::npos)
                    break;
                pos = comma + 1;
            }
            return out;
        }
    }

    void printUsage()
    {
        std::cout << "Delta++DataSync\n"
                  << "  --job <name>                  e.g. option_contract_sync\n"
                  << "  --underlying_ticker SPX,NVDA  (comma-separated; required for option_contract_sync)\n"
                  << "  --db <path>                   (required for option_contract_sync)\n";
    }

    std::optional<ParsedArgs> parseArgs(const int argc, char** argv)
    {
        ParsedArgs a;
        for (int i = 1; i < argc; ++i)
        {
            const std::string_view arg(argv[i]);

            auto takeValue = [&](std::string_view& v) -> bool {
                const auto eq = v.find('=');
                if (eq != std::string_view::npos)
                {
                    v = v.substr(eq + 1);
                    return true;
                }
                if (i + 1 >= argc)
                    return false;
                v = std::string_view(argv[++i]);
                return true;
            };

            if (arg == "--help" || arg == "-h")
            {
                printUsage();
                return std::nullopt;
            }

            if (arg.rfind("--job", 0) == 0)
            {
                std::string_view v = arg;
                if (!takeValue(v))
                {
                    std::cerr << "Missing value for --job\n";
                    return std::nullopt;
                }
                a.job = std::string(v);
                continue;
            }

            if (arg.rfind("--underlying_ticker", 0) == 0)
            {
                std::string_view v = arg;
                if (!takeValue(v))
                {
                    std::cerr << "Missing value for --underlying_ticker\n";
                    return std::nullopt;
                }
                a.underlyingTickers = splitCommaList(v);
                continue;
            }

            if (arg.rfind("--db", 0) == 0)
            {
                std::string_view v = arg;
                if (!takeValue(v))
                {
                    std::cerr << "Missing value for --db\n";
                    return std::nullopt;
                }
                a.dbPath = std::filesystem::path(std::string(v));
                continue;
            }

            std::cerr << "Unknown arg: " << arg << "\n";
            return std::nullopt;
        }

        return a;
    }
}
