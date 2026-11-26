//
// Created by Razor on 15/10/2026.
//
// Photos from an Immich album. Before every photo the album is listed again, so
// photos added in Immich are shown next and removed ones disappear from the queue.
// Each photo's preview (~1440 px) is downloaded on demand; nothing is stored on disk.
//

#pragma once
#include <string>

#include "features/slideshow/photo_source.h"
#include "features/slideshow/play_queue.h"
#include "integrations/immich/immich.h"

class ImmichSource : public PhotoSource
{
public:
    ImmichSource(std::string baseUrl, std::string apiKey, std::string albumId);

    SDL_Surface* loadNext() override;
    size_t count() const override { return queue.size(); }

private:
    void refresh();  // list the album and sync the queue

    ImmichClient client;
    std::string albumId;
    PlayQueue queue;          // ids are Immich asset ids
    bool listedOnce = false;  // false until the first successful listing
};
