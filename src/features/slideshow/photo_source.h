//
// Created by Razor on 15/10/2026.
//

#pragma once
#include <SDL3/SDL_surface.h>
#include <cstddef>

// Only called from the slideshow's loader thread, no locking needed
class PhotoSource
{
public:
    virtual ~PhotoSource() = default;

    // nullptr on failure, caller owns the surface
    virtual SDL_Surface* loadNext() = 0;

    virtual size_t count() const = 0;
};
