#ifndef WINSOCK
#include "socket_linux.h"
#include "../network.h"

SOCKET socket_initialize(void) {
    // initialize socket
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        printf("%s socket failed with error %d\n", NETWORKTYPESTR, SOCKET_LAST_ERROR);
        return INVALID_SOCKET;
    }

    // set non-blocking mode
    int rc = fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);
    if (rc == INVALID_SOCKET) {
        printf("%s fcntl failed with error: %d\n", NETWORKTYPESTR, rc);
        return INVALID_SOCKET;
    }

    return sock;
}

void socket_close(SOCKET sock) {
    int rc = closesocket(sock);
    if (rc == SOCKET_ERROR) {
        printf("%s closesocket failed with error %d\n", NETWORKTYPESTR, SOCKET_LAST_ERROR);
    }
}

#endif
