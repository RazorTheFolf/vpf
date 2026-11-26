cmake_minimum_required(VERSION 3.24)
project(vpf VERSION 0.1.0 LANGUAGES CXX)

# ---- C++ settings ----
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # better autocomplete in VS Code / CLion

# ---- Packages (installed by vcpkg from vcpkg.json) ----
find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)
find_package(PahoMqttCpp CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

# ---- Source files: picks up every .cpp in src/ ----
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(vpf ${SOURCES})
target_include_directories(vpf PRIVATE src)

# ---- Warnings: let the compiler catch bugs early ----
# -Werror=return-type makes "function doesn't return a value" a build error.
target_compile_options(vpf PRIVATE -Wall -Wextra -Werror=return-type)

# ---- Link the packages ----
target_link_libraries(vpf PRIVATE
        SDL3::SDL3
        SDL3_image::SDL3_image
        # Paho names its target differently for static vs shared builds
        $<IF:$<TARGET_EXISTS:PahoMqttCpp::paho-mqttpp3-static>,PahoMqttCpp::paho-mqttpp3-static,PahoMqttCpp::paho-mqttpp3>
        cpr::cpr
        nlohmann_json::nlohmann_json
        yaml-cpp::yaml-cpp
)

# ---- Sync assets/ (photos, settings.json) next to the executable ----
# A custom target with no output file runs on EVERY build, even when no code changed
# (a POST_BUILD command only runs when the executable is relinked).
# Deleting first keeps the copy exact: photos removed from assets/ disappear too.
# Uses CMAKE_BINARY_DIR (where the executable is built) rather than $<TARGET_FILE_DIR:vpf>:
# referring to vpf here would make sync_assets depend on vpf, and vpf on sync_assets (a cycle).
if (EXISTS "${CMAKE_SOURCE_DIR}/assets/app_config.yml")
    add_custom_target(sync_app_config ALL
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/assets/app_config.yml"
            "${CMAKE_BINARY_DIR}/app_config.yml"
            COMMENT "Syncing config file"
            VERBATIM)
    add_dependencies(vpf sync_app_config)
endif ()
