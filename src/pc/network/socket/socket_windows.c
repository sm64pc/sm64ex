#ifdef WINSOCK
#include "socket_windows.h"
#include "../network.h"

SOCKET socket_initialize(void) {
    // start up winsock
    WSADATA wsaData;
    int rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc != NO_ERROR) {
        printf("%s WSAStartup failed with error %d\n", NETWORKTYPESTR, rc);
        return INVALID_SOCKET;
    }

    // initialize socket
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        printf("%s socket failed with error %d\n", NETWORKTYPESTR, SOCKET_LAST_ERROR);
        return INVALID_SOCKET;
    }

    // set non-blocking mode
    u_long iMode = 1;
    rc = ioctlsocket(sock, FIONBIO, &iMode);
    if (rc != NO_ERROR) {
        printf("%s ioctlsocket failed with error: %d\n", NETWORKTYPESTR, rc);
        return INVALID_SOCKET;
    }

    return sock;
}

void socket_close(SOCKET sock) {
    int rc = closesocket(sock);
    if (rc == SOCKET_ERROR) {
        printf("%s closesocket failed with error %d\n", NETWORKTYPESTR, SOCKET_LAST_ERROR);
    }
    WSACleanup();
}

#endif
