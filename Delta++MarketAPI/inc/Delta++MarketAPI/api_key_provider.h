#pragma once

#include <string>
#include <expected>

namespace DPP
{
    struct IApiKeyProvider
    {
        virtual ~IApiKeyProvider() = default;
        virtual std::expected<std::string, std::string> getKey(const std::string& name) const = 0;
    };

    // Reads keys from environment variables
    class EnvApiKeyProvider : public IApiKeyProvider
    {
    public:
        std::expected<std::string, std::string> getKey(const std::string& name) const override;
    };
}
