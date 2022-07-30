//#include "network_message.cpp"

struct NetworkIOContext
{
    M_Arena *arena;
    String8 game_id;
    Socket_Handle lobby_socket;
    Socket_Handle host_socket;
    MessageType message_type;
    
    WSABUF message_type_wsa_buf;
    WSAOVERLAPPED message_type_overlapped;
};

global NetworkIOContext network_io_context;

internal void
ReceiveNextMessageType()
{
    DWORD flags = 0;
    if (WSARecv(network_io_context.host_socket,
                &network_io_context.message_type_wsa_buf,
                1,
                0,
                &flags,
                &network_io_context.message_type_overlapped,
                0
                ) == SOCKET_ERROR)
    {
        int error = WSAGetLastError();
        if(error != WSA_IO_PENDING)
        {
            Log("Receive failed with error %d", error);
            InvalidCodePath;
        }
    }
}

internal b32
ConnectToHost(char *host_address)
{
    b32 result = false;
    network_io_context.host_socket = W32_ConnectToServer(host_address, HOST_PORT);
    if(network_io_context.host_socket != InvalidSocket)
    {
        result = true;
        CreateIoCompletionPort((HANDLE)network_io_context.host_socket, network_thread_iocp_handle, NetworkMessageSource_Host, 1);
        ReceiveNextMessageType();
    }
    else
    {
        // NOTE(fakhri): probably the host is full?
    }
    return result;
}

