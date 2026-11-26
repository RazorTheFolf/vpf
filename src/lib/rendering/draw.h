//
// Created by Razor on 14/10/2026.
//

#pragma once
#include <SDL3/SDL_render.h>

// Centered, keeps the aspect ratio
void drawFitted(SDL_Renderer* renderer, SDL_Texture* texture);

// White debug text, centered
void drawCenteredText(SDL_Renderer* renderer, const char* text, float scale);
