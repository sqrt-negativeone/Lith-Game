/* date = February 3rd 2022 8:33 pm */

#ifndef NETWORK_MESSAGE_H
#define NETWORK_MESSAGE_H

enum NetworkMessageType
{
    NetworkMessageType_From_Player_Begin,
    // NOTE(fakhri): server messages here
    NetworkMessageType_From_Player_CONNECT_TO_SERVER,
    NetworkMessageType_From_Player_NEW_GAME_SESSION,
    NetworkMessageType_From_Player_USERNAME,
    NetworkMessageType_From_Player_End,
    
    NetworkMessageType_From_Server_Begin,
    // NOTE(fakhri): player messages here
    NetworkMessageType_From_Server_PLAYER_TURN,
    NetworkMessageType_From_Server_PLAYER_JOINED_GAME,
    NetworkMessageType_From_Server_INVALIDE_USERNAME,
    NetworkMessageType_From_Server_End,
};

#define IsNetworkMessageFromServer(type) ((type) > NetworkMessageType_From_Server_Begin && (type) < NetworkMessageType_From_Server_End )

#define IsNetworkMessageFromPlayer(type) ((type) > NetworkMessageType_From_Player_Begin && (type) < NetworkMessageType_From_Player_End )

struct NetworkMessage
{
    NetworkMessageType type;
    b32 is_available;
    s8 server_address;
    
    S8 player_username;
    u32 player_index;
};

#endif //NETWORK_MESSAGE_H
