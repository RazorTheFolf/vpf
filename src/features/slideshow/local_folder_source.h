//
// Created by Razor on 15/10/2026.
//

#pragma once
#include <string>

#include "features/slideshow/photo_source.h"
#include "features/slideshow/play_queue.h"

class LocalFolderSource : public PhotoSource
{
public:
    explicit LocalFolderSource(std::string folder);  // Must end with '/'

    SDL_Surface* loadNext() override;
    size_t count() const override { return queue.size(); }

private:
    void refresh();

    std::string folder;
    PlayQueue queue;          // File names
    bool listedOnce = false;
};
