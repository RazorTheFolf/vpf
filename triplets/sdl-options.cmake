# Extra CMake options for SDL when vcpkg builds it.
# Only video + rendering + events are needed for a photo frame.
if (PORT STREQUAL "sdl3")
    list(APPEND VCPKG_CMAKE_CONFIGURE_OPTIONS
            -DSDL_AUDIO=OFF
            -DSDL_JOYSTICK=OFF
            -DSDL_HAPTIC=OFF
            -DSDL_HIDAPI=OFF
            -DSDL_SENSOR=OFF
            -DSDL_CAMERA=OFF
    )
endif ()

if (PORT STREQUAL "sdl3-image")
    # Formats built into SDL_image that need no extra library.
    # The ones we use (PNG, JPEG) are controlled by vcpkg features instead.
    list(APPEND VCPKG_CMAKE_CONFIGURE_OPTIONS
            -DSDLIMAGE_BMP=OFF -DSDLIMAGE_GIF=OFF -DSDLIMAGE_LBM=OFF -DSDLIMAGE_PCX=OFF
            -DSDLIMAGE_PNM=OFF -DSDLIMAGE_QOI=OFF -DSDLIMAGE_SVG=OFF -DSDLIMAGE_TGA=OFF
            -DSDLIMAGE_XCF=OFF -DSDLIMAGE_XPM=OFF -DSDLIMAGE_XV=OFF -DSDLIMAGE_ANI=OFF
    )
endif ()