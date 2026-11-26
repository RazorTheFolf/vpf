//
// Created by Razor on 15/10/2026.
//

#include "immich_source.h"

#include <utility>
#include <vector>

#include "core/logger/logger.h"
#include "lib/image/image.h"

ImmichSource::ImmichSource(std::string baseUrl, std::string apiKey, std::string albumId)
    : client(std::move(baseUrl), std::move(apiKey)), albumId(std::move(albumId))
{
}

void ImmichSource::refresh()
{
    const std::optional<std::vector<AssetInfo>> assets = client.fetchAlbumAssetsInfo(albumId);

    // Request failed, keep the current queue
    if (!assets)
        return;

    std::vector<std::string> ids;
    ids.reserve(assets->size());
    for (const AssetInfo& asset : *assets)
        ids.push_back(asset.id);

    queue.sync(ids);

    // Only log the first listing and real changes
    const PlayQueue::Changes changes = queue.lastChanges();
    if (!listedOnce)
        LogInfof(LogCat::Immich, "Album loaded: %zu photos", queue.size());
    else if (changes.added > 0 || changes.removed > 0)
        LogInfof(LogCat::Immich, "Album changed: %zu new, %zu removed (%zu photos)",
                 changes.added, changes.removed, queue.size());
    listedOnce = true;
}

SDL_Surface* ImmichSource::loadNext()
{
    refresh();

    const std::string id = queue.next();
    if (id.empty())
        return nullptr;

    const std::string bytes = client.fetchAsset(id);
    if (bytes.empty())
        return nullptr;

    return decodeImage(bytes.data(), bytes.size(), id.c_str());
}
