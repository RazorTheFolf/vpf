//
// Entry point. SDL3 "main callbacks" mode: instead of writing our own main
// loop, SDL calls these four functions for us. Each one just forwards to the
// App object.
//

#define SDL_MAIN_USE_CALLBACKS 1 // must come before including SDL_main.h
#include <SDL3/SDL_main.h>

#include "core/app/app.h"
#include "core/logger/logger.h"

SDL_AppResult SDL_AppInit(void **appstate, int /*argc*/, char * /*argv*/[]) {
  InitLogging();

  auto *app = new App();
  *appstate = app;

  return app->init();
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  return static_cast<App *>(appstate)->iterate();
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  return static_cast<App *>(appstate)->event(*event);
}

void SDL_AppQuit(void *appstate, SDL_AppResult /*result*/) {
  // Always called, even if init failed, so appstate may still be null.
  auto *app = static_cast<App *>(appstate);
  if (!app)
    return;

  app->quit();
  delete app;
}
