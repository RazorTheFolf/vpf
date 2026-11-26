# Shortcuts for building and running the photo frame from the terminal.
# Wraps the CMake presets in CMakePresets.json (debug -> .dist/debug, release -> .dist/release).
#
#   make            build Debug (same as `make debug`)
#   make run        build Debug, then run it
#   make release    build Release
#   make run-release
#   make clean      delete all build output (.dist/)
#   make fresh      clean, then build Debug from scratch
#   make pi         build for the Raspberry Pi (in Docker) -> .dist/pi/
#   make deploy     build for the Pi, then copy it there (PI_HOST=user@host)
#   make help       list the targets

APP        := vpf
DIST       := .dist
DEBUG_DIR  := $(DIST)/debug
RELEASE_DIR:= $(DIST)/release
PI_DIR     := $(DIST)/pi

# Raspberry Pi: override on the command line, e.g. `make deploy PI_HOST=razor@frame.local`
PI_HOST    ?= user@192.168.2.32
PI_PATH    ?= ~/vpf
PI_IMAGE   := vpf-pi-builder

# Not files: always run these targets when asked.
.PHONY: all debug release run run-release clean fresh help check-tools pi pi-image deploy

all: debug

# ---- Configure (only when needed) ----
# build.ninja is created by the first configure. Making it a file target means
# `make` configures once, then skips straight to building. After that, CMake
# re-configures by itself when CMakeLists.txt changes.
$(DEBUG_DIR)/build.ninja: | check-tools
	cmake --preset debug

$(RELEASE_DIR)/build.ninja: | check-tools
	cmake --preset release

# ---- Build ----
# assets/ is synced by CMake itself (sync_assets target), so CLion builds get it too.
debug: $(DEBUG_DIR)/build.ninja
	cmake --build --preset debug

release: $(RELEASE_DIR)/build.ninja
	cmake --build --preset release

# ---- Run ----
run: debug
	./$(DEBUG_DIR)/$(APP)

run-release: release
	./$(RELEASE_DIR)/$(APP)

# ---- Clean ----
clean:
	rm -rf $(DIST)

fresh: clean debug

# ---- Raspberry Pi (Docker) ----
# The image only rebuilds when the Dockerfile changes (Docker caches it).
pi-image:
	docker build --platform linux/arm64 -t $(PI_IMAGE) -f docker/pi-build.Dockerfile docker

# Two named volumes keep vcpkg's downloads and built packages between runs:
# the first `make pi` builds every dependency (slow), later ones reuse them.
pi: pi-image
	docker run --rm --platform linux/arm64 \
		-v "$(CURDIR)":/src \
		-v vpf-vcpkg-cache:/cache/vcpkg \
		-v vpf-vcpkg-downloads:/opt/vcpkg/downloads \
		$(PI_IMAGE) \
		sh -c "cmake --preset pi && cmake --build --preset pi"

# Copies the executable and assets/ to the Pi. --delete removes files on the Pi
# that no longer exist here (e.g. a photo you deleted), so the Pi matches exactly.
deploy: pi
	ssh $(PI_HOST) "mkdir -p $(PI_PATH)"
	rsync -av --delete $(PI_DIR)/$(APP) $(PI_DIR)/assets $(PI_HOST):$(PI_PATH)/

# ---- Helpers ----
check-tools:
	@command -v cmake >/dev/null || { echo "cmake not found: brew install cmake"; exit 1; }
	@command -v ninja >/dev/null || { echo "ninja not found: brew install ninja"; exit 1; }

help:
	@grep -E '^#   make' Makefile | sed 's/^#   //'
