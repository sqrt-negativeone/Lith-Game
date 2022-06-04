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
internal b32 SendString(Socket_Handle s, String8 data);
internal b32 ReceiveString(Socket_Handle s, String8 *data);

#define NetworkSendValue(s, v) do {SendBuffer(s, &(v), sizeof(v));} while(0)
#define NetworkReceiveValue(s, v) do {ReceiveBuffer(s, &(v), sizeof(v));} while(0)

#define NetworkSendArray(s, arr, cnt, T) do {NetworkSendValue(s, cnt); SendBuffer(s, arr, (cnt) * sizeof(T));} while(0)
#define NetworkReceiveArray(s, arr, cnt, T) do { NetworkReceiveValue(s, cnt); ReceiveBuffer(s, arr, (cnt) * sizeof(T));} while(0)

#endif //NETWORK_UTILITIES_H
