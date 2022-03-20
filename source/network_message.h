/* date = February 3rd 2022 8:33 pm */

#ifndef NETWORK_MESSAGE_H
#define NETWORK_MESSAGE_H

enum Player_Move_Type
{
    Player_Move_None,
    Player_Move_Play_Card,
    Player_Move_Question_Credibility,
};

typedef u8 Compact_Card_Type;

enum MessageType
{
    MessageType_From_Player_Begin,
    // NOTE(fakhri): player messages here
    MessageType_From_Player_Connect_To_Host,
    MessageType_From_Player_USERNAME,
    MessageType_From_Player_Fetch_Hosts,
    MessageType_From_Player_Start_Host_Server,
    MessageType_From_Player_Stop_Host_Server,
    MessageType_From_Player_End,
    
    MessageType_From_Server_Begin,
    // NOTE(fakhri): Host messages here
    MessageType_From_Host_Invalid_Username,
    MessageType_From_Host_Host_Full,
    MessageType_From_Host_New_Player_Joined,
    MessageType_From_Host_Connected_Players_List,
    MessageType_From_Host_Shuffled_Deck,
    MessageType_From_Host_Change_Player_Turn,
    MessageType_From_Host_Player_Won,
    MessageType_From_Host_End,
    
    MessageType_Player_Move,
};

#define IsNetworkMessageFromServer(type) ((type) > NetworkMessageType_From_Server_Begin && (type) < NetworkMessageType_From_Server_End )

#define IsNetworkMessageFromPlayer(type) ((type) > NetworkMessageType_From_Player_Begin && (type) < NetworkMessageType_From_Player_End )

struct Hosts_Storage;
struct Player;

struct Player_Move
{
    Player_Move_Type type;
    Compact_Card_Type actual_card;
    Compact_Card_Type claimed_card;
};

#define MAX_PLAYER_COUNT (4u)
#define CARDS_PER_PLAYER (13)
#define DECK_CARDS_COUNT (MAX_PLAYER_COUNT * CARDS_PER_PLAYER)
#define InvalidePlayerID MAX_PLAYER_COUNT

struct MessagePlayer
{
    char username[20];
};

struct Message
{
    MessageType type;
    
    Hosts_Storage *hosts_storage;
    
    s8 server_address;
    
    Compact_Card_Type compact_deck[DECK_CARDS_COUNT];
    
    Player_Move player_move;
    MessagePlayer players[MAX_PLAYER_COUNT];
    u32 player_id;
    
    u32 players_count;
};

struct MessageResult
{
    Message message;
    b32 is_available;
};

#endif //NETWORK_MESSAGE_H
