#pragma once

#include "http_client.h"
#include "api_key_provider.h"
#include "dtos.h"
#include <cstddef>
#include <expected>
#include <memory>
#include <string_view>

namespace DPP
{
    class FredClient
    {
    public:
        FredClient(std::shared_ptr<IHttpClient> http, std::shared_ptr<IApiKeyProvider> keys);

        std::expected<FredSeriesResponse, std::string>
        getSeriesObservations(const std::string& seriesId,
                              const std::string& startDate = "",
                              const std::string& endDate = "") const;

    private:
        // Lengths of the quoted JSON keys searched by find("\"date\"") / find("\"value\"")
        static constexpr std::size_t kQuotedDateKeyLen = sizeof("\"date\"") - 1;
        static constexpr std::size_t kQuotedValueKeyLen = sizeof("\"value\"") - 1;

        static std::expected<FredSeriesResponse, std::string>
        parseFredSeriesResponseFromJson(std::string_view json, const std::string& seriesId);

        static bool tryParseJsonDouble(std::string_view s, double& out);

        std::shared_ptr<IHttpClient> m_http;
        std::shared_ptr<IApiKeyProvider> m_keys;
    };
}
