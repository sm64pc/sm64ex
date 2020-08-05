#ifndef NETWORK_H
#define NETWORK_H

#include <time.h>
#include <types.h>
#include <assert.h>
#include "../cliopts.h"

#define SYNC_DISTANCE_ONLY_DEATH -1
#define SYNC_DISTANCE_INFINITE 0
#define MAX_SYNC_OBJECTS 256
#define MAX_SYNC_OBJECT_FIELDS 16
#define PACKET_LENGTH 1024
#define NETWORKTYPESTR (networkType == NT_CLIENT ? "Client" : "Server")

enum PacketType {
    PACKET_PLAYER,
    PACKET_OBJECT,
    PACKET_LEVEL_WARP,
    PACKET_INSIDE_PAINTING,
};

struct Packet {
    int cursor;
    bool error;
    bool reliable;
    u16 seqId;
    char buffer[PACKET_LENGTH];
};

struct SyncObject {
    struct Object* o;
    float maxSyncDistance;
    bool owned;
    unsigned int ticksSinceUpdate;
    unsigned int syncDeactive;
    u8 extraFieldCount;
    void* extraFields[MAX_SYNC_OBJECT_FIELDS];
};

extern struct MarioState gMarioStates[];
extern u8 gInsidePainting;
extern s16 sCurrPlayMode;
extern enum NetworkType networkType;
extern struct SyncObject syncObjects[];

void network_init(enum NetworkType networkType);
void network_init_object(struct Object *object, float maxSyncDistance);
void network_send(struct Packet* p);
void network_update(void);
void network_shutdown(void);

// packet read / write
void packet_init(struct Packet* packet, enum PacketType packetType);
void packet_write(struct Packet* packet, void* data, int length);
void packet_read(struct Packet* packet, void* data, int length);

// packet headers
void network_update_player(void);
void network_receive_player(struct Packet* p);

void network_update_objects(void);
void network_receive_object(struct Packet* p);

void network_update_level_warp(void);
void network_receive_level_warp(struct Packet* p);

void network_update_inside_painting(void);
void network_receive_inside_painting(struct Packet* p);

#endif
