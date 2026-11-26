//
// Created by Razor on 14/10/2026.
//

#pragma once
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <atomic>
#include <memory>
#include <thread>

#include "features/slideshow/photo_source.h"

class Slideshow
{
public:
    Slideshow() = default;
    ~Slideshow();

    // Owns a thread and textures, no copies
    Slideshow(const Slideshow&) = delete;
    Slideshow& operator=(const Slideshow&) = delete;

    bool start(std::unique_ptr<PhotoSource> source);

    // Main thread only (creates textures)
    void update(SDL_Renderer* renderer);

    void draw(SDL_Renderer* renderer) const;

    // Safe to call twice
    void stop();

private:
    void startLoading();

    std::unique_ptr<PhotoSource> source;  // Loader thread only
    Uint64 nextSwitchMs = 0;  // 0 = show asap

    // Wait before retrying when a whole lap fails
    size_t failedInARow = 0;
    Uint64 retryAtMs = 0;     // 0 = not waiting

    SDL_Texture* texture = nullptr;

    // Loader writes these, then sets loadDone
    std::thread loaderThread;
    std::atomic<bool> loadDone{false};
    SDL_Surface* loadedSurface = nullptr;
    size_t photoCount = 0;
};
