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

#define LOBBY_SERVER_HOSTS_PORT "12345"
#define LOBBY_SERVER_PLAYERS_PORT "42069"

#define WORKER_THREAD_MAX 16
#define U64_MAX ((u64)-1)
#define MAIN_THREAD_COMPLETION_KEY 0

#include <stdio.h>

typedef void WorkThreadWork(void *);

struct Queue_Entry
{
    WorkThreadWork *work;
    void *data;
};

struct Work_Queue
{
    Queue_Entry queue[512];
    u64 head;
    u64 tail;
    
    HANDLE waiting_worker_threads_semaphore;
    HANDLE producer_mutex;
};

struct Thread_Info
{
    u64 thread_id;
    Work_Queue *work_queue;
    
    struct Lobby_Context *lobby_context;
};

enum HostOperation
{
    HostOperation_WaitingMessage,
};

struct Connected_Host
{
    SOCKET socket;
    Host_Info host_info;
    WSABUF wsa_buf;
    char buffer[1];
    HostOperation operation;
};

struct Connected_Host_Queue
{
    Connected_Host queue[512];
    u64 head;
    u64 tail;
};

struct Connected_Hosts_Storage
{
    Connected_Host hosts[1024];
    u32 hosts_count;
    
    HANDLE hosts_mutex;
};

struct Lobby_Context
{
    Work_Queue work_queue;
    Thread_Info main_thread_info;
    Thread_Info hosts_thread_info;
    HANDLE hosts_iocp;
    
    Connected_Host_Queue connected_host_queue;
    Thread_Info worker_thread_infos[WORKER_THREAD_MAX];
    
    Connected_Hosts_Storage hosts_storage;
};

struct PlayerWorkInput
{
    SOCKET player_socket;
    Lobby_Context *lobby_context;
};

struct HostWorkInput
{
    u64 host_index;
    Lobby_Context *lobby_context;
};

internal b32
CompleteHostInfo(Connected_Host *connected_host)
{
    
    // NOTE(fakhri): read the hostname
    if (!ReceiveBuffer(connected_host->socket, connected_host->host_info.hostname, sizeof(connected_host->host_info.hostname)))
    {
        // NOTE(fakhri): an erro occured when reading from the socket
        closesocket(connected_host->socket);
        return false;
    }
    
    // NOTE(fakhri): read the port
    if (!ReceiveBuffer(connected_host->socket, &connected_host->host_info.port, sizeof(connected_host->host_info.port)))
    {
        // NOTE(fakhri): an erro occured when reading from the socket
        closesocket(connected_host->socket);
        return false;
    }
    
    return true;
}

internal void
AddHostInfo(Connected_Hosts_Storage *hosts_storage, HANDLE hosts_iocp, Connected_Host *connected_host_to_add)
{
    // TODO(fakhri): should we force the hostname to be unique?
    WaitForSingleObject(hosts_storage->hosts_mutex, INFINITE);
    if (hosts_storage->hosts_count < ArrayCount(hosts_storage->hosts))
    {
        u64 host_index = hosts_storage->hosts_count;
        Connected_Host *connected_host = hosts_storage->hosts + host_index;
        connected_host->socket = connected_host_to_add->socket;
        connected_host->host_info = connected_host_to_add->host_info;
        if (CompleteHostInfo(connected_host))
        {
            MemoryBarrier();
            ++hosts_storage->hosts_count;
            
            if (CreateIoCompletionPort((HANDLE)connected_host->socket, hosts_iocp, (ULONG_PTR)connected_host, 1))
            {
                connected_host->wsa_buf.buf = connected_host->buffer;
                connected_host->wsa_buf.len = sizeof(connected_host->buffer);
                
                WSAOVERLAPPED  Overlapped;
                ZeroMemory(&Overlapped, sizeof(WSAOVERLAPPED));
                DWORD flags = 0;
                
                WSARecv(connected_host->socket, 
                        &connected_host->wsa_buf,
                        1,
                        0,
                        &flags,
                        &Overlapped,
                        0
                        );
            }
            
        }
    }
    else
    {
        // TODO(fakhri): should we notify the host about the problem? or the host always assuem that ther is no room and it should try later
        closesocket(connected_host_to_add->socket);
    }
    ReleaseMutex(hosts_storage->hosts_mutex);
}

