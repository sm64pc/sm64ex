#include <stdio.h>
#include <stdlib.h>
#include <ultra64.h>

#include "controller_api.h"

static FILE *fp;

static void tas_init(void) {
    fp = fopen("cont.m64", "rb");
    if (fp != NULL) {
        uint8_t buf[0x400];
        if (fread(buf, 1, sizeof(buf), fp) != 1) {
            printf("I/O error occurred.");
            exit(1);
        };
    }
}

static void tas_read(OSContPad *pad) {
    if (fp != NULL) {
        uint8_t bytes[4] = {0};
        if (fread(bytes, 1, 4, fp) != 1) {
            printf("I/O error occurred.");
            exit(1);
        };
        pad->button = (bytes[0] << 8) | bytes[1];
        pad->stick_x = bytes[2];
        pad->stick_y = bytes[3];
    }
}

static void tas_shutdown(void) {
    if (fp != NULL) {
        fclose(fp);
        fp = NULL;
    }
}

static u32 tas_rawkey(void) {
    return VK_INVALID;
}

struct ControllerAPI controller_recorded_tas = {
    VK_INVALID,
    tas_init,
    tas_read,
    tas_rawkey,
    NULL, // no rumble_play
    NULL, // no rumble_stop
    NULL, // no rebinding
    tas_shutdown
};
