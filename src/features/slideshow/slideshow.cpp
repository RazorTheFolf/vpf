//
// Created by Razor on 14/10/2026.
//

#include "slideshow.h"

#include <SDL3/SDL_timer.h>
#include <utility>

#include "core/config/config.h"
#include "core/logger/logger.h"
#include "lib/rendering/draw.h"

// Retry delay when nothing loads (server down, empty album...)
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

    source = std::move(newSource);
    startLoading();
    return true;
}

void Slideshow::update(SDL_Renderer* renderer)
{
    if (!loadDone)
        return;

    // Loading failed
    if (!loadedSurface)
    {
        if (retryAtMs != 0)
        {
            if (SDL_GetTicks() < retryAtMs)
                return;
            retryAtMs = 0;
            startLoading();
            return;
        }

        // Skip one bad photo, but wait if the whole lap failed
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

        startLoading();
        return;
    }

    failedInARow = 0;

    if (SDL_GetTicks() < nextSwitchMs)
        return;

    // Destroy the old texture first or it leaks
    SDL_DestroyTexture(texture);
    texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    if (!texture)
        LogErrorf(LogCat::Render, "Couldn't create texture: %s", SDL_GetError());

    SDL_DestroySurface(loadedSurface);
    loadedSurface = nullptr;

    nextSwitchMs = SDL_GetTicks() + static_cast<Uint64>(GetConfig().slideDuration) * 1000;
    startLoading();  // Preload the next one
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
    // The loader may still be writing loadedSurface
    if (loaderThread.joinable())
        loaderThread.join();

    SDL_DestroySurface(loadedSurface);
    loadedSurface = nullptr;

    SDL_DestroyTexture(texture);
    texture = nullptr;
}

void Slideshow::startLoading()
{
    if (loaderThread.joinable())
        loaderThread.join();

    loadDone = false;

    // Capturing `this` is fine, stop() joins before we're destroyed
    loaderThread = std::thread([this]
    {
        loadedSurface = source->loadNext();
        photoCount = source->count();

        loadDone = true;  // Must be last
    });
}
