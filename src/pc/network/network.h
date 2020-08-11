#ifndef NETWORK_H
#define NETWORK_H

#include <time.h>
#include <types.h>
#include <assert.h>
#include "../cliopts.h"

#define SYNC_DISTANCE_ONLY_DEATH -1.0f
#define SYNC_DISTANCE_ONLY_EVENTS -2.0f
#define SYNC_DISTANCE_INFINITE 0
#define MAX_SYNC_OBJECTS 256
#define MAX_SYNC_OBJECT_FIELDS 16
#define PACKET_LENGTH 1024
#define NETWORKTYPESTR (networkType == NT_CLIENT ? "Client" : "Server")

enum PacketType {
    PACKET_ACK,
    PACKET_PLAYER,
    PACKET_OBJECT,
    PACKET_SPAWN_OBJECTS,
    PACKET_SPAWN_STAR,
    PACKET_LEVEL_WARP,
    PACKET_INSIDE_PAINTING,
    PACKET_COLLECT_STAR,
    PACKET_COLLECT_COIN,
    PACKET_COLLECT_ITEM,
};

struct Packet {
    int dataLength;
    int cursor;
    bool error;
    bool reliable;
    u16 seqId;
    bool sent;
    char buffer[PACKET_LENGTH];
};

struct SyncObject {
    struct Object* o;
    float maxSyncDistance;
    bool owned;
    clock_t clockSinceUpdate;
    void* behavior;
    u16 onEventId;
    u8 extraFieldCount;
    bool fullObjectSync;
    bool keepRandomSeed;
    float maxUpdateRate;
    u8 (*ignore_if_true)(struct Object*);
    void* extraFields[MAX_SYNC_OBJECT_FIELDS];
};

extern struct MarioState gMarioStates[];
extern u8 gInsidePainting;
extern s16 sCurrPlayMode;
extern enum NetworkType networkType;
extern struct SyncObject syncObjects[];

void network_init(enum NetworkType networkType);
void network_init_object(struct Object *object, float maxSyncDistance);
void network_object_settings(struct Object *object, bool fullObjectSync, float maxUpdateRate, bool keepRandomSeed, u8 ignore_if_true(struct Object*));
void network_send(struct Packet* p);
void network_update(void);
void network_shutdown(void);

// packet read / write
void packet_init(struct Packet* packet, enum PacketType packetType, bool reliable);
void packet_write(struct Packet* packet, void* data, int length);
void packet_read(struct Packet* packet, void* data, int length);
u32 packet_hash(struct Packet* packet);
bool packet_check_hash(struct Packet* packet);

// packet headers
void network_send_ack(struct Packet* p);
void network_receive_ack(struct Packet* p);
void network_remember_reliable(struct Packet* p);
void network_update_reliable(void);

void network_update_player(void);
void network_receive_player(struct Packet* p);

bool network_owns_object(struct Object* o);
void network_update_objects(void);
void network_send_object(struct Object* o);
void network_receive_object(struct Packet* p);

void network_send_spawn_objects(struct Object* objects[], u32 models[], u8 objectCount);
void network_receive_spawn_objects(struct Packet* p);

void network_send_spawn_star(struct Object* o, u8 starType, f32 x, f32 y, f32 z, u32 behParams);
void network_receive_spawn_star(struct Packet* p);

void network_send_level_warp(void);
void network_receive_level_warp(struct Packet* p);

void network_update_inside_painting(void);
void network_receive_inside_painting(struct Packet* p);

void network_send_collect_star(struct Object* o, s16 coinScore, s16 starIndex);
void network_receive_collect_star(struct Packet* p);

void network_send_collect_coin(struct Object* o);
void network_receive_collect_coin(struct Packet* p);

void network_send_collect_item(struct Object* o);
void network_receive_collect_item(struct Packet* p);
#endif
