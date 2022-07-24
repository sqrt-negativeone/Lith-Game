/* date = February 3rd 2022 8:33 pm */

#ifndef NETWORK_MESSAGE_H
#define NETWORK_MESSAGE_H

#define MAX_PLAYER_COUNT (4u)
#define CARDS_PER_PLAYER (13)
#define DECK_CARDS_COUNT (MAX_PLAYER_COUNT * CARDS_PER_PLAYER)
#define InvalidePlayerID MAX_PLAYER_COUNT

#define NetworkSendValue(s, v) do {os->SendBuffer(s, &(v), sizeof(v));} while(0)
#define NetworkReceiveValue(s, v) do {os->ReceiveBuffer(s, &(v), sizeof(v));} while(0)

#define NetworkSendArray(s, arr, cnt, T) do {NetworkSendValue(s, cnt); os->SendBuffer(s, arr, (cnt) * sizeof(T));} while(0)
#define NetworkReceiveArray(s, arr, cnt, T) do { NetworkReceiveValue(s, cnt); os->ReceiveBuffer(s, arr, (cnt) * sizeof(T));} while(0)

typedef u8 PlayerMoveKind;
enum
{
    PlayerMove_None,
    PlayerMove_PlayCard,
    PlayerMove_QuestionCredibility,
};

typedef u8 Compact_Card_Type;

typedef u8 MessageType;
enum
{
    // NOTE(fakhri): player messages here
    PlayerMessage_JoinGameSession,
    PlayerMessage_HostGameSession,
    PlayerMessage_Username,
    PlayerMessage_PlayCard,
    PlayerMessage_QuestionCredibility,
    
    // NOTE(fakhri): Host messages here
    HostMessage_InvalidUsername,
    HostMessage_HostFull,
    HostMessage_NewPlayerJoined,
    HostMessage_ConnectedPlayersList,
    HostMessage_ShuffledDeck,
    HostMessage_ChangePlayerTurn,
    HostMessage_PlayerWon,
    HostMessage_PlayCard,
    HostMessage_QuestionCredibility,
    
    // NOTE(fakhri): other network messages
    NetworkMessage_GameID,
    NetworkMessage_FailedToHost,
    NetworkMessage_JoinedGame,
    NetworkMessage_FailedToJoin,
};

struct Hosts_Storage;
struct Player;

struct PlayCardMove
{
    // NOTE(fakhri): game move struct
    Compact_Card_Type *actual_cards;
    u32 played_cards_count;
    Compact_Card_Type declared_number;
};

struct Message
{
    MessageType type;
    u8 buffer[128];
    union
    {
        struct
        {
            // NOTE(fakhri): connect to host message
            String8 server_address;
        };
        struct 
        {
            // NOTE(fakhri): player usernames message
            String8 username;
        };
        struct 
        {
            // NOTE(fakhri): connected players usernames message
            String8 players_usernames[MAX_PLAYER_COUNT];
            u32 players_count;
        };
        struct 
        {
            // NOTE(fakhri): new player joined message
            String8 new_username;
            PlayerID new_player_id;
        };
        struct 
        {
            // NOTE(fakhri): compact deck message
            Compact_Card_Type *compact_deck;
            u32 compact_cards_count;
        };
        PlayCardMove player_move;
        PlayerID next_player_id;
        String8 game_id;
    };
};


internal void PushCreateConnectToServerMessage(String8 server_address);
internal void PushUsernameNetworkMessage(String8 username);
internal void PushPlayedCardNetworkMessage(struct Game_State *game_state);
internal void PushQuestionCredibilityNetworkMessage();
internal void PushHostGameSessionNetworkMessage(String8 session_name);

#endif //NETWORK_MESSAGE_H
