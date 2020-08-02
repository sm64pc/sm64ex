#ifndef NETWORK_H
#define NETWORK_H

#include <types.h>
#include <assert.h>
#include "../cliopts.h"

#define MAX_SYNC_OBJECTS 256
#define PACKET_LENGTH 1024
#define NETWORKTYPESTR (networkType == NT_CLIENT ? "Client" : "Server")

extern struct MarioState gMarioStates[];
extern enum NetworkType networkType;
extern struct Object* syncObjects[];

enum PacketType {
    PACKET_PLAYER,
    PACKET_OBJECT
};

struct Packet {
    int cursor;
    bool error;
    char buffer[PACKET_LENGTH];
};

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
void network_send_player(void);
void network_receive_player(struct Packet* p);

void network_send_object(struct Object* o);
void network_receive_object(struct Packet* p);

#endif
