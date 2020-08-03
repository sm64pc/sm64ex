#ifndef NETWORK_H
#define NETWORK_H

#include <types.h>
#include <assert.h>
#include "../cliopts.h"

#define MAX_SYNC_OBJECTS 256
#define MAX_SYNC_OBJECT_FIELDS 16
#define PACKET_LENGTH 1024
#define NETWORKTYPESTR (networkType == NT_CLIENT ? "Client" : "Server")

enum PacketType {
    PACKET_PLAYER,
    PACKET_OBJECT,
    PACKET_LEVEL_WARP,
};

struct Packet {
    int cursor;
    bool error;
    char buffer[PACKET_LENGTH];
};

struct SyncObject {
    struct Object* o;
    bool owned;
    unsigned int ticksSinceUpdate;
    unsigned int syncDeactive;
    u8 extraFieldCount;
    void* extraFields[MAX_SYNC_OBJECT_FIELDS];
};

extern struct MarioState gMarioStates[];
extern s16 sCurrPlayMode;
extern enum NetworkType networkType;
extern struct SyncObject syncObjects[];

void network_init(enum NetworkType networkType);
void network_init_object(struct Object *object);
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

#endif
