#ifndef SOCKET_H
#define SOCKET_H

#ifdef WINSOCK
#include "socket_windows.h"
#else
#include "socket_linux.h"
#endif

SOCKET socket_initialize(void);
int socket_bind(SOCKET sock, unsigned int port);
int socket_send(SOCKET sock, struct sockaddr_in* txAddr, char* buffer, int bufferLength);
int socket_receive(SOCKET sock, struct sockaddr_in* rxAddr, char* buffer, int bufferLength, int* receiveLength);
void socket_close(SOCKET sock);

#endif
