//
// Created by Razor on 15/10/2026.
//

#include "config.h"
#include "core/logger/logger.h"

#include <yaml-cpp/yaml.h>


// The one and only Config. `static` here means only this file can touch it directly:
// everyone else goes through GetConfig(), which returns a const reference,
// so no other part of the app can change the settings by accident.
static Config config;

const Config& GetConfig()
{
    return config;
}

void LoadConfig(const std::string& path)
{
    try
    {
        YAML::Node root = YAML::LoadFile(path);

        YAML::Node app = root["application"];
        config.appName = app["name"].as<std::string>(config.appName);
        config.appVersion = app["version"].as<std::string>(config.appVersion);
        config.appId = app["id"].as<std::string>(config.appId);

        YAML::Node render = root["render"];
        config.showCursor = render["show_cursor"].as<bool>(config.showCursor);
        config.renderFullscreen = render["render_fullscreen"].as<bool>(config.renderFullscreen);
        config.windowHeight = render["window_height"].as<int>(config.windowHeight);
        config.windowWidth = render["window_width"].as<int>(config.windowWidth);

        YAML::Node slideshow = root["slideshow"];
        config.slideDuration = slideshow["slide_duration"].as<int>(config.slideDuration);
        config.placeholderText = slideshow["placeholder_text"].as<std::string>(config.placeholderText);
        config.placeholderTextScale = slideshow["placeholder_text_scale"].as<float>(config.placeholderTextScale);

        YAML::Node immich = root["immich"];
        config.immichUrl = immich["url"].as<std::string>(config.immichUrl);
        config.immichAPIKey = immich["api_key"].as<std::string>(config.immichAPIKey);
        config.immichAlbumId = immich["album_id"].as<std::string>(config.immichAlbumId);
    }
    catch (const YAML::Exception& e)
    {
        LogWarn(std::string("Using default settings: ") + e.what());
    }
}
