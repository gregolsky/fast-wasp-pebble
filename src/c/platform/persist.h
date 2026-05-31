#pragma once

#ifdef FAST_PEBBLE_HOST_BUILD
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define PERSIST_MAX_SLOTS 32
#define PERSIST_MAX_BYTES 1024

typedef struct {
    uint32_t key;
    uint8_t  data[PERSIST_MAX_BYTES];
    size_t   size;
    bool     used;
} PersistSlot;

extern PersistSlot g_persist[PERSIST_MAX_SLOTS];

static inline void persist_reset_all(void) {
    for (int i = 0; i < PERSIST_MAX_SLOTS; i++) g_persist[i].used = false;
}

static inline PersistSlot *persist_find(uint32_t key) {
    for (int i = 0; i < PERSIST_MAX_SLOTS; i++)
        if (g_persist[i].used && g_persist[i].key == key) return &g_persist[i];
    return NULL;
}

static inline PersistSlot *persist_alloc(uint32_t key) {
    PersistSlot *s = persist_find(key);
    if (s) return s;
    for (int i = 0; i < PERSIST_MAX_SLOTS; i++) {
        if (!g_persist[i].used) { g_persist[i].key = key; g_persist[i].used = true; return &g_persist[i]; }
    }
    return NULL;
}

static inline bool persist_exists(uint32_t key) { return persist_find(key) != NULL; }

static inline int persist_read_int(uint32_t key) {
    PersistSlot *s = persist_find(key);
    if (!s || s->size < sizeof(int32_t)) return 0;
    int32_t v; memcpy(&v, s->data, sizeof(v)); return v;
}

static inline uint8_t persist_read_uint8(uint32_t key) {
    PersistSlot *s = persist_find(key);
    if (!s || s->size < 1) return 0;
    return s->data[0];
}

static inline int persist_read_data(uint32_t key, void *buf, size_t n) {
    PersistSlot *s = persist_find(key);
    if (!s) return -1;
    size_t cp = s->size < n ? s->size : n;
    memcpy(buf, s->data, cp);
    return (int)cp;
}

static inline void persist_write_int(uint32_t key, int32_t val) {
    PersistSlot *s = persist_alloc(key);
    if (!s) return;
    memcpy(s->data, &val, sizeof(val));
    s->size = sizeof(val);
}

static inline void persist_write_data(uint32_t key, const void *buf, size_t n) {
    PersistSlot *s = persist_alloc(key);
    if (!s || n > PERSIST_MAX_BYTES) return;
    memcpy(s->data, buf, n);
    s->size = n;
}

static inline int persist_get_size(uint32_t key) {
    PersistSlot *s = persist_find(key);
    return s ? (int)s->size : -1;
}

static inline void persist_delete(uint32_t key) {
    PersistSlot *s = persist_find(key);
    if (s) s->used = false;
}

#else
#include <pebble.h>
#endif
