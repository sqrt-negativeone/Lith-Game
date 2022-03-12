#include "network_message.cpp"

internal void
FetchHostsFromLobby(Socket_Handle lobby_socket, Hosts_Storage *hosts_storage)
{
    if (!ReceiveBuffer(lobby_socket, &hosts_storage->hosts_count, sizeof(hosts_storage->hosts_count)))
    {
        LogError("couldn'r read stuff from lobby");
        closesocket(lobby_socket);
        return;
    }
    
    hosts_storage->hosts_count = ntohl(hosts_storage->hosts_count);
    if (hosts_storage->hosts_count > ArrayCount(hosts_storage->hosts))
    {
        hosts_storage->hosts_count = ArrayCount(hosts_storage->hosts);
    }
    
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

struct Host_IO_Context
{
    Socket_Handle host_socket;
    MessageType message_type;
    WSABUF wsa_buf;
    WSAOVERLAPPED Overlapped;
};

global Host_IO_Context host_io_context;

internal void
ReceiveNextMessageType()
{
    DWORD flags = 0;
    if (WSARecv(host_io_context.host_socket,
                &host_io_context.wsa_buf,
                1,
                0,
                &flags,
                &host_io_context.Overlapped,
                0
                ) == SOCKET_ERROR)
    {
        int error = WSAGetLastError();
        if(error != WSA_IO_PENDING)
        {
            Log("Receive failed with error %d", error);
            Assert(!"See what is the problem");
        }
    }
}

internal void
HandlePlayerMessage(Message *message)
{
    switch(message->type)
    {
        case MessageType_From_Player_Fetch_Hosts:
        {
            Log("fetching hosts from lobby");
            Socket_Handle lobby_socket = ConnectToServer(LOBBY_ADDRESS, LOBBY_PLAYER_PORT);
            if (lobby_socket == INVALID_SOCKET)
            {
                int last_error = WSAGetLastError();
                // TODO(fakhri): for now let's assert
                Assert(false);
            }
            FetchHostsFromLobby(lobby_socket, message->hosts_storage);
            MemoryBarrier();
            message->hosts_storage->is_fetching = false;
            closesocket(lobby_socket);
        } break;
        case MessageType_From_Player_Connect_To_Host:
        {
            char *host_address = "127.0.0.1";
            host_io_context.host_socket = ConnectToServer(host_address, HOST_PORT);
            if(host_io_context.host_socket != INVALID_SOCKET)
            {
                CreateIoCompletionPort((HANDLE)host_io_context.host_socket, network_thread_iocp_handle, SERVER_MESSAGE, 1);
                ReceiveNextMessageType();
            }
            else
            {
                // NOTE(fakhri): probably the server is full?
            }
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
        case MessageType_From_Player_USERNAME:
        {
            SendBuffer(host_io_context.host_socket, message->players[0].username, sizeof(message->players[0].username));
        } break;
        default:
        {
            // NOTE(fakhri): unhandled message
            Assert(!"MESSAGE TYPE NOT IMPLEMENTED YET");
        }
    }
}

// NOTE(fakhri): this thread is responsible for network communication with the server
DWORD WINAPI NetworkMain(LPVOID lpParameter)
{
    host_io_context.wsa_buf.buf = (char *)&host_io_context.message_type;
    host_io_context.wsa_buf.len = sizeof(host_io_context.message_type);
    for(;;)
    {
        DWORD bytes_transferred;
        u64 completion_key;
        OVERLAPPED *Overlapped = 0;
        if (GetQueuedCompletionStatus(network_thread_iocp_handle, 
                                      &bytes_transferred,
                                      &completion_key,
                                      &Overlapped,
                                      INFINITE))
        {
            switch(completion_key)
            {
                case GAME_MESSAGE:
                {
                    for(MessageResult message_result = W32_GetNextMessageIfAvailable(&network_sending_queue);
                        message_result.is_available;
                        message_result = W32_GetNextMessageIfAvailable(&network_sending_queue)
                        )
                    {
                        HandlePlayerMessage(&message_result.message);
                    }
                } break;
                case SERVER_MESSAGE:
                {
                    // NOTE(fakhri): receiving message type completed
                    Message message = {};
                    message.type = host_io_context.message_type;
                    // NOTE(fakhri): read the message from the server
                    switch(host_io_context.message_type)
                    {
                        case MessageType_From_Host_Connected_Players_List:
                        {
                            ReceiveBuffer(host_io_context.host_socket, &message.players_count, sizeof(message.players_count));
                            for (u32 player_index = 0;
                                 player_index < message.players_count;
                                 ++player_index)
                            {
                                MessagePlayer *player = message.players + player_index;
                                ReceiveBuffer(host_io_context.host_socket, player->username, sizeof(player->username));
                            }
                        } break;
                        case MessageType_From_Host_New_Player_Joined:
                        {
                            ReceiveBuffer(host_io_context.host_socket, &message.player_id, sizeof(message.player_id));
                            MessagePlayer *player = message.players + message.player_id;
                            ReceiveBuffer(host_io_context.host_socket, player->username, sizeof(player->username));
                        } break;
                        case MessageType_From_Host_Shuffled_Deck:
                        {
                            ReceiveBuffer(host_io_context.host_socket, message.compact_deck, sizeof(message.compact_deck));
                        } break;
                        case MessageType_From_Host_Change_Player_Turn:
                        {
                            ReceiveBuffer(host_io_context.host_socket, &message.player_id, sizeof(message.player_id));
                        } break;
                        default :
                        {
                            Assert(!"MESSAGE TYPE NOT HANDLED YET");
                        }
                    }
                    W32_PushNetworkMessageToPlayer(message);
                    ReceiveNextMessageType();
                } break;
            }
        }
    }
}
