//
// Created by Razor on 14/10/2026.
//

#pragma once
#include <SDL3/SDL_render.h>


struct DisplayInfo
{
    int maxTextureSize = 0; // 0 = no limit
};

struct AppContext
{
    // Owned by SDL, destroyed on quit
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    DisplayInfo display;
};
