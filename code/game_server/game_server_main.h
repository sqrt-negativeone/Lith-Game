/* date = April 8th 2022 10:38 pm */

#ifndef GAME_SERVER_MAIN_H
#define GAME_SERVER_MAIN_H

struct Compact_Card_Hand
{
    Compact_Card_Type cards[DECK_CARDS_COUNT];
    u32 cards_count;
};

struct Connected_Player
{
    Socket_Handle socket;
    char buffer[USERNAME_BUFFER_SIZE];
    String8 username;
};

typedef u32 CardResidencyKind;
enum
{
    CardResidencyKind_Deck,
    CardResidencyKind_Table,
    CardResidencyKind_Burnt,
    CardResidencyKind_Player0,
    CardResidencyKind_Player1,
    CardResidencyKind_Player2,
    CardResidencyKind_Player3,
    CardResidencyKind_Count,
};

struct CardResidency
{
    Card_Type cards[DECK_CARDS_COUNT];
    u32 count;
};

enum Game_Step
{
    GameStep_None,
    GameStep_ShuffleDeck,
    GameStep_SendDeckToPlayers,
    GameStep_ChangePlayerTurn,
    GameStep_WaitForPlayerMove,
    GameStep_InvalidMove,
    GameStep_Finished,
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
    
    CardResidency residencies[CardResidencyKind_Count];
    u64 completed_username_work;
    u32 prev_played_card_count;
    Card_Number declared_number;
    volatile b32 host_running;
};

global Host_Context host_context;

internal void
BroadcastShuffledDeckMessage()
{
    CardResidency *deck_residency = host_context.residencies + CardResidencyKind_Deck;
    Compact_Card_Type compact_deck[DECK_CARDS_COUNT];
    for(u32 index = 0;
        index < deck_residency->count;
        ++index)
    {
        Card_Type card = deck_residency->cards[index];
        compact_deck[index] = MakeCompactCardType(card.category, card.number);
    }
    
    MessageType type = HostMessage_ShuffledDeck;
    for (u32 player_index = 0;
         player_index < host_context.players_storage.count;
         ++player_index)
    {
        Connected_Player *player = host_context.players_storage.players + player_index;
        // NOTE(fakhri): send the message to player
        NetworkSendValue(player->socket, type);
        SendBuffer(player->socket, compact_deck, sizeof(compact_deck));
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
        MessageType type = HostMessage_NewPlayerJoined;
        NetworkSendValue(player->socket, type);
        NetworkSendValue(player->socket, player_id);
        SendString(player->socket, new_player->username);
    }
}

internal void
BroadcastChangeTurnMessage(PlayerID player_id)
{
    for (u32 player_index = 0;
         player_index < host_context.players_storage.count;
         ++player_index)
    {
        Connected_Player *player = host_context.players_storage.players + player_index;
        // NOTE(fakhri): send the message to player
        MessageType type = HostMessage_ChangePlayerTurn;
        NetworkSendValue(player->socket, type);
        NetworkSendValue(player->socket, player_id);
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
        MessageType type = HostMessage_PlayerWon;
        NetworkSendValue(player->socket, type);
        NetworkSendValue(player->socket, winner_id);
    }
}

internal void
SendConnectedPlayersList(Socket_Handle s)
{
    MessageType type = HostMessage_ConnectedPlayersList;
    NetworkSendValue(s, type);
    Players_Storage *players_storage = &host_context.players_storage;
    NetworkSendValue(s, players_storage->count);
    for (u32 player_index = 0;
         player_index < players_storage->count;
         ++player_index)
    {
        Connected_Player *player = players_storage->players + player_index;
        SendString(s, player->username);
    }
}


internal void
W32_WaitForSemaphore(Semaphore_Handle semaphore)
{
    WaitForSingleObject(semaphore, INFINITE);
}

internal void
W32_ReleaseSemaphore(Semaphore_Handle semaphore)
{
    ReleaseSemaphore(semaphore, 1, 0);
}

internal void
W32_WaitForMutex(Mutex_Handle mutex)
{
    WaitForSingleObject(mutex, INFINITE);
}

internal Mutex_Handle
W32_CreateMutex()
{
    Mutex_Handle result = CreateMutexA(0, FALSE, 0);
    return result;
}

internal void
W32_ReleaseMutex(Mutex_Handle mutex)
{
    ReleaseMutex(mutex);
}

#endif //GAME_SERVER_MAIN_H
