//////////////////////////
// TODO: port to linux! //
//////////////////////////

#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

#include "network.h"

#define NETWORKTYPESTR (networkType == NT_CLIENT ? "Client" : "Server")
enum NetworkType networkType;
SOCKET gSocket;
#define BUFFER_LENGTH 1024
unsigned short txPort;
char* txIpAddr = "127.0.0.1";

struct MarioState *marioStates = NULL;

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

    if (networkType == NT_SERVER) {
        //-----------------------------------------------
        // Bind the socket to any address and the specified port.
        struct sockaddr_in rxAddr;
        rxAddr.sin_family = AF_INET;
        rxAddr.sin_port = htons(27015);
        rxAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        rc = bind(gSocket, (SOCKADDR *)& rxAddr, sizeof(rxAddr));
        if (rc != 0) {
            wprintf(L"%s bind failed with error %d\n", NETWORKTYPESTR, WSAGetLastError());
            return;
        }
    }
    else if (networkType == NT_CLIENT) {
        txPort = htons(27015);
    }
}

void network_track(struct MarioState *inMarioStates) {
    marioStates = inMarioStates;
}

void network_send(char* buffer, int buffer_length) {
    if (networkType == NT_NONE) { return; }
    struct sockaddr_in txAddr;
    txAddr.sin_family = AF_INET;
    txAddr.sin_port = txPort;
    txAddr.sin_addr.s_addr = inet_addr(txIpAddr);

    int rc = sendto(gSocket, buffer, buffer_length, 0, (SOCKADDR *)& txAddr, sizeof(txAddr));
    if (rc == SOCKET_ERROR) {
        wprintf(L"%s sendto failed with error: %d\n", NETWORKTYPESTR, WSAGetLastError());
        closesocket(gSocket);
        WSACleanup();
        return;
    }
}

void network_update(void) {
    if (networkType == NT_NONE) { return; }
    char buffer[BUFFER_LENGTH];
    struct sockaddr_in rxAddr;
    int rxAddrSize = sizeof(rxAddr);

    if (marioStates != NULL) {
        memcpy(&buffer[0], &marioStates[0], 96);
        memcpy(&buffer[96], marioStates[0].controller, 20);
        memcpy(&buffer[96 + 20], marioStates[0].marioObj->rawData.asU32, 320);
        network_send(buffer, 96 + 20 + 320);
    }

    do {
        int rc = recvfrom(gSocket, buffer, BUFFER_LENGTH, 0, (SOCKADDR *)&rxAddr, &rxAddrSize);
        if (rc == SOCKET_ERROR) {
            int error = WSAGetLastError();
            if (error != WSAEWOULDBLOCK && error != WSAECONNRESET) {
                wprintf(L"%s recvfrom failed with error %d\n", NETWORKTYPESTR, WSAGetLastError());
            }
            return;
        }
        if (networkType == NT_SERVER) { txPort = rxAddr.sin_port; }

        if (rc != 96 + 20 + 320) {
            printf("%s received error: %s\n", NETWORKTYPESTR, rc);
            break;
        } else if (marioStates != NULL) {
            int oldActionState = marioStates[1].actionState;
            memcpy(&marioStates[1], &buffer[0], 96);
            memcpy(marioStates[1].controller, &buffer[96], 20);
            memcpy(&marioStates[1].marioObj->rawData.asU32, &buffer[96 + 20], 320);
            marioStates[1].actionState = oldActionState;
        }
    }  while (1);

}

void network_shutdown(void) {
    if (networkType == NT_NONE) { return; }
    int rc = closesocket(gSocket);
    if (rc == SOCKET_ERROR) {
        wprintf(L"%s closesocket failed with error %d\n", NETWORKTYPESTR, WSAGetLastError());
    }
    WSACleanup();
}