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
    packet->dataLength = 3;
    packet->cursor = 3;
    packet->error = false;
    packet->reliable = reliable;
    packet->sent = false;
}

void packet_write(struct Packet* packet, void* data, int length) {
    if (data == NULL) { packet->error = true; return; }
    memcpy(&packet->buffer[packet->cursor], data, length);
    packet->dataLength += length;
    packet->cursor += length;
}

void packet_read(struct Packet* packet, void* data, int length) {
    if (data == NULL) { packet->error = true; return; }
    int cursor = packet->cursor;
    memcpy(data, &packet->buffer[cursor], length);
    packet->cursor = cursor + length;
}

u32 packet_hash(struct Packet* packet) {
    u32 hash = 0;
    int byte = 0;
    for (int i = 0; i < packet->dataLength; i++) {
        hash ^= ((u32)packet->buffer[i]) << (8 * byte);
        byte = (byte + 1) % sizeof(u32);
    }
    return hash;
}

bool packet_check_hash(struct Packet* packet) {
    u32 localHash = packet_hash(packet);
    u32 packetHash = 0;
    memcpy(&packetHash, &packet->buffer[packet->dataLength], sizeof(u32));
    return localHash == packetHash;
}
