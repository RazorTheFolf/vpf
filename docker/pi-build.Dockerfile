# Same Debian and glibc as Raspberry Pi OS Bookworm
FROM debian:bookworm

RUN apt-get update && apt-get install -y --no-install-recommends \
        # Build tools
        build-essential git curl ca-certificates zip unzip tar pkg-config \
        autoconf automake libtool perl python3 python3-pip linux-libc-dev \
        # KMSDRM headers, draws without a desktop
        libdrm-dev libgbm-dev libegl-dev libgles-dev libudev-dev \
    && rm -rf /var/lib/apt/lists/*

# Debian's cmake is too old for vcpkg
RUN python3 -m pip install --no-cache-dir --break-system-packages cmake ninja

# vcpkg needs the system cmake/ninja on ARM
ENV VCPKG_ROOT=/opt/vcpkg \
    VCPKG_FORCE_SYSTEM_BINARIES=1 \
    VCPKG_DEFAULT_BINARY_CACHE=/cache/vcpkg
RUN git clone https://github.com/microsoft/vcpkg.git "$VCPKG_ROOT" \
    && "$VCPKG_ROOT/bootstrap-vcpkg.sh" -disableMetrics \
    && mkdir -p /cache/vcpkg

WORKDIR /src
