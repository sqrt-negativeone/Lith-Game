/* date = April 8th 2022 10:38 pm */

#ifndef GAME_HOST_H
#define GAME_HOST_H

#define LOBBY_HOST_PORT "1234"
#define LOBBY_PLAYER_PORT "1235"

#define HOST_PORT "23451"

struct Compact_Card_Hand
{
    Compact_Card_Type cards[DECK_CARDS_COUNT];
    u32 cards_count;
};

#define USERNAME_BUFFER_SIZE 20
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
};

internal void GameHostWork(void *data);
internal void
BroadcastShuffledDeckMessage(Host_Context *host_context)
{
    CardResidency *deck_residency = host_context->residencies + CardResidencyKind_Deck;
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
         player_index < host_context->players_storage.count;
         ++player_index)
    {
        Connected_Player *player = host_context->players_storage.players + player_index;
        // NOTE(fakhri): send the message to player
        NetworkSendValue(player->socket, type);
        os->SendBuffer(player->socket, compact_deck, sizeof(compact_deck));
    }
}

internal void
BroadcastNewPlayerJoinedMessage(Host_Context *host_context, Connected_Player *new_player, u32 player_id)
{
    for (u32 player_index = 0;
         player_index < host_context->players_storage.count;
         ++player_index)
    {
        Connected_Player *player = host_context->players_storage.players + player_index;
        // NOTE(fakhri): send the message to player
        MessageType type = HostMessage_NewPlayerJoined;
        NetworkSendValue(player->socket, type);
        NetworkSendValue(player->socket, player_id);
        os->SendString(player->socket, new_player->username);
    }
}

internal void
BroadcastChangeTurnMessage(Host_Context *host_context, PlayerID player_id)
{
    for (u32 player_index = 0;
         player_index < host_context->players_storage.count;
         ++player_index)
    {
        Connected_Player *player = host_context->players_storage.players + player_index;
        // NOTE(fakhri): send the message to player
        MessageType type = HostMessage_ChangePlayerTurn;
        NetworkSendValue(player->socket, type);
        NetworkSendValue(player->socket, player_id);
    }
}

internal void
BroadcastPlayerWon(Host_Context *host_context, u32 winner_id)
{
    for (u32 player_index = 0;
         player_index < host_context->players_storage.count;
         ++player_index)
    {
        Connected_Player *player = host_context->players_storage.players + player_index;
        // NOTE(fakhri): send the message to player
        MessageType type = HostMessage_PlayerWon;
        NetworkSendValue(player->socket, type);
        NetworkSendValue(player->socket, winner_id);
    }
}

internal void
SendConnectedPlayersList(Host_Context *host_context, Socket_Handle s)
{
    MessageType type = HostMessage_ConnectedPlayersList;
    NetworkSendValue(s, type);
    Players_Storage *players_storage = &host_context->players_storage;
    NetworkSendValue(s, players_storage->count);
    for (u32 player_index = 0;
         player_index < players_storage->count;
         ++player_index)
    {
        Connected_Player *player = players_storage->players + player_index;
        os->SendString(s, player->username);
    }
}


#endif //GAME_HOST_H
