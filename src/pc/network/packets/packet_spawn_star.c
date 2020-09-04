#include <stdio.h>
#include "../network.h"
#include "object_fields.h"
#include "src/game/behavior_actions.h"

extern struct Object* gCurrentObject;

void network_send_spawn_star(struct Object* o, u8 starType, f32 x, f32 y, f32 z, u32 behParams) {
    struct Packet p;
    packet_init(&p, PACKET_SPAWN_STAR, true);
    packet_write(&p, &starType, sizeof(u8));
    packet_write(&p, &x, sizeof(f32));
    packet_write(&p, &y, sizeof(f32));
    packet_write(&p, &z, sizeof(f32));
    packet_write(&p, &behParams, sizeof(u32));

    packet_write(&p, &o->oPosX, sizeof(u32) * 3);
    packet_write(&p, &o->oHomeX, sizeof(u32) * 3);

    network_send(&p);
}

void network_receive_spawn_star(struct Packet* p) {
    u8 starType;
    f32 x, y, z;
    u32 behParams;

    packet_read(p, &starType, sizeof(u8));
    packet_read(p, &x, sizeof(f32));
    packet_read(p, &y, sizeof(f32));
    packet_read(p, &z, sizeof(f32));
    packet_read(p, &behParams, sizeof(u32));

    u32 oldBehParams = gCurrentObject->oBehParams;
    gCurrentObject->oBehParams = behParams;
    struct Object* o = NULL;
    switch (starType) {
        case 0: o = spawn_default_star(x, y, z); break;
        case 1: o = spawn_red_coin_cutscene_star(x, y, z); break;
        case 2: o = spawn_no_exit_star(x, y, z); break;
        default: printf("UNKNOWN SPAWN STAR %d\n", starType);
    }
    gCurrentObject->oBehParams = oldBehParams;

    if (o != NULL) {
        packet_read(p, &o->oPosX, sizeof(u32) * 3);
        packet_read(p, &o->oHomeX, sizeof(u32) * 3);
    }
}
