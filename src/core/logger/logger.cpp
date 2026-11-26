//
// Created by Razor on 14/10/2026.
//
// All log calls (ours AND SDL's internal ones) end up in Output() below,
// thanks to SDL_SetLogOutputFunction. That gives one consistent format everywhere:
//
//   17:25:03.412 INFO  [app] Slideshow with 3 photos
//

#include "logger.h"

#include <SDL3/SDL.h>
#include <cstdarg>
#include <cstdio>
#include <mutex>
#include <unistd.h>  // isatty, fileno

// Anonymous namespace = these helpers are private to this file.
namespace
{
    std::mutex logMutex; // several threads log; the lock keeps lines from mixing
    bool useColor = false; // set once in InitLogging

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

    // Label and ANSI terminal color for each level.
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
        // SDL_TimeToDateTime is thread-safe, unlike localtime() which shares one buffer.
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
    // Colors only in a real terminal. When output goes to a file or systemd's journal
    // (on the Pi), escape codes would just show up as garbage.
    useColor = isatty(fileno(stderr));

    // SDL's own categories: warnings and errors only (hides its startup info lines).
    SDL_SetLogPriorities(SDL_LOG_PRIORITY_WARN);

    // Our categories: everything in Debug builds, info and above in Release.
    // CMake defines NDEBUG automatically for Release builds.
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
// Always pass the message through "%s": if it contained a '%' (e.g. a file named
// "100%_sunset.jpg") and were used as the format itself, it would print garbage or crash.

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
// va_start/va_end must be in each function: they read that function's own `...`.

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
