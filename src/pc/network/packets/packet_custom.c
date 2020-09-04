#include <stdio.h>
#include "../network.h"

#define MAX_CUSTOM_PACKETS 32

struct NetworkCustomPacket {
    void (*send_callback)(struct Packet* p, void* params);
    void (*receive_callback)(struct Packet* p);
};

static u8 onCustomPacketId = 0;
static struct NetworkCustomPacket customPackets[MAX_CUSTOM_PACKETS];

u8 network_register_custom_packet(void (*send_callback)(struct Packet* p, void* params), void (*receive_callback)(struct Packet* p)) {
    assert(onCustomPacketId < MAX_CUSTOM_PACKETS);

    u8 i = onCustomPacketId;
    customPackets[i].send_callback = send_callback;
    customPackets[i].receive_callback = receive_callback;

    onCustomPacketId++;
    return i;
}

void network_send_custom(u8 customId, bool reliable, void* params) {
    if (customPackets[customId].send_callback == NULL) { return; }

    struct Packet p;
    packet_init(&p, PACKET_CUSTOM, reliable);
    packet_write(&p, &customId, sizeof(u8));
    customPackets[customId].send_callback(&p, params);
    network_send(&p);
}

void network_receive_custom(struct Packet* p) {
    u8 customId;
    packet_read(p, &customId, sizeof(u8));
    if (customPackets[customId].receive_callback == NULL) { return; }
    customPackets[customId].receive_callback(p);
}
