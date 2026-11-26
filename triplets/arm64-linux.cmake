# Raspberry Pi OS 64-bit
set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME Linux)

include(${CMAKE_CURRENT_LIST_DIR}/sdl-options.cmake)

# Draw without a desktop
if (PORT STREQUAL "sdl3")
    list(APPEND VCPKG_CMAKE_CONFIGURE_OPTIONS
            -DSDL_KMSDRM=ON
            -DSDL_UNIX_CONSOLE_BUILD=ON
            -DSDL_X11=OFF
            -DSDL_WAYLAND=OFF
    )
endif ()
