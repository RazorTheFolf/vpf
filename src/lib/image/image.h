//
// Created by Razor on 14/10/2026.
//
// Image loading. Pure CPU work, no renderer needed, so it is safe to call
// from a background thread.
//
// Both functions return a CPU-side surface, or nullptr on failure (and log why).
// The caller owns the surface and must free it with SDL_DestroySurface.
//

#pragma once
#include <SDL3/SDL_surface.h>
#include <cstddef>

// Loads an image file (PNG, JPEG...) from disk.
SDL_Surface* loadImage(const char* imagePath);

// Decodes an image that is already in memory (e.g. downloaded from Immich).
// `name` is only used in log messages. The format is detected from the bytes.
SDL_Surface* decodeImage(const void* data, size_t size, const char* name);
