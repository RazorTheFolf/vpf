//
// Created by Razor on 14/10/2026.
//
// Small drawing helpers. They know nothing about the app: they take a renderer
// and what to draw. They don't clear or present; the caller owns the frame.
//

#pragma once
#include <SDL3/SDL_render.h>

// Draws a texture as large as possible inside the window, keeping its aspect ratio
// and centering it. Leftover space stays as whatever was cleared (black bars).
void drawFitted(SDL_Renderer* renderer, SDL_Texture* texture);

// Draws white debug-font text centered in the window, enlarged by `scale`.
void drawCenteredText(SDL_Renderer* renderer, const char* text, float scale);
