#include <stdio.h>
#include "network.h"
#include "object_fields.h"
#include "object_constants.h"
#include "socket/socket.h"

enum NetworkType networkType;
static SOCKET gSocket;
struct sockaddr_in txAddr;

#define LOADING_LEVEL_THRESHOLD 10
u8 networkLoadingLevel = 0;
bool networkLevelLoaded = false;

void network_init(enum NetworkType inNetworkType, char* ip, char* port) {
    networkType = inNetworkType;

    if (networkType == NT_NONE) { return; }

    // Create a receiver socket to receive datagrams
    gSocket = socket_initialize();
    if (gSocket == INVALID_SOCKET) { return; }

    // Bind the socket to any address and the specified port.
    if (networkType == NT_SERVER) {
        int rc = socket_bind(gSocket, atoi(port));
        if (rc != NO_ERROR) { return; }
    } else {
        // Save the port to send to
        txAddr.sin_family = AF_INET;
        txAddr.sin_port = htons(atoi(port));
        txAddr.sin_addr.s_addr = inet_addr(ip);
    }
}

void network_on_init_level(void) {
    networkLoadingLevel = 0;
    networkLevelLoaded = false;
}

void network_on_loaded_level(void) {
    if (networkType == NT_CLIENT) {
        network_send_reservation_request();
    }
}

void network_send(struct Packet* p) {
    // sanity checks
    if (networkType == NT_NONE) { return; }
    if (p->error) { printf("%s packet error!\n", NETWORKTYPESTR); return; }

    // remember reliable packets
    network_remember_reliable(p);

    // save inside packet buffer
    u32 hash = packet_hash(p);
    memcpy(&p->buffer[p->dataLength], &hash, sizeof(u32));

    // send
    int rc = socket_send(gSocket, &txAddr, p->buffer, p->cursor + sizeof(u32));
    if (rc != NO_ERROR) { return; }
    p->sent = true;
}

void network_update(void) {
    if (networkType == NT_NONE) { return; }
    if (!networkLevelLoaded) {
        if (networkLoadingLevel++ >= LOADING_LEVEL_THRESHOLD) {
            networkLevelLoaded = true;
            network_on_loaded_level();
        }
    }

    if (gInsidePainting && sCurrPlayMode == PLAY_MODE_CHANGE_LEVEL) {
        network_update_inside_painting();
    } else if (sCurrPlayMode == PLAY_MODE_NORMAL) {
        network_update_player();
        network_update_objects();
    }

    do {
        // receive packet
        struct Packet p = { .cursor = 3 };
        int rc = socket_receive(gSocket, &txAddr, p.buffer, PACKET_LENGTH, &p.dataLength);
        if (rc != NO_ERROR) { break; }

        // subtract and check hash
        p.dataLength -= sizeof(u32);
        if (!packet_check_hash(&p)) {
            printf("Invalid packet!\n");
            continue;
        }

        // execute packet
        switch (p.buffer[0]) {
            case PACKET_ACK: network_receive_ack(&p); break;
            case PACKET_PLAYER: network_receive_player(&p); break;
            case PACKET_OBJECT: network_receive_object(&p); break;
            case PACKET_SPAWN_OBJECTS: network_receive_spawn_objects(&p); break;
            case PACKET_SPAWN_STAR: network_receive_spawn_star(&p); break;
            case PACKET_LEVEL_WARP: network_receive_level_warp(&p); break;
            case PACKET_INSIDE_PAINTING: network_receive_inside_painting(&p); break;
            case PACKET_COLLECT_STAR: network_receive_collect_star(&p); break;
            case PACKET_COLLECT_COIN: network_receive_collect_coin(&p); break;
            case PACKET_COLLECT_ITEM: network_receive_collect_item(&p); break;
            case PACKET_RESERVATION_REQUEST: network_receive_reservation_request(&p); break;
            case PACKET_RESERVATION: network_receive_reservation(&p); break;
            default: printf("%s received unknown packet: %d\n", NETWORKTYPESTR, p.buffer[0]);
        }

        // send an ACK if requested
        network_send_ack(&p);

    }  while (1);

    network_update_reliable();
}

void network_shutdown(void) {
    if (networkType == NT_NONE) { return; }
    socket_close(gSocket);
}
