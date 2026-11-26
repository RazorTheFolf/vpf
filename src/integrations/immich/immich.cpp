//
// Created by Razor on 15/10/2026.
//

#include "immich.h"

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <string>
#include <utility>

#include "core/logger/logger.h"

namespace
{
    // Without timeouts cpr waits forever. These requests run on the slideshow's loader
    // thread, and Slideshow::stop() joins that thread: a stalled server would freeze quitting.
    const cpr::ConnectTimeout ConnectTimeout{5'000};  // ms to reach the server
    const cpr::Timeout RequestTimeout{20'000};        // ms for the whole request

    constexpr int PageSize = 1000;  // assets per search page
    constexpr int MaxPages = 100;   // safety stop (100'000 photos) in case nextPage never ends

    // Safe field readers: return a fallback instead of throwing when a field is
    // missing, null, or of the wrong type, so one odd photo can't break the whole album.
    std::string stringOrEmpty(const nlohmann::json& object, const char* key)
    {
        const auto it = object.find(key);
        return (it != object.end() && it->is_string()) ? it->get<std::string>() : std::string{};
    }

    int intOrZero(const nlohmann::json& object, const char* key)
    {
        const auto it = object.find(key);
        return (it != object.end() && it->is_number_integer()) ? it->get<int>() : 0;
    }
}

bool ImmichClient::checkResponse(const cpr::Response& response, const std::string& what) const
{
    std::string reason;
    if (response.error)  // network-level failure (server down, timeout...): no HTTP status
        reason = response.error.message;
    else if (response.status_code != 200)  // 401 = bad API key, 400 = bad album id, ...
        reason = "HTTP " + std::to_string(response.status_code);

    if (reason.empty())
    {
        if (!lastError.empty())
            LogInfo("Server reachable again", LogCat::Immich);
        lastError.clear();
        return true;
    }

    // Compare the reason only, not `what`: with the server down, the album search and
    // the photo download fail for the same reason and should log once, not alternate.
    if (reason != lastError)
        LogErrorf(LogCat::Immich, "%s failed: %s", what.c_str(), reason.c_str());
    lastError = reason;
    return false;
}

ImmichClient::ImmichClient(std::string baseUrl, std::string apiKey)
    : baseUrl(std::move(baseUrl)), apiKey(std::move(apiKey))
{
    // Inside the body, plain `baseUrl` would mean the (now moved-from) parameter,
    // so the member needs `this->`.
    // "http://host:2283/" -> "http://host:2283", so "/api/..." can be appended safely.
    while (!this->baseUrl.empty() && this->baseUrl.back() == '/')
        this->baseUrl.pop_back();
}

std::optional<std::vector<AssetInfo>> ImmichClient::fetchAlbumAssetsInfo(const std::string& albumId) const
{
    std::vector<AssetInfo> result;

    const cpr::Url url{baseUrl + "/api/search/metadata"};
    const cpr::Header headers{
        {"x-api-key", apiKey},
        {"Content-Type", "application/json"},
        {"Accept", "application/json"},
    };

    // Results come one page at a time; keep asking until there is no next page.
    for (int page = 1; page <= MaxPages; page++)
    {
        nlohmann::json body;
        body["albumIds"] = nlohmann::json::array({albumId});
        body["type"] = "IMAGE";  // skip videos
        body["page"] = page;
        body["size"] = PageSize;

        const cpr::Response response =
            cpr::Post(url, headers, cpr::Body{body.dump()}, ConnectTimeout, RequestTimeout);
        if (!checkResponse(response, "Album search"))
            return std::nullopt;

        // `false` = don't throw on invalid JSON; returns a "discarded" value instead.
        const nlohmann::json parsed = nlohmann::json::parse(response.text, nullptr, false);
        if (parsed.is_discarded())
        {
            LogError("Album search: response is not valid JSON", LogCat::Immich);
            return std::nullopt;
        }

        // Expected shape: { "assets": { "items": [ ... ], "nextPage": "2" or null } }
        // find() returns end() when the key is missing (or when the value isn't an object).
        const auto assetsIt = parsed.find("assets");
        const auto itemsIt = assetsIt != parsed.end() ? assetsIt->find("items") : parsed.end();
        if (assetsIt == parsed.end() || itemsIt == assetsIt->end() || !itemsIt->is_array())
        {
            LogError("Album search: unexpected response format", LogCat::Immich);
            return std::nullopt;
        }

        for (const auto& item : *itemsIt)
        {
            if (!item.is_object())
                continue;

            AssetInfo asset;
            asset.id = stringOrEmpty(item, "id");
            asset.originalFileName = stringOrEmpty(item, "originalFileName");
            asset.width = intOrZero(item, "width");
            asset.height = intOrZero(item, "height");

            if (!asset.id.empty())  // without an id we can't download it
                result.push_back(std::move(asset));
        }

        const auto nextIt = assetsIt->find("nextPage");
        if (nextIt == assetsIt->end() || nextIt->is_null())
            return result;  // last page: the list is complete
    }

    LogWarnf(LogCat::Immich, "Album search stopped after %d pages", MaxPages);
    return result;
}

std::string ImmichClient::fetchAsset(const std::string& assetId) const
{
    cpr::Response response = cpr::Get(cpr::Url{baseUrl + "/api/assets/" + assetId + "/thumbnail"},
                                       cpr::Header{{"x-api-key", apiKey}},
                                       cpr::Parameters{{"size", "preview"}},
                                       ConnectTimeout, RequestTimeout);

    if (!checkResponse(response, "Photo download"))
        return {};

    // Move the bytes out of the response instead of copying them (~300 KB per photo).
    return std::move(response.text);
}
