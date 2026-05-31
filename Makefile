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
  tests/unit/test_edit_start.c \
  tests/unit/test_ring.c \
  tests/unit/test_wakeup.c \
  tests/unit/runner.c

UNIT_BIN := tests/unit/build/run_tests

.PHONY: test build clean

test: $(UNIT_BIN)
	$(UNIT_BIN)

$(UNIT_BIN): $(UNIT_SRCS)
	@mkdir -p tests/unit/build
	$(CC) $(CFLAGS) -o $@ $(UNIT_SRCS)

build:
	pebble build

clean:
	rm -rf tests/unit/build build
