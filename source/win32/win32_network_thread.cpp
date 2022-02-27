#include "network_message.cpp"

global SOCKET lobby_socket;
global SOCKET host_socket;

internal void
FetchHostsFromLobby(Hosts_Storage *hosts_storage)
{
    if (!ReceiveBuffer(lobby_socket, &hosts_storage->hosts_count, sizeof(hosts_storage->hosts_count)))
    {
        LogError("couldn'r read stuff from lobby");
        closesocket(lobby_socket);
        return;
    }
    
    hosts_storage->hosts_count = ntohl(hosts_storage->hosts_count);
    
    u32 host_index = 0;
    while(host_index < hosts_storage->hosts_count)
    {
        Host_Info *host_info = hosts_storage->hosts + host_index;
        if (ReceiveHostInfo(lobby_socket, host_info))
        {
            ++host_index;
        }
        else
        {
            --hosts_storage->hosts_count;
        }
    }
    
}

internal void
HandlePlayerMessage(NetworkMessage *message)
{
    switch(message->type)
    {
        case NetworkMessageType_From_Player_FETCH_HOSTS:
        {
            Log("fetching hosts from lobby");
            lobby_socket = ConnectToServer(LOBBY_ADDRESS, LOBBY_PORT);
            if (lobby_socket == INVALID_SOCKET)
            {
                int last_error = WSAGetLastError();
                // TODO(fakhri): for now let's assert
                Assert(false);
            }
            FetchHostsFromLobby(message->hosts_storage);
            MemoryBarrier();
            message->hosts_storage->is_fetching = false;
            closesocket(lobby_socket);
        } break;
        case NetworkMessageType_From_Player_CONNECT_TO_SERVER:
        {
#if 0
            *connect_socket = ConnectToServer(message->server_address);
            if (*connect_socket != INVALID_SOCKET)
            {
                CreateIoCompletionPort((HANDLE)*connect_socket, global_iocp_handle, SERVER_MESSAGE, 1);
                Log("Network Thread: Connected to server at address %s", message->server_address.str);
            }
            else
            {
                W32_PushNetworkMessageToPlayer(CreateCouldNotConnectToServerMessage());
                Log("Network Thread: Coouldn't Connect to server at address %s", message->server_address.str);
            }
#endif
        } break;
        default:
        {
            // NOTE(fakhri): unhandled message
            Assert(false);
        }
    }
}

// NOTE(fakhri): this thread is responsible for network communication with the server
DWORD WINAPI NetworkMain(LPVOID lpParameter)
{
    
    for(;;)
    {
        DWORD bytes_transferred;
        u64 completion_key;
        OVERLAPPED *Overlapped = 0;
        if (GetQueuedCompletionStatus(global_iocp_handle, 
                                      &bytes_transferred,
                                      &completion_key,
                                      &Overlapped,
                                      INFINITE))
        {
            switch(completion_key)
            {
                case GAME_MESSAGE:
                {
                    
                    for(NetworkMessageResult message_result = W32_GetNextMessageIfAvailable(&global_network_sending_queue);
                        message_result.is_available;
                        message_result = W32_GetNextMessageIfAvailable(&global_network_sending_queue)
                        )
                    {
                        HandlePlayerMessage(&message_result.message);
                    }
                } break;
                case SERVER_MESSAGE:
                {
                    // TODO(fakhri): we have a message from the server
                    // TODO(fakhri): handle the request
                    // TODO(fakhri): insert the server message into the network in queue
                    
                } break;
            }
        }
    }
}
