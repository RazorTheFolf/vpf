//
// Created by Razor on 14/10/2026.
//

#include "draw.h"

void drawFitted(SDL_Renderer* renderer, SDL_Texture* texture)
{
    // Real pixel size of the window (differs from SDL_GetWindowSize on Retina screens).
    int outW = 0, outH = 0;
    SDL_GetRenderOutputSize(renderer, &outW, &outH);

    // "Contain": the smaller ratio makes the whole image fit.
    // Use SDL_max instead to "cover" the window (fills it, crops the edges).
    const float scale = SDL_max(static_cast<float>(outW) / static_cast<float>(texture->w),
                                static_cast<float>(outH) / static_cast<float>(texture->h));

    SDL_FRect dst;
    dst.w = static_cast<float>(texture->w) * scale;
    dst.h = static_cast<float>(texture->h) * scale;
    dst.x = (static_cast<float>(outW) - dst.w) / 2.0f;
    dst.y = (static_cast<float>(outH) - dst.h) / 2.0f;

    SDL_RenderTexture(renderer, texture, nullptr, &dst); // nullptr src = whole texture
}

void drawCenteredText(SDL_Renderer* renderer, const char* text, float scale)
{
    int outW = 0, outH = 0;
    SDL_GetRenderOutputSize(renderer, &outW, &outH);

    // SetRenderScale enlarges everything drawn afterwards, including positions.
    // So the centering math works in "scaled" units: divide the window size by scale.
    SDL_SetRenderScale(renderer, scale, scale);

    const float textW = static_cast<float>(SDL_strlen(text)) * SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;
    constexpr float textH = SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;
    const float x = (static_cast<float>(outW) / scale - textW) / 2.0f;
    const float y = (static_cast<float>(outH) / scale - textH) / 2.0f;

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderDebugText(renderer, x, y, text);

    // Reset, or everything drawn after this (like the photo) would be scaled too.
    SDL_SetRenderScale(renderer, 1.0f, 1.0f);
}
