//
// Created by Razor on 14/10/2026.
//

#pragma once
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>

#include "core/app_context.h"
#include "features/slideshow/slideshow.h"

class App {
public:
  SDL_AppResult init();                        // once at startup
  SDL_AppResult iterate();                     // once per frame
  SDL_AppResult event(const SDL_Event &event); // once per input/window event
  void quit();                                 // once at shutdown

private:
  AppContext ctx;
  Slideshow slideshow;
};
