#include "Delta++MarketAPI/api_key_provider.h"
#include <cstdlib>

namespace DPP
{
    std::expected<std::string, std::string> EnvApiKeyProvider::getKey(const std::string& name) const
    {
        const char* val = std::getenv(name.c_str());
        if (!val || !*val)
            return std::unexpected("Environment variable '" + name + "' is not set");
        return std::string(val);
    }
}
