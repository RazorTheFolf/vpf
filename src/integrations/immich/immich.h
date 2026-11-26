//
// Created by Razor on 15/10/2026.
//

#pragma once
#include <optional>
#include <string>
#include <vector>

namespace cpr
{
    class Response;
}

struct AssetInfo
{
    std::string id;
    std::string originalFileName;
    int width = 0;   // 0 = unknown
    int height = 0;
};

class ImmichClient
{
public:
    ImmichClient(std::string baseUrl, std::string apiKey);

    // nullopt on failure, so a failed request doesn't look like an empty album
    std::optional<std::vector<AssetInfo>> fetchAlbumAssetsInfo(const std::string& albumId) const;

    // Preview size (~1440px), empty on failure
    std::string fetchAsset(const std::string& assetId) const;

private:
    // Only logs when the error changes, so an offline server logs once
    bool checkResponse(const cpr::Response& response, const std::string& what) const;

    std::string baseUrl;
    std::string apiKey;

    mutable std::string lastError;  // "" = last request worked
};
