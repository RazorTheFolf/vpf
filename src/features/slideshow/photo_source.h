//
// Created by Razor on 15/10/2026.
//
// Where the slideshow's photos come from: a local folder, an Immich album...
// The Slideshow only knows this interface, so adding a new source never changes it.
// Each source decides the order itself (see PlayQueue).
//
// Threading: the Slideshow calls these methods ONLY from its loader thread, one call
// at a time. Implementations may block (disk, network) and need no locking.
//

#pragma once
#include <SDL3/SDL_surface.h>
#include <cstddef>

class PhotoSource
{
public:
    // Sources are deleted through a PhotoSource pointer (the Slideshow's unique_ptr),
    // so the destructor must be virtual for the derived part to be destroyed too.
    virtual ~PhotoSource() = default;

    // Checks the source for added/removed photos, picks the next photo, then loads
    // and decodes it. Returns nullptr if there is no photo or it failed (reason logged).
    // The caller owns the surface.
    virtual SDL_Surface* loadNext() = 0;

    // How many photos the source had at the last loadNext().
    virtual size_t count() const = 0;
};
