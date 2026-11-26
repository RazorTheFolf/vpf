//
// Created by Razor on 14/10/2026.
//

#include "app.h"

#include <SDL3/SDL_filesystem.h>
#include <memory>
#include <string>
#include <utility>

#include "core/config/config.h"
#include "core/logger/logger.h"
#include "features/slideshow/local_folder_source.h"
#include "integrations/immich/immich_source.h"

SDL_AppResult App::init()
{
    // Build paths from the executable's folder, not the current working directory,
    // so the app finds its files no matter where it is launched from.
    // SDL_GetBasePath can fail (returns nullptr), and std::string(nullptr) would crash.
    const char* basePath = SDL_GetBasePath();
    if (!basePath)
    {
        LogErrorf(LogCat::App, "Couldn't find the executable folder: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    LoadConfig(std::string(basePath) + "app_config.yml"); // next to the photos, not the working directory
    const Config& config = GetConfig();
    SDL_SetAppMetadata(config.appName.c_str(), config.appVersion.c_str(), config.appId.c_str());

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        LogErrorf(LogCat::App, "SDL_Init failed: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    if (!SDL_CreateWindowAndRenderer(config.appName.c_str(), config.windowWidth, config.windowHeight,
                                     config.renderFullscreen ? SDL_WINDOW_FULLSCREEN : SDL_WINDOW_RESIZABLE,
                                     &ctx.window, &ctx.renderer))
    {
        LogErrorf(LogCat::Render, "Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_SetRenderVSync(ctx.renderer, 1);
    config.showCursor ? SDL_ShowCursor() : SDL_HideCursor();

    const SDL_PropertiesID props = SDL_GetRendererProperties(ctx.renderer);
    ctx.display.maxTextureSize = static_cast<int>(
        SDL_GetNumberProperty(props, SDL_PROP_RENDERER_MAX_TEXTURE_SIZE_NUMBER, 0));
    LogInfof(LogCat::Render, "Renderer: %s, max texture size: %d",
             SDL_GetRendererName(ctx.renderer), ctx.display.maxTextureSize);


    if (config.immichUrl.empty() || config.immichAPIKey.empty() || config.immichAlbumId.empty())
    {
        LogError("Photo source 'immich' needs immich.url, immich.api_key and immich.album_id "
                 "in config.yml", LogCat::Immich);
        return SDL_APP_FAILURE;
    }
    LogInfof(LogCat::App, "Photo source: Immich at %s", config.immichUrl.c_str());
    std::unique_ptr<PhotoSource> source = std::make_unique<ImmichSource>(
        config.immichUrl, config.immichAPIKey, config.immichAlbumId);


    if (!slideshow.start(std::move(source)))
        return SDL_APP_FAILURE;

    return SDL_APP_CONTINUE;
}


SDL_AppResult App::iterate()
{
    // 1. Update: let features change their state (e.g. switch photo).
    slideshow.update(ctx.renderer);

    // 2. Draw: App owns the frame (clear + present); features only draw into it.
    SDL_SetRenderDrawColor(ctx.renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(ctx.renderer);
    slideshow.draw(ctx.renderer);
    SDL_RenderPresent(ctx.renderer);

    return SDL_APP_CONTINUE;
}


SDL_AppResult App::event(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_QUIT)
        return SDL_APP_SUCCESS; // window closed: end the program normally

    return SDL_APP_CONTINUE;
}


void App::quit()
{
    // Free textures while the renderer still exists: SDL destroys it right after this.
    // (The Slideshow destructor would also do it, but being explicit about order is clearer.)
    slideshow.stop();
}
