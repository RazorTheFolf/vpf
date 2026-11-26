# Shortcuts around the CMake presets
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

# Override with `make deploy PI_HOST=...`
PI_HOST    ?= user@192.168.2.32
PI_PATH    ?= ~/vpf
PI_IMAGE   := vpf-pi-builder

.PHONY: all debug release run run-release clean fresh help check-tools pi pi-image deploy

all: debug

# ---- Configure ----
# Only once, cmake reconfigures itself after
$(DEBUG_DIR)/build.ninja: | check-tools
	cmake --preset debug

$(RELEASE_DIR)/build.ninja: | check-tools
	cmake --preset release

# ---- Build ----
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
# Only rebuilds when the Dockerfile changes
pi-image:
	docker build --platform linux/arm64 -t $(PI_IMAGE) -f docker/pi-build.Dockerfile docker

# Volumes cache vcpkg between runs, first build is slow
pi: pi-image
	docker run --rm --platform linux/arm64 \
		-v "$(CURDIR)":/src \
		-v vpf-vcpkg-cache:/cache/vcpkg \
		-v vpf-vcpkg-downloads:/opt/vcpkg/downloads \
		$(PI_IMAGE) \
		sh -c "cmake --preset pi && cmake --build --preset pi"

# --delete keeps the Pi an exact copy
deploy: pi
	ssh $(PI_HOST) "mkdir -p $(PI_PATH)"
	rsync -av --delete $(PI_DIR)/$(APP) $(PI_DIR)/assets $(PI_HOST):$(PI_PATH)/

# ---- Helpers ----
check-tools:
	@command -v cmake >/dev/null || { echo "cmake not found: brew install cmake"; exit 1; }
	@command -v ninja >/dev/null || { echo "ninja not found: brew install ninja"; exit 1; }

help:
	@grep -E '^#   make' Makefile | sed 's/^#   //'
