//
// Created by Razor on 14/10/2026.
//
// State shared by several parts of the app.
// Rule of thumb: if only one feature uses something, it belongs inside that feature
// (e.g. the slideshow's texture lives in Slideshow), not here.
//

#pragma once
#include <SDL3/SDL_render.h>


// Facts about the display, queried once at startup.
struct DisplayInfo
{
    int maxTextureSize = 0; // largest texture side in pixels; 0 = renderer reports no limit
};

struct AppContext
{
    // Owned by SDL: created in App::init, destroyed automatically by SDL at shutdown.
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    DisplayInfo display;
};
