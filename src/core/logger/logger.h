//
// Created by Razor on 14/10/2026.
//

#pragma once
#include <SDL3/SDL_log.h>
#include <string>

// One category per module, after SDL's own ones
enum class LogCat : int
{
    App = SDL_LOG_CATEGORY_CUSTOM,
    Image,
    Render,
    Net,
    Immich,
    HomeAssistant,
    Count // Keep last
};

// Call first in SDL_AppInit
void InitLogging();

// ---- String versions ----
void LogDebug(const std::string& msg, LogCat cat = LogCat::App);
void LogInfo(const std::string& msg, LogCat cat = LogCat::App);
void LogWarn(const std::string& msg, LogCat cat = LogCat::App);
void LogError(const std::string& msg, LogCat cat = LogCat::App);

// ---- printf versions ----
// Category first since `...` must be last
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
