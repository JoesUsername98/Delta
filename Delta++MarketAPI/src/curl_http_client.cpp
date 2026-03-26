#include "Delta++MarketAPI/curl_http_client.h"
#include <curl/curl.h>
#include <sstream>

// SCARY FILE WITH RAW POINTERS!
namespace DPP
{
    static size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata)
    {
        auto* buf = static_cast<std::string*>(userdata);
        buf->append(ptr, size * nmemb);
        return size * nmemb;
    }

    CurlHttpClient::CurlHttpClient()
    {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }

    CurlHttpClient::~CurlHttpClient()
    {
        curl_global_cleanup();
    }

    HttpResponse CurlHttpClient::get(const std::string& url,
                                      const std::map<std::string, std::string>& params) const
    {
        CURL* curl = curl_easy_init();
        if (!curl)
            return std::unexpected("Failed to initialise CURL");

        // Build query string
        std::string fullUrl = url;
        if (!params.empty())
        {
            fullUrl += "?";
            bool first = true;
            for (const auto& [k, v] : params)
            {
                if (!first) fullUrl += "&";
                char* ek = curl_easy_escape(curl, k.c_str(), 0);
                char* ev = curl_easy_escape(curl, v.c_str(), 0);
                fullUrl += std::string(ek) + "=" + std::string(ev);
                curl_free(ek);
                curl_free(ev);
                first = false;
            }
        }

        std::string body;
        curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            std::string err = curl_easy_strerror(res);
            curl_easy_cleanup(curl);
            return std::unexpected("CURL error: " + err);
        }

        curl_easy_cleanup(curl);
        return body;
    }
}