internal void
HandlePlayerMessage(Message *message)
{
    char *lobby_address = "127.0.0.1";
    switch(message->type)
    {
        case PlayerMessage_CancelJoin:
        {
            os->CloseSocket(network_io_context.host_socket);
        } break;
        case PlayerMessage_CancelHost:
        {
            os->StopGameHost();
            
            Socket_Handle s = os->ConnectToServer("127.0.0.1", HOST_PORT);
            if (s != InvalidSocket)
            {
                os->CloseSocket(s);
            }
        } break;
        case PlayerMessage_HostGameSession:
        {
            // NOTE(fakhri): start the host
            b32 io_successed = true;
            Socket_Handle LobbySocket = W32_ConnectToServer(lobby_address, LOBBY_HOST_PORT);
            if (LobbySocket != InvalidSocket)
            {
                network_io_context.lobby_socket = LobbySocket;
                u64 game_id_size;
                NetworkReceiveValue(LobbySocket, game_id_size, io_successed);
                if (io_successed)
                {
                    if (!network_io_context.game_id.str)
                    {
                        network_io_context.game_id.str = PushArrayZero(network_io_context.arena, u8, game_id_size);
                        network_io_context.game_id.size = game_id_size;
                    }
                    
                    Assert(game_id_size == network_io_context.game_id.size);
                    
                    io_successed &= os->ReceiveBuffer(LobbySocket, network_io_context.game_id.str, (u32)game_id_size);
                    if (io_successed)
                    {
                        Message *message = W32_BeginMessageQueueWrite(&network_message_queue);
                        message->type = NetworkMessage_GameID;
                        Assert(network_io_context.game_id.size <= ArrayCount(message->buffer));
                        MemoryCopy(message->buffer, network_io_context.game_id.str, game_id_size);
                        message->game_id = Str8(message->buffer, game_id_size);
                        W32_EndMessageQueueWrite(&network_message_queue);
                        io_successed &= ConnectToHost("127.0.0.1");
                    }
                }
                
                if (!io_successed)
                {
                    W32_CloseSocket(LobbySocket);
                }
            }
            
            if (!io_successed)
            {
                Message *message = W32_BeginMessageQueueWrite(&network_message_queue);
                message->type = NetworkMessage_FailedToHost;
                W32_EndMessageQueueWrite(&network_message_queue);
            }
            
        } break;
        case PlayerMessage_JoinGameSession:
        {
            b32 io_successed = true;
            Socket_Handle LobbySocket = W32_ConnectToServer(lobby_address, LOBBY_PLAYER_PORT);
            NetworkSendString(LobbySocket, message->game_id, io_successed);
            
            u32 host_address;
            NetworkReceiveValue(LobbySocket, host_address, io_successed);
            if (io_successed)
            {
                Log("Received address is %d", host_address);
                struct in_addr ip_addr;
                ip_addr.S_un.S_addr = host_address;
                char *str_address = inet_ntoa(ip_addr);
                Log("address as string is %s", str_address);
                io_successed &= ConnectToHost(str_address);
                
                if (io_successed)
                {
                    Message *message = W32_BeginMessageQueueWrite(&network_message_queue);
                    message->type = NetworkMessage_JoinedGame;
                    W32_EndMessageQueueWrite(&network_message_queue);
                }
            }
            
            if (!io_successed)
            {
                Message *message = W32_BeginMessageQueueWrite(&network_message_queue);
                message->type = NetworkMessage_FailedToJoin;
                W32_EndMessageQueueWrite(&network_message_queue);
            }
            
            if (LobbySocket != InvalidSocket)
            {
                os->CloseSocket(LobbySocket);
            }
        } break;
        case PlayerMessage_Username:
        {
            b32 io_successed = true;
            Assert(network_io_context.host_socket != InvalidSocket);
            NetworkSendString(network_io_context.host_socket, message->username, io_successed);
            if (!io_successed)
            {
                Message *message = W32_BeginMessageQueueWrite(&network_message_queue);
                message->type = NetworkMessage_HostDown;
                W32_EndMessageQueueWrite(&network_message_queue);
            }
        } break;
        case PlayerMessage_PlayCard:
        {
            b32 io_successed = true;
            PlayerMoveKind move_kind = PlayerMove_PlayCard;
            NetworkSendValue(network_io_context.host_socket, move_kind, io_successed);
            NetworkSendArray(network_io_context.host_socket,
                             message->player_move.actual_cards, 
                             message->player_move.played_cards_count, 
                             Compact_Card_Type, io_successed);
            NetworkSendValue(network_io_context.host_socket, message->player_move.declared_number, io_successed);
            if (!io_successed)
            {
                Message *message = W32_BeginMessageQueueWrite(&network_message_queue);
                message->type = NetworkMessage_HostDown;
                W32_EndMessageQueueWrite(&network_message_queue);}
        } break;
        case PlayerMessage_QuestionCredibility:
        {
            b32 io_successed = true;
            PlayerMoveKind move_kind = PlayerMove_QuestionCredibility;
            NetworkSendValue(network_io_context.host_socket, move_kind, io_successed);
            if (!io_successed)
            {
                Message *message = W32_BeginMessageQueueWrite(&network_message_queue);
                message->type = NetworkMessage_HostDown;
                W32_EndMessageQueueWrite(&network_message_queue);
            }
            
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
    Thread_Ctx WorkerThreadContext = MakeTCTX();
    SetTCTX(&WorkerThreadContext);
    
    network_io_context.message_type_wsa_buf.buf = (char *)&network_io_context.message_type;
    network_io_context.message_type_wsa_buf.len = sizeof(network_io_context.message_type);
    
    network_io_context.arena = M_ArenaAlloc(Megabytes(1));
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
                    Message *message = W32_BeginMessageQueueWrite(&network_message_queue);
                    message->type = network_io_context.message_type;
                    // NOTE(fakhri): receive the actual message from the host
                    switch(network_io_context.message_type)
                    {
                        case HostMessage_HostShuttingDown:
                        {
                            b32 io_successed = true;
                            NetworkReceiveValue(network_io_context.host_socket, message->host_closing_reason, io_successed);
                            if (!io_successed)
                            {
                                message->host_closing_reason = ClosingReason_Unkown;
                            }
                            os->CloseSocket(network_io_context.host_socket);
                            network_io_context.host_socket = InvalidSocket;
                        } break;
                        case HostMessage_InvalidUsername:
                        {
                            // NOTE(fakhri): nothing to do
                        } break;
                        case HostMessage_PlayCard:
                        {
                            b32 io_successed = true;
                            message->player_move.actual_cards = (Compact_Card_Type *)message->buffer;
                            NetworkReceiveArrayNoAlloc(network_io_context.host_socket, 
                                                       message->player_move.actual_cards, 
                                                       message->player_move.played_cards_count, 
                                                       Compact_Card_Type, io_successed);
                            NetworkReceiveValue(network_io_context.host_socket, message->player_move.declared_number, io_successed);
                            
                            if (!io_successed)
                            {
                                message->type = NetworkMessage_HostDown;
                            }
                        } break;
                        case HostMessage_QuestionCredibility:
                        {
                            // NOTE(fakhri): just type is enough
                        } break;
                        case HostMessage_ConnectedPlayersList:
                        {
                            b32 io_successed = true;
                            Log("HostMessage_ConnectedPlayersList");
                            NetworkReceiveValue(network_io_context.host_socket, message->players_count, io_successed);
                            
                            u32 offset = 0;
                            for (u32 player_index = 0;
                                 player_index < message->players_count;
                                 ++player_index)
                            {
                                String8 *player_username = message->players_usernames + player_index;
                                NetworkReceiveValue(network_io_context.host_socket, player_username->len, io_successed);
                                Assert(offset + player_username->len < sizeof(message->buffer));
                                Assert(player_username->len < USERNAME_BUFFER_SIZE);
                                os->ReceiveBuffer(network_io_context.host_socket, message->buffer + offset, (i32)player_username->len);
                                player_username->str = message->buffer + offset;
                                offset += (i32)player_username->len;
                            }
                            if (!io_successed)
                            {
                                message->type = NetworkMessage_HostDown;
                            }
                        } break;
                        case HostMessage_NewPlayerJoined:
                        {
                            Log("HostMessage_NewPlayerJoined");
                            os->ReceiveBuffer(network_io_context.host_socket, &message->new_player_id, sizeof(message->new_player_id));
                            os->ReceiveBuffer(network_io_context.host_socket, &message->new_username.len, sizeof(message->new_username.len));
                            Assert(message->new_username.len < USERNAME_BUFFER_SIZE);
                            Assert(message->new_username.len < sizeof(message->buffer));
                            os->ReceiveBuffer(network_io_context.host_socket, message->buffer, (i32)message->new_username.len);
                            message->new_username.str = message->buffer;
                            
                        } break;
                        case HostMessage_ShuffledDeck:
                        {
                            Log("HostMessage_ShuffledDeck");
                            message->compact_deck = (Compact_Card_Type *)message->buffer;
                            u32 compact_deck_size = DECK_CARDS_COUNT * sizeof(Compact_Card_Type);
                            Assert(compact_deck_size < sizeof(message->buffer));
                            if (!os->ReceiveBuffer(network_io_context.host_socket, message->compact_deck, compact_deck_size))
                            {
                                message->type = NetworkMessage_HostDown;
                            }
                        } break;
                        case HostMessage_ChangePlayerTurn:
                        {
                            b32 io_successed = true;
                            Log("HostMessage_ChangePlayerTurn");
                            NetworkReceiveValue(network_io_context.host_socket, message->next_player_id, io_successed);
                            if (!io_successed)
                            {
                                message->type = NetworkMessage_HostDown;
                            }
                        } break;
                        default :
                        {
                            NotImplemented;
                        }
                    }
                    // NOTE(fakhri): make the changes visible to consumer threads
                    W32_EndMessageQueueWrite(&network_message_queue);
                    if (network_io_context.host_socket != InvalidSocket)
                    {
                        ReceiveNextMessageType();
                    }
                } break;
            }
        }
    }
}
