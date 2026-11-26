//
// Created by Razor on 14/10/2026.
//

#include "draw.h"

void drawFitted(SDL_Renderer* renderer, SDL_Texture* texture)
{
    // Real pixels, differs on Retina
    int outW = 0, outH = 0;
    SDL_GetRenderOutputSize(renderer, &outW, &outH);

    // SDL_max = cover (crops), SDL_min = contain
    const float scale = SDL_max(static_cast<float>(outW) / static_cast<float>(texture->w),
                                static_cast<float>(outH) / static_cast<float>(texture->h));

    SDL_FRect dst;
    dst.w = static_cast<float>(texture->w) * scale;
    dst.h = static_cast<float>(texture->h) * scale;
    dst.x = (static_cast<float>(outW) - dst.w) / 2.0f;
    dst.y = (static_cast<float>(outH) - dst.h) / 2.0f;

    SDL_RenderTexture(renderer, texture, nullptr, &dst);
}

void drawCenteredText(SDL_Renderer* renderer, const char* text, float scale)
{
    int outW = 0, outH = 0;
    SDL_GetRenderOutputSize(renderer, &outW, &outH);

    // SetRenderScale scales positions too, so divide by scale
    SDL_SetRenderScale(renderer, scale, scale);

    const float textW = static_cast<float>(SDL_strlen(text)) * SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;
    constexpr float textH = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;
    const float x = (static_cast<float>(outW) / scale - textW) / 2.0f;
    const float y = (static_cast<float>(outH) / scale - textH) / 2.0f;

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderDebugText(renderer, x, y, text);

    // Reset for the next draws
    SDL_SetRenderScale(renderer, 1.0f, 1.0f);
}
