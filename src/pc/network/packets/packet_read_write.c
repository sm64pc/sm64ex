#include "../network.h"

void packet_init(struct Packet* packet, enum PacketType packetType) {
    memset(packet->buffer, 0, PACKET_LENGTH);
    packet->buffer[0] = (char)packetType;
    packet->cursor = 1;
    packet->error = false;
}

void packet_write(struct Packet* packet, void* data, int length) {
    if (data == NULL) { packet->error = true; return; }
    memcpy(&packet->buffer[packet->cursor], data, length);
    packet->cursor += length;
}

void packet_read(struct Packet* packet, void* data, int length) {
    if (data == NULL) { packet->error = true; return; }
    memcpy(data, &packet->buffer[packet->cursor], length);
    packet->cursor += length;
}
