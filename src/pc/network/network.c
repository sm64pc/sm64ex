#include <stdio.h>
#include "network.h"
#include "object_fields.h"
#include "object_constants.h"

// Winsock includes
#include <winsock2.h>
#include <Ws2tcpip.h>
//#pragma comment(lib, "Ws2_32.lib")

//////////////////////////
// TODO: port to linux! //
//////////////////////////

enum NetworkType networkType;
SOCKET gSocket;
unsigned short txPort;

void network_init(enum NetworkType inNetworkType) {
    networkType = inNetworkType;
    if (networkType == NT_NONE) { return; }

    //-----------------------------------------------
    // Initialize Winsock
    WSADATA wsaData;
    int rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc != NO_ERROR) {
        wprintf(L"%s WSAStartup failed with error %d\n", NETWORKTYPESTR, rc);
        return;
    }
    //-----------------------------------------------
    // Create a receiver socket to receive datagrams
    gSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (gSocket == INVALID_SOCKET) {
        wprintf(L"%s socket failed with error %d\n", NETWORKTYPESTR, WSAGetLastError());
        return;
    }

    // Set non-blocking mode
    u_long iMode = 1;
    rc = ioctlsocket(gSocket, FIONBIO, &iMode);
    if (rc != NO_ERROR) {
        printf("%s ioctlsocket failed with error: %ld\n", NETWORKTYPESTR, rc);
    }

    // Bind the socket to any address and the specified port.
    struct sockaddr_in rxAddr;
    rxAddr.sin_family = AF_INET;
    rxAddr.sin_port = htons(networkType == NT_SERVER ? 27015 : 27016);
    rxAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    rc = bind(gSocket, (SOCKADDR *)& rxAddr, sizeof(rxAddr));
    if (rc != 0) {
        wprintf(L"%s bind failed with error %d\n", NETWORKTYPESTR, WSAGetLastError());
        return;
    }

    // Save the port to send to
    txPort = htons(networkType == NT_SERVER ? 27016 : 27015);
}

void network_send(struct Packet* p) {
    if (networkType == NT_NONE) { return; }
    if (p->error) { printf("%s packet error!\n", NETWORKTYPESTR); return; }
    struct sockaddr_in txAddr;
    txAddr.sin_family = AF_INET;
    txAddr.sin_port = txPort;
    txAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    network_remember_reliable(p);

    int rc = sendto(gSocket, p->buffer, p->cursor, 0, (SOCKADDR *)& txAddr, sizeof(txAddr));
    if (rc == SOCKET_ERROR) {
        wprintf(L"%s sendto failed with error: %d\n", NETWORKTYPESTR, WSAGetLastError());
        return;
    }

    p->sent = true;
}

void network_update(void) {
    if (networkType == NT_NONE) { return; }

    // TODO: refactor the way we do these update functions, it will get messy quick
    if (gInsidePainting && sCurrPlayMode == PLAY_MODE_CHANGE_LEVEL) {
        network_update_inside_painting();
    } else if (sCurrPlayMode == PLAY_MODE_SYNC_LEVEL) {
        network_update_level_warp();
    } else {
        network_update_player();
        network_update_objects();
    }

    do {
        struct sockaddr_in rxAddr;
        int rxAddrSize = sizeof(rxAddr);
        struct Packet p = { .cursor = 3 };
        int rc = recvfrom(gSocket, p.buffer, PACKET_LENGTH, 0, (SOCKADDR *)&rxAddr, &rxAddrSize);
        if (rc == SOCKET_ERROR) {
            int error = WSAGetLastError();
            if (error != WSAEWOULDBLOCK && error != WSAECONNRESET) {
                wprintf(L"%s recvfrom failed with error %d\n", NETWORKTYPESTR, WSAGetLastError());
            }
            break;
        }
        if (rc == 0) { break; }

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
            default: printf("%s received unknown packet: %d\n", NETWORKTYPESTR, p.buffer[0]);
        }

        // send an ACK if requested
        network_send_ack(&p);

    }  while (1);

    network_update_reliable();
}

void network_shutdown(void) {
    if (networkType == NT_NONE) { return; }
    int rc = closesocket(gSocket);
    if (rc == SOCKET_ERROR) {
        wprintf(L"%s closesocket failed with error %d\n", NETWORKTYPESTR, WSAGetLastError());
    }
    WSACleanup();
}
