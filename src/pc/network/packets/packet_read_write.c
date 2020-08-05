#include "../network.h"

static u16 nextSeqNum = 1;
void packet_init(struct Packet* packet, enum PacketType packetType, bool reliable) {
    memset(packet->buffer, 0, PACKET_LENGTH);
    packet->buffer[0] = (char)packetType;
    if (reliable) {
        memcpy(&packet->buffer[1], &nextSeqNum, 2);
        packet->seqId = nextSeqNum;
        nextSeqNum++;
        if (nextSeqNum == 0) { nextSeqNum++;  }
    }
    packet->cursor = 3;
    packet->error = false;
    packet->reliable = reliable;
    packet->sent = false;
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
