//
// Created by Razor on 14/10/2026.
//

#include "logger.h"

#include <SDL3/SDL.h>
#include <cstdarg>
#include <cstdio>
#include <mutex>
#include <unistd.h>  // isatty, fileno

namespace
{
    std::mutex logMutex; // Several threads log at once
    bool useColor = false;

    const char* CategoryName(const int category)
    {
        switch (category)
        {
        case static_cast<int>(LogCat::App): return "app";
        case static_cast<int>(LogCat::Image): return "image";
        case static_cast<int>(LogCat::Render): return "render";
        case static_cast<int>(LogCat::Net): return "net";
        case static_cast<int>(LogCat::Immich): return "immich";
        case static_cast<int>(LogCat::HomeAssistant): return "ha";
        case SDL_LOG_CATEGORY_VIDEO: return "sdl/video";
        case SDL_LOG_CATEGORY_RENDER: return "sdl/render";
        default: return "sdl";
        }
    }

    void LevelStyle(const SDL_LogPriority priority, const char*& name, const char*& color)
    {
        switch (priority)
        {
        case SDL_LOG_PRIORITY_DEBUG:
            name = "DEBUG";
            color = "\033[90m"; // gray
            break;
        case SDL_LOG_PRIORITY_INFO:
            name = "INFO";
            color = "\033[36m"; // cyan
            break;
        case SDL_LOG_PRIORITY_WARN:
            name = "WARN";
            color = "\033[33m"; // yellow
            break;
        case SDL_LOG_PRIORITY_ERROR:
        case SDL_LOG_PRIORITY_CRITICAL:
            name = "ERROR";
            color = "\033[31m"; // red
            break;
        default:
            name = "TRACE";
            color = "\033[90m"; // gray
            break;
        }
    }

    void Output(void* /*userdata*/, const int category, const SDL_LogPriority priority, const char* message)
    {
        // Thread-safe, unlike localtime()
        SDL_Time now = 0;
        SDL_DateTime dt{};
        SDL_GetCurrentTime(&now);
        SDL_TimeToDateTime(now, &dt, true);

        const char* name = nullptr;
        const char* color = nullptr;
        LevelStyle(priority, name, color);

        const char* reset = "\033[0m";

        std::lock_guard lock(logMutex);
        std::fprintf(stderr, "%02d:%02d:%02d.%03d %s%-5s%s [%s] %s\n",
                     dt.hour, dt.minute, dt.second, dt.nanosecond / 1'000'000,
                     useColor ? color : "", name, useColor ? reset : "",
                     CategoryName(category), message);
    }
}

void InitLogging()
{
    // No colors when piped to a file or journald
    useColor = isatty(fileno(stderr));

    // Only warnings and errors from SDL itself
    SDL_SetLogPriorities(SDL_LOG_PRIORITY_WARN);

    // Debug logs only in Debug builds
#ifdef NDEBUG
    constexpr SDL_LogPriority ours = SDL_LOG_PRIORITY_INFO;
#else
    constexpr SDL_LogPriority ours = SDL_LOG_PRIORITY_DEBUG;
#endif
    for (int c = static_cast<int>(LogCat::App); c < static_cast<int>(LogCat::Count); c++)
        SDL_SetLogPriority(c, ours);

    SDL_SetLogOutputFunction(Output, nullptr);
}

// ---- String versions ----
// Always go through "%s", a '%' in the message would break the format

void LogDebug(const std::string& msg, LogCat cat)
{
    SDL_LogDebug(static_cast<int>(cat), "%s", msg.c_str());
}

void LogInfo(const std::string& msg, LogCat cat)
{
    SDL_LogInfo(static_cast<int>(cat), "%s", msg.c_str());
}

void LogWarn(const std::string& msg, LogCat cat)
{
    SDL_LogWarn(static_cast<int>(cat), "%s", msg.c_str());
}

void LogError(const std::string& msg, LogCat cat)
{
    SDL_LogError(static_cast<int>(cat), "%s", msg.c_str());
}

// ---- printf versions ----

void LogDebugf(LogCat cat, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    SDL_LogMessageV(static_cast<int>(cat), SDL_LOG_PRIORITY_DEBUG, fmt, args);
    va_end(args);
}

void LogInfof(LogCat cat, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    SDL_LogMessageV(static_cast<int>(cat), SDL_LOG_PRIORITY_INFO, fmt, args);
    va_end(args);
}

void LogWarnf(LogCat cat, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    SDL_LogMessageV(static_cast<int>(cat), SDL_LOG_PRIORITY_WARN, fmt, args);
    va_end(args);
}

void LogErrorf(LogCat cat, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    SDL_LogMessageV(static_cast<int>(cat), SDL_LOG_PRIORITY_ERROR, fmt, args);
    va_end(args);
}
