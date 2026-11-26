# Build environment matching Raspberry Pi OS Bookworm (64-bit).
# On an Apple Silicon Mac this runs natively (arm64), so builds are fast.
# The binary built here runs on the Pi because both use Debian 12 and the same glibc.
FROM debian:bookworm

RUN apt-get update && apt-get install -y --no-install-recommends \
        # compilers and tools vcpkg needs
        build-essential git curl ca-certificates zip unzip tar pkg-config \
        autoconf automake libtool perl python3 python3-pip linux-libc-dev \
        # SDL's KMSDRM video driver (draws without a desktop): headers only,
        # SDL loads the real libraries at runtime on the Pi
        libdrm-dev libgbm-dev libegl-dev libgles-dev libudev-dev \
    && rm -rf /var/lib/apt/lists/*

# Debian's CMake is too old for recent vcpkg ports: get a current CMake and Ninja.
RUN python3 -m pip install --no-cache-dir --break-system-packages cmake ninja

# vcpkg inside the image. On ARM Linux vcpkg must use the system cmake/ninja.
ENV VCPKG_ROOT=/opt/vcpkg \
    VCPKG_FORCE_SYSTEM_BINARIES=1 \
    VCPKG_DEFAULT_BINARY_CACHE=/cache/vcpkg
RUN git clone https://github.com/microsoft/vcpkg.git "$VCPKG_ROOT" \
    && "$VCPKG_ROOT/bootstrap-vcpkg.sh" -disableMetrics \
    && mkdir -p /cache/vcpkg

WORKDIR /src
