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
    // Without timeouts a dead server would hang quitting
    const cpr::ConnectTimeout ConnectTimeout{5'000};  // ms
    const cpr::Timeout RequestTimeout{20'000};        // ms

    constexpr int PageSize = 1000;
    constexpr int MaxPages = 100;   // Safety stop

    // Fallback instead of throwing on missing or bad fields
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
    if (response.error)  // Network error, no HTTP status
        reason = response.error.message;
    else if (response.status_code != 200)  // 401 = bad key, 400 = bad album
        reason = "HTTP " + std::to_string(response.status_code);

    if (reason.empty())
    {
        if (!lastError.empty())
            LogInfo("Server reachable again", LogCat::Immich);
        lastError.clear();
        return true;
    }

    // Compare only the reason so the same outage logs once
    if (reason != lastError)
        LogErrorf(LogCat::Immich, "%s failed: %s", what.c_str(), reason.c_str());
    lastError = reason;
    return false;
}

ImmichClient::ImmichClient(std::string baseUrl, std::string apiKey)
    : baseUrl(std::move(baseUrl)), apiKey(std::move(apiKey))
{
    // Trim trailing slashes
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

    for (int page = 1; page <= MaxPages; page++)
    {
        nlohmann::json body;
        body["albumIds"] = nlohmann::json::array({albumId});
        body["type"] = "IMAGE";  // Skip videos
        body["page"] = page;
        body["size"] = PageSize;

        const cpr::Response response =
            cpr::Post(url, headers, cpr::Body{body.dump()}, ConnectTimeout, RequestTimeout);
        if (!checkResponse(response, "Album search"))
            return std::nullopt;

        // Don't throw on bad JSON
        const nlohmann::json parsed = nlohmann::json::parse(response.text, nullptr, false);
        if (parsed.is_discarded())
        {
            LogError("Album search: response is not valid JSON", LogCat::Immich);
            return std::nullopt;
        }

        // { "assets": { "items": [...], "nextPage": "2" | null } }
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

            if (!asset.id.empty())
                result.push_back(std::move(asset));
        }

        const auto nextIt = assetsIt->find("nextPage");
        if (nextIt == assetsIt->end() || nextIt->is_null())
            return result;
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

    return std::move(response.text);
}
