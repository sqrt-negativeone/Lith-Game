/* date = February 26th 2022 0:00 pm */

#ifndef NETWORK_UTILITIES_H
#define NETWORK_UTILITIES_H

#include <winsock2.h>
#include <ws2tcpip.h>

#define LISTENQ 1024
typedef SOCKET Socket_Handle;

#define Accept(fd, addr, addrlen) accept(fd, addr, addrlen)
#define CloseSocket(fd) closesocket(fd)
internal Socket_Handle ConnectToServer(char *server_address, char *port);
internal Socket_Handle OpenListenSocket(char *port);
internal b32 SendBuffer(Socket_Handle s, void *data, i32 len);
internal b32 ReceiveBuffer(Socket_Handle s, void *data, i32 len);

#endif //NETWORK_UTILITIES_H
