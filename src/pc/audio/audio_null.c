#include "audio_api.h"

static bool audio_null_init(void) {
    return true;
}

static int audio_null_buffered(void) {
    return 0;
}

static int audio_null_get_desired_buffered(void) {
    return 0;
}

static void audio_null_play(const uint8_t *buf, size_t len) {
}

static void audio_null_shutdown(void) {
}

struct AudioAPI audio_null = {
    audio_null_init,
    audio_null_buffered,
    audio_null_get_desired_buffered,
    audio_null_play,
    audio_null_shutdown
};