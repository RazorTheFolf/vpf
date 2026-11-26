//
// Created by Razor on 14/10/2026.
//

#include "slideshow.h"

#include <SDL3/SDL_timer.h>
#include <utility>

#include "core/config/config.h"
#include "core/logger/logger.h"
#include "lib/rendering/draw.h"

// How long to wait before retrying when no photo could be loaded
// (server offline, empty album, missing folder...).
static constexpr Uint64 RetryDelayMs = 30'000;

Slideshow::~Slideshow()
{
    stop();
}

bool Slideshow::start(std::unique_ptr<PhotoSource> newSource)
{
    if (!newSource)
    {
        LogError("Slideshow has no photo source", LogCat::App);
        return false;
    }

    // unique_ptr can't be copied, only moved: the slideshow becomes the only owner.
    source = std::move(newSource);
    startLoading();
    return true;
}

void Slideshow::update(SDL_Renderer* renderer)
{
    // Guard clause: most frames there is nothing to do, so leave early.
    if (!loadDone)
        return;

    // The loader finished but produced no photo (the reason was already logged).
    if (!loadedSurface)
    {
        // Already waiting after a lap where everything failed?
        if (retryAtMs != 0)
        {
            if (SDL_GetTicks() < retryAtMs)
                return;  // not yet: do nothing this frame
            retryAtMs = 0;
            startLoading();  // the server may be back, the album may have changed
            return;
        }

        // One bad photo: skip to the next one right away.
        // No photos at all, or a whole lap of bad ones: stop and wait, otherwise we
        // would retry every frame (~60 requests per second) and flood the log.
        failedInARow++;
        if (photoCount == 0 || failedInARow >= photoCount)
        {
            if (photoCount == 0)
                LogWarnf(LogCat::App, "No photos available, retrying in %d s",
                         static_cast<int>(RetryDelayMs / 1000));
            else
                LogWarnf(LogCat::App, "None of the %zu photos could be loaded, retrying in %d s",
                         photoCount, static_cast<int>(RetryDelayMs / 1000));
            failedInARow = 0;
            retryAtMs = SDL_GetTicks() + RetryDelayMs;
            return;
        }

        startLoading();  // skip to the next photo
        return;
    }

    failedInARow = 0;  // a photo loaded fine: the failure streak is over

    if (SDL_GetTicks() < nextSwitchMs)
        return;  // next photo is ready, but the current one still has time on screen

    // Order matters: free the OLD texture before overwriting the pointer,
    // otherwise it leaks (and destroying after overwriting would free the NEW one).
    SDL_DestroyTexture(texture);  // safe on nullptr
    texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    if (!texture)
        LogErrorf(LogCat::Render, "Couldn't create texture: %s", SDL_GetError());

    // The pixels now live on the GPU; the CPU copy is no longer needed.
    SDL_DestroySurface(loadedSurface);
    loadedSurface = nullptr;

    nextSwitchMs = SDL_GetTicks() + static_cast<Uint64>(GetConfig().slideDuration) * 1000;  // seconds -> ms
    startLoading();  // preload the following photo while this one is displayed
}

void Slideshow::draw(SDL_Renderer* renderer) const
{
    if (texture)
        drawFitted(renderer, texture);
    else
        drawCenteredText(renderer, GetConfig().placeholderText.c_str(), GetConfig().placeholderTextScale);
}

void Slideshow::stop()
{
    // Wait for the worker first: it may still be writing loadedSurface.
    if (loaderThread.joinable())
        loaderThread.join();

    // Free and reset to nullptr, so calling stop() twice (quit + destructor) is harmless.
    SDL_DestroySurface(loadedSurface);
    loadedSurface = nullptr;

    SDL_DestroyTexture(texture);
    texture = nullptr;
}

void Slideshow::startLoading()
{
    // A std::thread can only be reused after join(). We only get here once the
    // previous load is done (loadDone was true), so this join returns immediately.
    if (loaderThread.joinable())
        loaderThread.join();

    loadDone = false;

    // Capturing `this` is safe because stop() (and the destructor) join the thread
    // before the Slideshow is destroyed.
    loaderThread = std::thread([this]
    {
        // The source checks for added/removed photos, picks the next one and decodes it.
        // Slow (disk, network), which is why it runs here and not on the main thread.
        loadedSurface = source->loadNext();
        photoCount = source->count();

        loadDone = true;  // publish: must be the LAST thing we do
    });
}
