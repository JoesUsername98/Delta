#pragma once

#include <string>
#include <expected>
#include <map>

namespace DPP
{
    using HttpResponse = std::expected<std::string, std::string>;

    struct IHttpClient
    {
        virtual ~IHttpClient() = default;
        virtual HttpResponse get(const std::string& url,
                                 const std::map<std::string, std::string>& params = {}) const = 0;
    };
}
