//
// Created by Razor on 14/10/2026.
//
// Shows photos from a PhotoSource one after another, switching every
// Config::slideDuration seconds. Choosing and decoding the next photo happens on a
// background thread while the current one is shown, so the window never freezes,
// even when the source is a slow network server.
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
    ~Slideshow();  // calls stop(), so cleanup can't be forgotten (RAII)

    // A slideshow owns a thread and GPU textures: copying it makes no sense.
    // Deleting the copy operations turns an accidental copy into a compile error.
    Slideshow(const Slideshow&) = delete;
    Slideshow& operator=(const Slideshow&) = delete;

    // Takes ownership of `source` and starts loading the first photo in the background.
    // Returns false if `source` is null.
    bool start(std::unique_ptr<PhotoSource> source);

    // Call once per frame. Swaps in the next photo when it's loaded AND its time has come.
    // Must run on the main thread (it creates textures).
    void update(SDL_Renderer* renderer);

    // Draws the current photo, or a loading message if none is ready yet.
    // const: drawing never changes the slideshow's state.
    void draw(SDL_Renderer* renderer) const;

    // Waits for the loader thread and frees all memory. Safe to call more than once.
    void stop();

private:
    void startLoading();  // ask the source for its next photo, on the loader thread

    std::unique_ptr<PhotoSource> source;  // used ONLY by the loader thread
    Uint64 nextSwitchMs = 0;  // SDL_GetTicks() value after which we may switch; 0 = show ASAP

    // Failure handling: if a whole lap of photos fails, wait before trying again
    // instead of retrying every frame.
    size_t failedInARow = 0;  // consecutive photos that couldn't be loaded
    Uint64 retryAtMs = 0;     // 0 = not waiting; otherwise SDL_GetTicks() value to retry at

    SDL_Texture* texture = nullptr;  // photo on screen (GPU memory, main thread only)

    // Hand-off between the loader thread and the main thread:
    // the worker writes photoCount and loadedSurface, THEN sets loadDone. The main
    // thread only reads them after seeing loadDone == true. std::atomic guarantees
    // the worker's writes are visible by then.
    std::thread loaderThread;
    std::atomic<bool> loadDone{false};
    SDL_Surface* loadedSurface = nullptr;  // decoded photo (CPU memory), waiting to become a texture
    size_t photoCount = 0;                 // how many photos the source had at the last load
};
