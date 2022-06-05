//#include "network_message.cpp"

internal void
FetchHostsFromLobby(Socket_Handle lobby_socket, Hosts_Storage *hosts_storage)
{
    if (!W32_ReceiveBuffer(lobby_socket, &hosts_storage->hosts_count, sizeof(hosts_storage->hosts_count)))
    {
        LogError("couldn't read stuff from lobby");
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
        case PlayerMessage_FetchHosts:
        {
            
#if 0            
            Log("fetching hosts from lobby");
            Socket_Handle lobby_socket = ConnectToServer(LOBBY_ADDRESS, LOBBY_PLAYER_PORT);
            if (lobby_socket == InvalidSocket)
            {
                int last_error = WSAGetLastError();
                Assert(!"NetworkThread::COULDN'T REACH LOBBY");
            }
            FetchHostsFromLobby(lobby_socket, message->hosts_storage);
            MemoryBarrier();
            message->hosts_storage->is_fetching = false;
            CloseSocket(lobby_socket);
#endif
        } break;
        case PlayerMessage_ConnectToHost:
        {
            char *host_address = "127.0.0.1";
            host_io_context.host_socket = W32_ConnectToServer(host_address, HOST_PORT);
            if(host_io_context.host_socket != InvalidSocket)
            {
                CreateIoCompletionPort((HANDLE)host_io_context.host_socket, network_thread_iocp_handle, NetworkMessageSource_Host, 1);
                ReceiveNextMessageType();
            }
            else
            {
                // NOTE(fakhri): probably the server is full?
            }
        } break;
        case PlayerMessage_Username:
        {
            Log("Username request");
            Log("username is %s", message->username.cstr);
            Assert(host_io_context.host_socket != InvalidSocket);
            W32_SendString(host_io_context.host_socket, message->username);
        } break;
        case PlayerMessage_PlayCard:
        {
            PlayerMoveKind move_kind = PlayerMove_PlayCard;
            NetworkSendValue(host_io_context.host_socket, move_kind);
            NetworkSendArray(host_io_context.host_socket,
                             message->player_move.actual_cards, 
                             message->player_move.played_cards_count, 
                             Compact_Card_Type);
            NetworkSendValue(host_io_context.host_socket, message->player_move.declared_number);
        } break;
        case PlayerMessage_QuestionCredibility:
        {
            PlayerMoveKind move_kind = PlayerMove_QuestionCredibility;
            NetworkSendValue(host_io_context.host_socket, move_kind);
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
            Log("Received network message");
            switch(completion_key)
            {
                case NetworkMessageSource_Player:
                {
                    Log("From Player");
                    while(!W32_IsMessageQueueEmpty(&player_message_queue))
                    {
                        // NOTE(fakhri): peek the head message
                        Message *message = W32_BeginMessageQueueRead(&player_message_queue);
                        HandlePlayerMessage(message);
                        // NOTE(fakhri): make the read visible to other threads
                        W32_EndMessageQueueRead(&player_message_queue);
                    }
                } break;
                case NetworkMessageSource_Host:
                {
                    Log("From host");
                    Message *message = W32_BeginMessageQueueWrite(&host_message_queue);
                    message->type = host_io_context.message_type;
                    // NOTE(fakhri): receive the actual message from the host
                    switch(host_io_context.message_type)
                    {
                        case HostMessage_PlayCard:
                        {
                            message->player_move.actual_cards = (Compact_Card_Type *)message->buffer;
                            NetworkReceiveArray(host_io_context.host_socket, 
                                                message->player_move.actual_cards, 
                                                message->player_move.played_cards_count, 
                                                Compact_Card_Type);
                            NetworkReceiveValue(host_io_context.host_socket, message->player_move.declared_number);
                        } break;
                        case HostMessage_QuestionCredibility:
                        {
                            // NOTE(fakhri): just type is enough
                        } break;
                        case HostMessage_ConnectedPlayersList:
                        {
                            Log("HostMessage_ConnectedPlayersList");
                            NetworkReceiveValue(host_io_context.host_socket, message->players_count);
                            
                            u32 offset = 0;
                            for (u32 player_index = 0;
                                 player_index < message->players_count;
                                 ++player_index)
                            {
                                String8 *player_username = message->players_usernames + player_index;
                                NetworkReceiveValue(host_io_context.host_socket, player_username->len);
                                Assert(offset + player_username->len < sizeof(message->buffer));
                                Assert(player_username->len < USERNAME_BUFFER_SIZE);
                                W32_ReceiveBuffer(host_io_context.host_socket, message->buffer + offset, (i32)player_username->len);
                                player_username->str = message->buffer + offset;
                                offset += (i32)player_username->len;
                            }
                        } break;
                        case HostMessage_NewPlayerJoined:
                        {
                            Log("HostMessage_NewPlayerJoined");
                            W32_ReceiveBuffer(host_io_context.host_socket, &message->new_player_id, sizeof(message->new_player_id));
                            W32_ReceiveBuffer(host_io_context.host_socket, &message->new_username.len, sizeof(message->new_username.len));
                            Assert(message->new_username.len < USERNAME_BUFFER_SIZE);
                            Assert(message->new_username.len < sizeof(message->buffer));
                            W32_ReceiveBuffer(host_io_context.host_socket, message->buffer, (i32)message->new_username.len);
                            message->new_username.str = message->buffer;
                            
                        } break;
                        case HostMessage_ShuffledDeck:
                        {
                            Log("HostMessage_ShuffledDeck");
                            message->compact_deck = (Compact_Card_Type *)message->buffer;
                            u32 compact_deck_size = DECK_CARDS_COUNT * sizeof(Compact_Card_Type);
                            Assert(compact_deck_size < sizeof(message->buffer));
                            W32_ReceiveBuffer(host_io_context.host_socket, message->compact_deck, compact_deck_size);
                        } break;
                        case HostMessage_ChangePlayerTurn:
                        {
                            Log("HostMessage_ChangePlayerTurn");
                            NetworkReceiveValue(host_io_context.host_socket, message->next_player_id);
                        } break;
                        default :
                        {
                            NotImplemented;
                        }
                    }
                    // NOTE(fakhri): make the changes visible to consumer threads
                    W32_EndMessageQueueWrite(&host_message_queue);
                    ReceiveNextMessageType();
                } break;
            }
        }
    }
}
