
// NOTE(fakhri): windows shenanigans
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef DONT_SHOW_ASSERTION_MESSAGE_WINDOW
#define DONT_SHOW_ASSERTION_MESSAGE_WINDOW
#endif

#include <windows.h>
#include <windowsx.h>

#include "language_layer.h"
#include "language_layer.c"

#include "network_shared/network_utilities.h"
#include "network_shared/network_utilities.cpp"

#include "network_shared/host_info.h"
#include "network_shared/host_info.cpp"

#define LOBBY_ADDRESS "127.0.0.1"
#define LOBBY_PLAYER_PORT "42069"
#define LOBBY_HOST_PORT "12345"
#define WORKER_THREAD_MAX 16

struct Hosts_Storage
{
    // TODO(fakhri): how many do we need to display?
    b32 is_fetching;
    Host_Info hosts[10];
    u32 hosts_count;
};

#define _CRT_RAND_S  
#include <stdlib.h>  
#include <stdio.h>

#include "time.h"

#include <ntsecapi.h>

DWORD WINAPI TestPlayerMain(LPVOID param)
{
    
    u32 seed = (u32)time(0);
    
    for (;;)
    {
        Log("Player connecting to lobby\n");
        SOCKET lobby_socket = ConnectToServer(LOBBY_ADDRESS, LOBBY_PLAYER_PORT);
        Hosts_Storage hosts_storage = {};
        if (!ReceiveBuffer(lobby_socket, &hosts_storage.hosts_count, sizeof(hosts_storage.hosts_count)))
        {
            
            closesocket(lobby_socket);
            return 1;
        }
        
        hosts_storage.hosts_count = ntohl(hosts_storage.hosts_count);
        Log("Player receiving %u hosts\n", hosts_storage.hosts_count);
        
        u32 host_index = 0;
        while(host_index < hosts_storage.hosts_count)
        {
            Host_Info *host_info = hosts_storage.hosts + host_index;
            if (ReceiveHostInfo(lobby_socket, host_info))
            {
                ++host_index;
            }
            else
            {
                --hosts_storage.hosts_count;
            }
        }
        closesocket(lobby_socket);
        Log("Player hosts finished\n");
    }
}

DWORD WINAPI TestHostServer(LPVOID param)
{
    for (;;)
    {
        SOCKET lobby_socket = ConnectToServer(LOBBY_ADDRESS, LOBBY_HOST_PORT);;
        Log("Host connected to lobby");
        char hostname[20] = "hostname";
        char port[6] = "12345";
        SendBuffer(lobby_socket, hostname, sizeof(hostname));
        SendBuffer(lobby_socket, port, sizeof(port));
        closesocket(lobby_socket);
        Log("Host connection with lobby closed");
    }
}



int
main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) 
    {
        return 1;
    }
    
#if 1    
    for (u32 thread_index = 0;
         thread_index < WORKER_THREAD_MAX / 2;
         ++thread_index)
    {
        HANDLE worker_thread_handle = CreateThread(0, 0, TestHostServer, 0, 0, 0);
        CloseHandle(worker_thread_handle);
    }
#endif
    
    for (u32 thread_index = 0;
         thread_index < WORKER_THREAD_MAX / 2;
         ++thread_index)
    {
        HANDLE worker_thread_handle = CreateThread(0, 0, TestPlayerMain, 0, 0, 0);
        CloseHandle(worker_thread_handle);
    }
    TestPlayerMain(0);
    
    return 0;
}