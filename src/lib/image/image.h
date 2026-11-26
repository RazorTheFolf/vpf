//
// Created by Razor on 14/10/2026.
//

#pragma once
#include <SDL3/SDL_surface.h>
#include <cstddef>

// Safe off the main thread, caller frees with SDL_DestroySurface
SDL_Surface* loadImage(const char* imagePath);

// `name` is only for logs
SDL_Surface* decodeImage(const void* data, size_t size, const char* name);
