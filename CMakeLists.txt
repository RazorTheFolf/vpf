cmake_minimum_required(VERSION 3.24)
project(vpf VERSION 0.1.0 LANGUAGES CXX)

# ---- C++ settings ----
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---- Packages ----
find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)
find_package(PahoMqttCpp CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)

# ---- Sources ----
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(vpf ${SOURCES})
target_include_directories(vpf PRIVATE src)

# ---- Warnings ----
target_compile_options(vpf PRIVATE -Wall -Wextra -Werror=return-type)

# ---- Link the packages ----
target_link_libraries(vpf PRIVATE
        SDL3::SDL3
        SDL3_image::SDL3_image
        # Static and shared builds name it differently
        $<IF:$<TARGET_EXISTS:PahoMqttCpp::paho-mqttpp3-static>,PahoMqttCpp::paho-mqttpp3-static,PahoMqttCpp::paho-mqttpp3>
        cpr::cpr
        nlohmann_json::nlohmann_json
        yaml-cpp::yaml-cpp
)

# ---- Copy app_config.yml next to the executable ----
if (EXISTS "${CMAKE_SOURCE_DIR}/assets/app_config.yml")
    add_custom_target(sync_app_config ALL
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/assets/app_config.yml"
            "${CMAKE_BINARY_DIR}/app_config.yml"
            COMMENT "Syncing config file"
            VERBATIM)
    add_dependencies(vpf sync_app_config)
endif ()
