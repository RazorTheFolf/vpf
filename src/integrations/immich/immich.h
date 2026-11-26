//
// Created by Razor on 15/10/2026.
//
// Minimal client for an Immich server's REST API.
//
// Every request is independent (no shared session state). Use the client from one
// thread at a time (the slideshow's loader thread), never from the render loop:
// network requests can take seconds.
//

#pragma once
#include <optional>
#include <string>
#include <vector>

// Forward declaration: says "cpr::Response is a class" without including all of cpr.
// Enough to use it by reference below; only immich.cpp needs the full definition.
namespace cpr
{
    class Response;
}

struct AssetInfo
{
    std::string id;
    std::string originalFileName;
    int width = 0;   // 0 = unknown (Immich may not report it)
    int height = 0;
};

class ImmichClient
{
public:
    // baseUrl: e.g. "http://192.168.1.10:2283" (a trailing slash is fine).
    // Parameters are taken by value and moved in: callers passing temporaries pay no copy.
    ImmichClient(std::string baseUrl, std::string apiKey);

    // Lists ALL the photos (no videos) in an album, following Immich's pagination.
    // Returns std::nullopt if any request fails: a partial list would look like photos
    // were removed. An empty album gives an empty vector, which is different from nullopt.
    std::optional<std::vector<AssetInfo>> fetchAlbumAssetsInfo(const std::string& albumId) const;

    // Downloads the "preview" version of a photo (~1440 px, usually JPEG; WebP if the
    // server's preview format is set to WebP). Returns the raw file bytes, or an empty
    // string on failure. std::string is just a byte container here, not text.
    std::string fetchAsset(const std::string& assetId) const;

private:
    // Returns true for a usable response (HTTP 200). Logs a failure only when its reason
    // differs from the previous one, and logs once when requests work again, so an
    // offline server prints one error instead of one every slide.
    bool checkResponse(const cpr::Response& response, const std::string& what) const;

    std::string baseUrl;
    std::string apiKey;

    // `mutable`: changed inside const methods. It's bookkeeping for the log, not part of
    // what the client "is", so the methods can stay const for their callers.
    mutable std::string lastError;  // reason of the last failure; "" = last request worked
};
