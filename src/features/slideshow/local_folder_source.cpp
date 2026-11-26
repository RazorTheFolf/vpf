//
// Created by Razor on 15/10/2026.
//

#include "local_folder_source.h"

#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_stdinc.h>
#include <algorithm>
#include <utility>
#include <vector>

#include "core/logger/logger.h"
#include "lib/image/image.h"

namespace
{
    bool isPhotoFile(const char* name)
    {
        const char* ext = SDL_strrchr(name, '.');
        if (!ext)
            return false;
        return SDL_strcasecmp(ext, ".jpg") == 0 || SDL_strcasecmp(ext, ".jpeg") == 0 ||
               SDL_strcasecmp(ext, ".png") == 0 || SDL_strcasecmp(ext, ".webp") == 0;
    }
}

LocalFolderSource::LocalFolderSource(std::string folder) : folder(std::move(folder)) {}

void LocalFolderSource::refresh()
{
    int count = 0;
    char** names = SDL_GlobDirectory(folder.c_str(), nullptr, 0, &count);
    if (!names)
    {
        // Folder busy, keep the current queue
        LogErrorf(LogCat::Image, "Can't list '%s': %s", folder.c_str(), SDL_GetError());
        return;
    }

    std::vector<std::string> files;
    for (int i = 0; i < count; i++)
        if (isPhotoFile(names[i]))
            files.emplace_back(names[i]);
    SDL_free(names);

    std::sort(files.begin(), files.end());  // Same order on every OS
    queue.sync(files);

    // Only log the first listing and real changes
    const PlayQueue::Changes changes = queue.lastChanges();
    if (!listedOnce)
        LogInfof(LogCat::Image, "Folder loaded: %zu photos", queue.size());
    else if (changes.added > 0 || changes.removed > 0)
        LogInfof(LogCat::Image, "Folder changed: %zu new, %zu removed (%zu photos)",
                 changes.added, changes.removed, queue.size());
    listedOnce = true;
}

SDL_Surface* LocalFolderSource::loadNext()
{
    refresh();

    const std::string file = queue.next();
    if (file.empty())
        return nullptr;

    return loadImage((folder + file).c_str());
}