internal void
ServePlayer(void *data)
{
    PlayerWorkInput *input = (PlayerWorkInput *)data;
    Lobby_Context *lobby_context = input->lobby_context;
    Connected_Hosts_Storage *hosts_storage = &input->lobby_context->hosts_storage;
    
    WaitForSingleObject(hosts_storage->hosts_mutex, INFINITE);
    // NOTE(fakhri): send the hosts count
    u32 hosts_count_ip_order = htonl(hosts_storage->hosts_count);
    SendBuffer(input->player_socket, (char *)&hosts_count_ip_order, sizeof(hosts_count_ip_order));
    // NOTE(fakhri): send the hosts
    for(u64 host_index = 0;
        host_index < hosts_storage->hosts_count;
        ++host_index)
    {
        Connected_Host *connected_host = hosts_storage->hosts + host_index;
        if (!SendHostInfo(input->player_socket, &connected_host->host_info))
        {
            break;
        }
    }
    ReleaseMutex(hosts_storage->hosts_mutex);
    VirtualFree(input, sizeof(PlayerWorkInput), MEM_RELEASE);
    closesocket(input->player_socket);
}

internal void
ServeHost(void *data)
{
    HostWorkInput *input = (HostWorkInput *)data;
    Lobby_Context *lobby_context = input->lobby_context;
    Connected_Hosts_Storage *hosts_storage = &input->lobby_context->hosts_storage;
    Connected_Host *connected_host = hosts_storage->hosts + input->host_index;
    
    char message;
    recv(connected_host->socket, &message, sizeof(message), 0);
    if (message == 1)
    {
        
    }
    VirtualFree(input, sizeof(HostWorkInput), MEM_RELEASE);
}

internal b32
IsSocketQueueFull(Work_Queue *socket_queue)
{
    b32 result = ((socket_queue->tail - socket_queue->head) == ArrayCount(socket_queue->queue));
    return result;
}

internal inline b32
IsWorkQueueEmpty(Work_Queue *socket_queue)
{
    b32 result = (socket_queue->tail == socket_queue->head);
    return result;
}



internal b32
PushWorkQueueEntry(Work_Queue *work_queue, Queue_Entry work_entry)
{
    b32 result = false;
    
    WaitForSingleObject(work_queue->producer_mutex, INFINITE);
    if (work_queue->tail - work_queue->head < ArrayCount(work_queue->queue))
    {
        result = true;
        u64 entry_index = work_queue->tail % ArrayCount(work_queue->queue);
        work_queue->queue[entry_index] = work_entry;
        MemoryBarrier();
        ++work_queue->tail;
        
        ReleaseSemaphore(work_queue->waiting_worker_threads_semaphore, 1, 0);
    }
    ReleaseMutex(work_queue->producer_mutex);
    
    return result;
}

DWORD WINAPI WorkerThreadMain(LPVOID param)
{
    Thread_Info *thread_info = (Thread_Info *)param;
    Work_Queue *work_queue = thread_info->work_queue;;
    
    for(;;)
    {
        if (!IsWorkQueueEmpty(work_queue))
        {
            u64 old_head = work_queue->head;
            u64 head_index = old_head % ArrayCount(work_queue->queue);
            Queue_Entry work_entry = work_queue->queue[head_index];
            volatile u64 *destination = (volatile u64*)&work_queue->head;
            u64 new_head = old_head + 1;
            if (InterlockedCompareExchange(destination, new_head, old_head) != old_head)
            {
                // NOTE(fakhri): ops some other thread took the socket before us
                continue;
            }
            work_entry.work(work_entry.data);
        }
        else
        {
            // NOTE(fakhri): queue is empty
            WaitForSingleObject(work_queue->waiting_worker_threads_semaphore, INFINITE);
        }
    }
}

