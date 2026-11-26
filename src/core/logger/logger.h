//
// Created by Razor on 14/10/2026.
//
// Logging on top of SDL_Log, with levels, per-module categories, timestamps and colors.
//
//   LogInfo("Loading " + path, LogCat::Image);                 // string version
//   LogInfof(LogCat::Image, "Size %dx%d", w, h);               // printf version
//
// Debug messages are hidden in Release builds.
//

#pragma once
#include <SDL3/SDL_log.h>
#include <string>

// One category per module, so you can tell where a message comes from.
// Starts at SDL_LOG_CATEGORY_CUSTOM so it never collides with SDL's own categories.
enum class LogCat : int
{
    App = SDL_LOG_CATEGORY_CUSTOM,
    Image,
    Render,
    Net,
    Immich,
    HomeAssistant,
    Count // keep last: used to loop over all our categories
};

// Call once, first thing in SDL_AppInit.
void InitLogging();

// ---- String versions ----
void LogDebug(const std::string& msg, LogCat cat = LogCat::App);
void LogInfo(const std::string& msg, LogCat cat = LogCat::App);
void LogWarn(const std::string& msg, LogCat cat = LogCat::App);
void LogError(const std::string& msg, LogCat cat = LogCat::App);

// ---- printf versions ----
// The category comes first because `...` must be the last parameter.
// The SDL_PRINTF_* macros let the compiler check each %d/%s against its argument.
void LogDebugf(LogCat cat, SDL_PRINTF_FORMAT_STRING const char* fmt, ...) SDL_PRINTF_VARARG_FUNC
(

2
);
void LogInfof(LogCat cat, SDL_PRINTF_FORMAT_STRING const char* fmt, ...) SDL_PRINTF_VARARG_FUNC
(

2
);
void LogWarnf(LogCat cat, SDL_PRINTF_FORMAT_STRING const char* fmt, ...) SDL_PRINTF_VARARG_FUNC
(

2
);
void LogErrorf(LogCat cat, SDL_PRINTF_FORMAT_STRING const char* fmt, ...) SDL_PRINTF_VARARG_FUNC
(

2
);
