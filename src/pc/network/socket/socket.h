#ifndef SOCKET_H
#define SOCKET_H

#ifdef WINSOCK
#include "socket_windows.h"
#else
#include "socket_linux.h"
#endif

SOCKET socket_initialize(void);
int socket_bind(SOCKET sock, unsigned int port);
int socket_send(SOCKET sock, char* ip, unsigned int port, char* buffer, int bufferLength);
int socket_receive(SOCKET sock, char* buffer, int bufferLength, int* receiveLength);
void socket_close(SOCKET sock);

#endif
