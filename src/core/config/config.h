//
// Created by Razor on 15/10/2026.
//

#pragma once
#include <string>
#include <SDL3/SDL_stdinc.h>


struct Config
{
    // Application
    std::string appName = "vpf";
    std::string appVersion = "x.x.x";
    std::string appId = "dev.razor.vpf";

    // Render
    bool showCursor = false;
    bool renderFullscreen = false;
    int windowWidth = 1280;
    int windowHeight = 800;

    // Slideshow
    std::string photoSource = "local";  // "local" (the assets folder) or "immich"
    int slideDuration = 5; // seconds (int: yaml-cpp reads Uint8 as a character)
    std::string placeholderText = "Please wait...";
    float placeholderTextScale = 2.0f;

    // Immich (used when photoSource is "immich")
    std::string immichUrl;  // e.g. "http://192.168.1.10:2283"
    std::string immichAPIKey;
    std::string immichAlbumId;
};

// Reads the YAML file into the global config. Call once, at the start of App::init().
// Missing keys (or a missing file) keep the defaults above.
void LoadConfig(const std::string& path);

// The global config, read-only. Usable from any file that includes config.h:
//   GetConfig().slideDuration
const Config& GetConfig();
