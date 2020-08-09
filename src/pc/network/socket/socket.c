#include "socket.h"
#include "../network.h"

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

int socket_send(SOCKET sock, char* ip, unsigned int port, char* buffer, int bufferLength) {
    struct sockaddr_in txAddr;
    txAddr.sin_family = AF_INET;
    txAddr.sin_port = htons(port);
    txAddr.sin_addr.s_addr = inet_addr(ip);

    int rc = sendto(sock, buffer, bufferLength, 0, (SOCKADDR*)&txAddr, sizeof(txAddr));
    if (rc == SOCKET_ERROR) {
        printf("%s sendto failed with error: %d\n", NETWORKTYPESTR, SOCKET_LAST_ERROR);
    }

    return rc;
}

int socket_receive(SOCKET sock, char* buffer, int bufferLength, int* receiveLength) {
    *receiveLength = 0;

    struct sockaddr_in rxAddr;
    int rxAddrSize = sizeof(rxAddr);

    int rc = recvfrom(sock, buffer, bufferLength, 0, (SOCKADDR*)&rxAddr, &rxAddrSize);
    if (rc == SOCKET_ERROR) {
        int error = SOCKET_LAST_ERROR;
        if (error != EWOULDBLOCK && error != ECONNRESET) {
            printf("%s recvfrom failed with error %d\n", NETWORKTYPESTR, SOCKET_LAST_ERROR);
        }
        return rc;
    }

    *receiveLength = rc;
    return NO_ERROR;
}
