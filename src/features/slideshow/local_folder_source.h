//
// Created by Razor on 15/10/2026.
//
// Photos from a folder on disk: every .jpg/.jpeg/.png/.webp file in it, sorted by name.
// The folder is re-read before every photo, so added/removed files are noticed at once.
//

#pragma once
#include <string>

#include "features/slideshow/photo_source.h"
#include "features/slideshow/play_queue.h"

class LocalFolderSource : public PhotoSource
{
public:
    explicit LocalFolderSource(std::string folder);  // folder path must end with '/'

    SDL_Surface* loadNext() override;
    size_t count() const override { return queue.size(); }

private:
    void refresh();  // re-read the folder and sync the queue

    std::string folder;
    PlayQueue queue;          // ids are file names relative to `folder`
    bool listedOnce = false;  // false until the first successful listing
};
