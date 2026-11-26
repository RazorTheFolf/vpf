//
// Created by Razor on 14/10/2026.
//

#include "image.h"

#include <SDL3_image/SDL_image.h>

#include "core/logger/logger.h"

SDL_Surface* loadImage(const char* imagePath)
{
    SDL_Surface* surface = IMG_Load(imagePath);
    if (!surface)
        LogErrorf(LogCat::Image, "Couldn't load image '%s': %s", imagePath, SDL_GetError());

    return surface;
}

SDL_Surface* decodeImage(const void* data, size_t size, const char* name)
{
    if (!data || size == 0)
    {
        LogErrorf(LogCat::Image, "Couldn't decode '%s': no data", name);
        return nullptr;
    }

    // Wrap the bytes in an SDL stream (no copy), so SDL_image can read them like a file.
    SDL_IOStream* io = SDL_IOFromConstMem(data, size);
    if (!io)
    {
        LogErrorf(LogCat::Image, "Couldn't decode '%s': %s", name, SDL_GetError());
        return nullptr;
    }

    // `true` = SDL_image closes the stream for us, whether decoding succeeds or not.
    SDL_Surface* surface = IMG_Load_IO(io, true);
    if (!surface)
        LogErrorf(LogCat::Image, "Couldn't decode '%s': %s", name, SDL_GetError());

    return surface;
}
