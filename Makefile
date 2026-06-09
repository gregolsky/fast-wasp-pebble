PEBBLE_IMAGE := gregolsky/pebble-sdk:pebble-tool-5.0.37-sdk-4.9.169
VERSION      := $(shell python3 -c "import json; print(json.load(open('package.json'))['version'])")

CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Wpedantic -g \
           -Isrc/c \
           -Itests/unit/vendor/unity \
           -Itests/unit/shims \
           -DFAST_PEBBLE_HOST_BUILD

UNIT_SRCS := \
  src/c/fasting.c \
  src/c/storage.c \
  src/c/notify.c \
  tests/unit/shims/time_shim.c \
  tests/unit/shims/persist_shim.c \
  tests/unit/vendor/unity/unity.c \
  tests/unit/test_program.c \
  tests/unit/test_fasting.c \
  tests/unit/test_omad.c \
  tests/unit/test_stats.c \
  tests/unit/test_wakeup.c \
  tests/unit/runner.c

UNIT_BIN := tests/unit/build/run_tests

.PHONY: test build clean e2e

src/c/version.h: package.json
	echo '#define APP_VERSION "$(VERSION)"' > src/c/version.h

test: src/c/version.h $(UNIT_BIN)
	$(UNIT_BIN)

$(UNIT_BIN): $(UNIT_SRCS)
	@mkdir -p tests/unit/build
	$(CC) $(CFLAGS) -o $@ $(UNIT_SRCS)

build: src/c/version.h
	docker run --rm -v "$(CURDIR):/pebble" -w /pebble $(PEBBLE_IMAGE) pebble build

e2e:
	./run-e2e.sh

clean:
	rm -rf tests/unit/build build src/c/version.h
