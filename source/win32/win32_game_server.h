/* date = March 6th 2022 0:34 pm */

#ifndef WIN32_GAME_SERVER_H
#define WIN32_GAME_SERVER_H



typedef HANDLE Semaphore_Handle;
typedef HANDLE Mutex_Handle;

struct Card_Hand
{
    Card_Type cards[DECK_CARDS_COUNT];
    u32 cards_count;
};

struct Compact_Card_Hand
{
    Compact_Card_Type cards[DECK_CARDS_COUNT];
    u32 cards_count;
};

struct Connected_Player
{
    Socket_Handle socket;
    char username[20];
    Compact_Card_Hand hand;
    
#if 0
    Connected_Player *next_free;
#endif
};

enum Game_Step
{
    Game_Step_None,
    Game_Step_Shuffle_Deck,
    Game_Step_Send_Deck_To_Players,
    Game_Step_Change_Player_Turn,
    Game_Step_Wait_For_Player_Move,
};

struct Players_Storage
{
    Mutex_Handle players_mutex;
    Connected_Player players[MAX_PLAYER_COUNT];
    u32 count;
};

struct Host_Context
{
    Players_Storage players_storage;
    Players_Storage temporary_storage;
    
    Semaphore_Handle usernames_done_sempahore;
    volatile b32 host_running;
};

global Host_Context host_context;


internal void
BroadcastPlayerMove(Player_Move player_move)
{
    for (u32 player_index = 0;
         player_index < host_context.players_storage.count;
         ++player_index)
    {
        Connected_Player *player = host_context.players_storage.players + player_index;
        // NOTE(fakhri): send the message to player
        u32 type = MessageType_Player_Move;
        SendBuffer(player->socket, &type, sizeof(type));
        
        SendBuffer(player->socket, &player_move.type, sizeof(player_move.type));
        if(player_move.type == Player_Move_Play_Card)
        {
            SendBuffer(player->socket, &player_move.actual_card, sizeof(player_move.actual_card));
            SendBuffer(player->socket, &player_move.claimed_card, sizeof(player_move.claimed_card));
        }
    }
}

internal void
BroadcastShuffledDeckMessage(Compact_Card_Type *compact_deck, u32 deck_size)
{
    for (u32 player_index = 0;
         player_index < host_context.players_storage.count;
         ++player_index)
    {
        Connected_Player *player = host_context.players_storage.players + player_index;
        // NOTE(fakhri): send the message to player
        u32 type = MessageType_From_Host_Shuffled_Deck;
        SendBuffer(player->socket, &type, sizeof(type));
        SendBuffer(player->socket, compact_deck, deck_size);
    }
}

internal void
BroadcastNewPlayerJoinedMessage(Connected_Player *new_player, u32 player_id)
{
    for (u32 player_index = 0;
         player_index < host_context.players_storage.count;
         ++player_index)
    {
        Connected_Player *player = host_context.players_storage.players + player_index;
        // NOTE(fakhri): send the message to player
        u32 type = MessageType_From_Host_New_Player_Joined;
        SendBuffer(player->socket, &type, sizeof(type));
        SendBuffer(player->socket, &player_id, sizeof(player_id));
        SendBuffer(player->socket, new_player->username, sizeof(new_player->username));
    }
}

internal void
BroadcastChangeTurnMessage(u32 player_id)
{
    for (u32 player_index = 0;
         player_index < host_context.players_storage.count;
         ++player_index)
    {
        Connected_Player *player = host_context.players_storage.players + player_index;
        // NOTE(fakhri): send the message to player
        u32 type = MessageType_From_Host_Change_Player_Turn;
        SendBuffer(player->socket, &type, sizeof(type));
        SendBuffer(player->socket, &player_id, sizeof(player_id));
    }
}

internal void
BroadcastPlayerWon(u32 winner_id)
{
    for (u32 player_index = 0;
         player_index < host_context.players_storage.count;
         ++player_index)
    {
        Connected_Player *player = host_context.players_storage.players + player_index;
        // NOTE(fakhri): send the message to player
        u32 type = MessageType_From_Host_Player_Won;
        SendBuffer(player->socket, &type, sizeof(type));
        SendBuffer(player->socket, &winner_id, sizeof(winner_id));
    }
}

internal b32
ReceivePlayerMove(Socket_Handle s, Player_Move *player_move)
{
    b32 result = false;
    u32 type;
    ReceiveBuffer(s, &type, sizeof(type));
    
    if(type == MessageType_Player_Move)
    {
        result = true;
        ReceiveBuffer(s, &player_move->type, sizeof(player_move->type));
        if(player_move->type == Player_Move_Play_Card)
        {
            ReceiveBuffer(s, &player_move->actual_card, sizeof(player_move->actual_card));
            ReceiveBuffer(s, &player_move->claimed_card, sizeof(player_move->claimed_card));
        }
    }
    return result;
}

internal void
SendConnectedPlayersList(Socket_Handle s)
{
    u32 type = MessageType_From_Host_Connected_Players_List;
    SendBuffer(s, &type, sizeof(type));
    
    Players_Storage *players_storage = &host_context.players_storage;
    SendBuffer(s, &players_storage->count, sizeof(players_storage->count));
    for (u32 player_index = 0;
         player_index < players_storage->count;
         ++player_index)
    {
        Connected_Player *player = players_storage->players + player_index;
        SendBuffer(s, player->username, sizeof(player->username));
    }
}
#endif //WIN32_GAME_SERVER_H
