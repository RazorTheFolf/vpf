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
    std::string photoSource = "local";  // local | immich
    int slideDuration = 5; // Seconds, int since yaml-cpp reads Uint8 as a char
    std::string placeholderText = "Please wait...";
    float placeholderTextScale = 2.0f;

    // Immich
    std::string immichUrl;
    std::string immichAPIKey;
    std::string immichAlbumId;
};

// Missing keys keep the defaults
void LoadConfig(const std::string& path);

const Config& GetConfig();
