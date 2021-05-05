#ifdef AAPI_SDL1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL/SDL.h>

#include "audio_api.h"

#define SNDPACKETLEN (8 * 1024)

// this is basically SDL_dataqueue but slightly less generic

typedef struct sndpacket {
    size_t datalen;         /* bytes currently in use in this packet. */
    size_t startpos;        /* bytes currently consumed in this packet. */
    struct sndpacket *next; /* next item in linked list. */
    Uint8 data[];           /* packet data */
} sndpacket_t;

static sndpacket_t *qhead;
static sndpacket_t *qtail;
static sndpacket_t *qpool;
static size_t queued;

static SDL_AudioSpec aspec;
static int was_init = 0;

static void sndqueue_init(const size_t bufsize) {
    const size_t wantpackets = (bufsize + (SNDPACKETLEN - 1)) / SNDPACKETLEN;
    for (size_t i = 0; i < wantpackets; ++i) {
        sndpacket_t *packet = malloc(sizeof(sndpacket_t) + SNDPACKETLEN);
        if (packet) {
            packet->datalen = 0;
            packet->startpos = 0;
            packet->next = qpool;
            qpool = packet;
        }
    }
}

static size_t sndqueue_read(void *buf, size_t len) {
    sndpacket_t *packet;
    Uint8 *ptr = buf;

    while ((len > 0) && ((packet = qhead) != NULL)) {
        const size_t avail = packet->datalen - packet->startpos;
        const size_t tx = (len < avail) ? len : avail;

        memcpy(ptr, packet->data + packet->startpos, tx);
        packet->startpos += tx;
        ptr += tx;
        queued -= tx;
        len -= tx;

        if (packet->startpos == packet->datalen) {
            qhead = packet->next;
            packet->next = qpool;
            qpool = packet;
        }
    }

    if (qhead == NULL)
        qtail = NULL;

    return (size_t)(ptr - (Uint8*)buf);
}

static inline sndpacket_t *alloc_sndpacket(void) {
    sndpacket_t *packet = qpool;

    if (packet) {
        qpool = packet->next;
    } else {
        packet = malloc(sizeof(sndpacket_t) + SNDPACKETLEN);
        if (!packet) return NULL;
    }

    packet->datalen = 0;
    packet->startpos = 0;
    packet->next = NULL;

    if (qtail == NULL)
        qhead = packet;
    else
        qtail->next = packet;
    qtail = packet;

    return packet;
}

static int sndqueue_push(const void *data, size_t len) {
    sndpacket_t *orighead = qhead;
    sndpacket_t *origtail = qtail;
    size_t origlen = origtail ? origtail->datalen : 0;
    Uint8 *ptr = data;

    while (len > 0) {
        sndpacket_t *packet = qtail;
        if (!packet || (packet->datalen >= SNDPACKETLEN)) {
            packet = alloc_sndpacket();
            if (!packet) {
                // out of memory, fuck everything
                return -1;
            }
        }

        const size_t room = SNDPACKETLEN - packet->datalen;
        const size_t datalen = (len < room) ? len : room;
        memcpy(packet->data + packet->datalen, ptr, datalen);
        ptr += datalen;
        len -= datalen;
        packet->datalen += datalen;
        queued += datalen;
    }

    return 0;
}

static void audio_drain(void *user, Uint8 *buf, int len) {
    const size_t tx = sndqueue_read(buf, len);
    buf += tx;
    len -= (int)tx;
    if (len > 0) memset(buf, aspec.silence, len);
}

static bool audio_sdl_init(void) {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "SDL init error: %s\n", SDL_GetError());
        return false;
    }

    SDL_AudioSpec want, have;
    memset(&want, 0, sizeof(want));
    want.freq = 32000;
    want.format = AUDIO_S16SYS;
    want.channels = 2;
    want.samples = 512;
    want.callback = audio_drain;
    if (SDL_OpenAudio(&want, &have) == -1) {
        fprintf(stderr, "SDL_OpenAudio error: %s\n", SDL_GetError());
        return false;
    }

    aspec = have;

    was_init = 1;
    SDL_PauseAudio(0);

    return true;
}

static int audio_sdl_buffered(void) {
    SDL_LockAudio();
    int len = queued / 4;
    SDL_UnlockAudio();
    return len;
}

static int audio_sdl_get_desired_buffered(void) {
    return 1100;
}

static void audio_sdl_play(const uint8_t *buf, size_t len) {
    SDL_LockAudio();
    // Don't fill the audio buffer too much in case this happens
    if (queued / 4 < 6000)
        sndqueue_push(buf, len);
    SDL_UnlockAudio();
}

static void audio_sdl_shutdown(void)  {
    if (SDL_WasInit(SDL_INIT_AUDIO)) {
        if (was_init) {
            SDL_CloseAudio();
            was_init = 0;
        }
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }
}

struct AudioAPI audio_sdl = {
    audio_sdl_init,
    audio_sdl_buffered,
    audio_sdl_get_desired_buffered,
    audio_sdl_play,
    audio_sdl_shutdown
};

#endif
