#include <stdio.h>
#include "../network.h"
#include "socket.h"

int socket_bind(SOCKET sock, unsigned int port) {
    struct sockaddr_in rxAddr;
    rxAddr.sin_family = AF_INET;
    rxAddr.sin_port = htons(port);
    rxAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int rc = bind(sock, (SOCKADDR*)&rxAddr, sizeof(rxAddr));
    if (rc != 0) {
        printf("%s bind failed with error %d\n", NETWORKTYPESTR, SOCKET_LAST_ERROR);
    }

    return rc;
}

int socket_send(SOCKET sock, struct sockaddr_in* txAddr, u8* buffer, u16 bufferLength) {
    int txAddrSize = sizeof(struct sockaddr_in);
    int rc = sendto(sock, (char*)buffer, bufferLength, 0, (struct sockaddr*)txAddr, txAddrSize);
    if (rc == SOCKET_ERROR) {
        printf("%s sendto failed with error: %d\n", NETWORKTYPESTR, SOCKET_LAST_ERROR);
    }

    return rc;
}

int socket_receive(SOCKET sock, struct sockaddr_in* rxAddr, u8* buffer, u16 bufferLength, u16* receiveLength) {
    *receiveLength = 0;

    int rxAddrSize = sizeof(struct sockaddr_in);
    int rc = recvfrom(sock, (char*)buffer, bufferLength, 0, (struct sockaddr*)rxAddr, &rxAddrSize);
    if (rc == SOCKET_ERROR) {
        int error = SOCKET_LAST_ERROR;
        if (error != SOCKET_EWOULDBLOCK && error != SOCKET_ECONNRESET) {
            printf("%s recvfrom failed with error %d\n", NETWORKTYPESTR, SOCKET_LAST_ERROR);
        }
        return rc;
    }

    *receiveLength = rc;
    return NO_ERROR;
}
