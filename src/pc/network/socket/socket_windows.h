#ifndef SOCKET_WINDOWS_H
#define SOCKET_WINDOWS_H

#include <winsock2.h>
#include <Ws2tcpip.h>

#define SOCKET_LAST_ERROR WSAGetLastError()
#define EWOULDBLOCK WSAEWOULDBLOCK
#define ECONNRESET WSAECONNRESET

#endif
