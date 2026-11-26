//
// Created by Razor on 15/10/2026.
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
    void refresh();

    ImmichClient client;
    std::string albumId;
    PlayQueue queue;          // Asset ids
    bool listedOnce = false;
};
