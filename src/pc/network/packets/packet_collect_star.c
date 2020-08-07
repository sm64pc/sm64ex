#include <stdio.h>
#include "../network.h"
#include "course_table.h"

extern s16 gCurrSaveFileNum;
extern s16 gCurrCourseNum;

void network_send_collect_star(s16 coinScore, s16 starIndex) {
    struct Packet p;
    packet_init(&p, PACKET_COLLECT_STAR, true);

    packet_write(&p, &gCurrSaveFileNum, sizeof(s16));
    packet_write(&p, &gCurrCourseNum, sizeof(s16));
    packet_write(&p, &coinScore, sizeof(s16));
    packet_write(&p, &starIndex, sizeof(s16));

    network_send(&p);
}

void network_receive_collect_star(struct Packet* p) {
    s16 coinScore, starIndex;
    s16 lastSaveFileNum = gCurrSaveFileNum;
    s16 lastCourseNum = gCurrCourseNum;

    packet_read(p, &gCurrSaveFileNum, sizeof(s16));
    packet_read(p, &gCurrCourseNum, sizeof(s16));
    packet_read(p, &coinScore, sizeof(s16));
    packet_read(p, &starIndex, sizeof(s16));

    save_file_collect_star_or_key(coinScore, starIndex);

    s32 numStars = save_file_get_total_star_count(gCurrSaveFileNum - 1, COURSE_MIN - 1, COURSE_MAX - 1);
    for (int i = 0; i < 2; i++) {
        gMarioStates[i].numStars = numStars;
    }

    gCurrSaveFileNum = lastSaveFileNum;
    gCurrCourseNum = lastCourseNum;
}
