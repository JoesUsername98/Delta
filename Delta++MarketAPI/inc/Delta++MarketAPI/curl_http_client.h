#pragma once

#include "http_client.h"

namespace DPP
{
    class CurlHttpClient : public IHttpClient
    {
    public:
        CurlHttpClient();
        ~CurlHttpClient() override;

        HttpResponse get(const std::string& url,
                         const std::map<std::string, std::string>& params = {}) const override;
    };
}