DWORD WINAPI HostsThreadMain(LPVOID param)
{
    Thread_Info *thread_info = (Thread_Info *) param;
    Work_Queue *work_queue = thread_info->work_queue;
    Lobby_Context *lobby_context = thread_info->lobby_context;
    for(;;)
    {
        DWORD bytes_transferred;
        u64 completion_key;
        OVERLAPPED *Overlapped = 0;
        if (GetQueuedCompletionStatus(lobby_context->hosts_iocp, 
                                      &bytes_transferred,
                                      &completion_key,
                                      &Overlapped,
                                      INFINITE))
        {
            if (completion_key)
            {
                Connected_Host *connected_host = (Connected_Host *)completion_key;
                
                if (connected_host->operation == HostOperation_WaitingMessage)
                {
                    // NOTE(fakhri): waiting message from server operatoin finished
                    if (bytes_transferred == 0)
                    {
                        Log("host %s closed", connected_host->host_info.hostname);
                        // NOTE(fakhri): connection closed, remove the host from the storage
                        Connected_Hosts_Storage *hosts_storage = &lobby_context->hosts_storage;
                        WaitForSingleObject(hosts_storage->hosts_mutex, INFINITE);
                        *connected_host = hosts_storage->hosts[hosts_storage->hosts_count - 1];
                        --hosts_storage->hosts_count;
                        ReleaseMutex(hosts_storage->hosts_mutex);
                        closesocket(connected_host->socket);
                    }
                    else
                    {
                        // TODO(fakhri): do we have any other possible message?
                        Assert(!"DIDN'T IMPLEMENT THIS CASE YET, SHOULD NOT HAPPEN");
                    }
                }
            }
            else
            {
                while(lobby_context->connected_host_queue.tail != lobby_context->connected_host_queue.head)
                {
                    
                    if(lobby_context->connected_host_queue.tail - lobby_context->connected_host_queue.head >= ArrayCount(lobby_context->connected_host_queue.queue))
                    {
                        Assert(!"WHAT THE FUCK IS GOIN ON HERE?");
                        Log("*sighs*");
                    }
                    
                    u64 host_index = lobby_context->connected_host_queue.head % ArrayCount(lobby_context->connected_host_queue.queue);
                    Connected_Host *connected_host = lobby_context->connected_host_queue.queue + host_index;
                    AddHostInfo(&lobby_context->hosts_storage, lobby_context->hosts_iocp, connected_host);
                    MemoryBarrier();
                    ++lobby_context->connected_host_queue.head;
                }
            }
        }
    }
}

internal void
InitLobbyContext(Lobby_Context *lobby_context)
{
    lobby_context->work_queue.producer_mutex = CreateMutexA(0, FALSE, 0);
    
    lobby_context->main_thread_info.thread_id = U64_MAX;
    lobby_context->main_thread_info.work_queue = &lobby_context->work_queue;
    
    u32 concurrent_threads_count = 1;
    lobby_context->hosts_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE,
                                                       0, 0,
                                                       concurrent_threads_count);
    
    u32 intial_count = 0;
    u32 max_count = WORKER_THREAD_MAX;
    
    lobby_context->work_queue.waiting_worker_threads_semaphore = CreateSemaphoreA(0, intial_count, max_count, 0);
    
    
    lobby_context->hosts_storage.hosts_mutex = CreateMutexA(0, FALSE, 0);
    
    
    // NOTE(fakhri): start worker threads
    for (u32 thread_index = 0;
         thread_index < WORKER_THREAD_MAX;
         ++thread_index)
    {
        Thread_Info *thread_info    = lobby_context->worker_thread_infos + thread_index;
        thread_info->thread_id      = thread_index;
        thread_info->work_queue     = &lobby_context->work_queue;
        thread_info->lobby_context = lobby_context;
        
        HANDLE worker_thread_handle = CreateThread(0, 0, WorkerThreadMain, thread_info, 0, 0);
        CloseHandle(worker_thread_handle);
    }
    Log("Started worker threads");
    
    lobby_context->hosts_thread_info.thread_id      = U64_MAX - 1;
    lobby_context->hosts_thread_info.work_queue     = &lobby_context->work_queue;
    lobby_context->hosts_thread_info.lobby_context = lobby_context;
    HANDLE hosts_thread_handle  = CreateThread(0, 0, HostsThreadMain, &lobby_context->hosts_thread_info, 0, 0);
    CloseHandle(hosts_thread_handle);
    Log("Started hosts thread");
    
    
}

int
WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR lp_cmd_line, int n_show_cmd)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) 
    {
        LogError("Couldn't init socket");
        PostQuitMessage(1);
    }
    Log("Initialized socket interface");
    
    Lobby_Context lobby_context = {};
    InitLobbyContext(&lobby_context);
    Log("Initialized lobby context");
    
    SOCKET listen_players_socket = OpenListenSocket(LOBBY_SERVER_PLAYERS_PORT);
    SOCKET listen_hosts_socket = OpenListenSocket(LOBBY_SERVER_HOSTS_PORT);
    
    if (listen_players_socket == INVALID_SOCKET || listen_hosts_socket == INVALID_SOCKET )
    {
        LogError("Couldn't open a socket to listen to");
        PostQuitMessage(1);
    }
    
    Log("opened listen sockets");
    
    fd_set ready_sockets;
    fd_set all_sockets;
    
    FD_ZERO(&ready_sockets);
    FD_ZERO(&all_sockets);
    
    FD_SET(listen_players_socket, &all_sockets);
    FD_SET(listen_hosts_socket, &all_sockets);
    
    for (;;)
    {
        ready_sockets = all_sockets;
        
        Log("blocking on select call");
        if (select(0, &ready_sockets, NULL, NULL, NULL) == SOCKET_ERROR)
        {
            LogError("socket failed");
            return 1;
        }
        
        Log("select call succeed");
        
        if (FD_ISSET(listen_hosts_socket, &ready_sockets))
        {
            Log("received host connection");
            sockaddr_in host_addr;
            i32 addrlen = sizeof(host_addr);
            
            SOCKET host_socket = accept(listen_hosts_socket, (sockaddr *)&host_addr, &addrlen);
            if (host_socket != INVALID_SOCKET)
            {
                Log("host connection accepted");
                Connected_Host connected_host = {};
                connected_host.socket = host_socket;
                inet_ntop(host_addr.sin_family, &host_addr.sin_addr, connected_host.host_info.ip, sizeof(connected_host.host_info.ip));
                
                Log("host ip address is %s", connected_host.host_info.ip);
                
                u64 hosts_queue_size = ArrayCount(lobby_context.connected_host_queue.queue); 
                if (lobby_context.connected_host_queue.tail - lobby_context.connected_host_queue.head < hosts_queue_size)
                {
                    u64 entry_index = lobby_context.connected_host_queue.tail % hosts_queue_size;
                    Connected_Host *queue_entry = lobby_context.connected_host_queue.queue + entry_index;
                    *queue_entry = connected_host;
                    MemoryBarrier();
                    ++lobby_context.connected_host_queue.tail;
                    
                    u64 completion_key = MAIN_THREAD_COMPLETION_KEY;
                    PostQueuedCompletionStatus(lobby_context.hosts_iocp, 0, completion_key, 0);
                }
                else
                {
                    // NOTE(fakhri): queue is full, we process it ourselves
                    AddHostInfo(&lobby_context.hosts_storage, lobby_context.hosts_iocp,  &connected_host);
                }
            }
            else
            {
                int last_error = WSAGetLastError();
                Log("Invalid address, error number %d", last_error);
                
            }
        }
        
        if (FD_ISSET(listen_players_socket, &ready_sockets))
        {
            Log("received player connection");
            
            SOCKET player_socket = accept(listen_players_socket, 0, 0);
            if (player_socket != INVALID_SOCKET)
            {
                Log("accepted player connection");
                
                Queue_Entry work_entry = {};
                PlayerWorkInput *player_work_input = (PlayerWorkInput *)VirtualAlloc(0, sizeof(PlayerWorkInput), MEM_COMMIT, PAGE_READWRITE);
                
                player_work_input->player_socket = player_socket;
                player_work_input->lobby_context = &lobby_context;
                
                work_entry.work = ServePlayer;
                work_entry.data = player_work_input;
                
                if(!PushWorkQueueEntry(&lobby_context.work_queue, work_entry))
                {
                    // NOTE(fakhri): queue is full we do the work ourselves
                    work_entry.work(work_entry.data);
                }
            }
        }
        else
        {
            Log("invalide player socket");
        }
    }
}