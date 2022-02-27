
internal NetworkMessage
CreateConnectToServerMessage(M_Arena *arena, s8 server_address)
{
    NetworkMessage result = {};
    result.type = NetworkMessageType_From_Player_CONNECT_TO_SERVER;
    result.server_address = StringCopy(arena, server_address);
    return result;
}

internal NetworkMessage
CreateNewGameSessionMessage()
{
    NetworkMessage result = {};
    result.type = NetworkMessageType_From_Player_NEW_GAME_SESSION;
    // TODO(fakhri): get the server adress;
    return result;
}

internal NetworkMessage
CreateUsernameMessage(s8 username)
{
    NetworkMessage result = {};
    result.type     = NetworkMessageType_From_Player_USERNAME;
    result.player_username = username;
    return result;
}

internal NetworkMessage
CreateFetchAvailableHostsMessage(Hosts_Storage *hosts_storage)
{
    NetworkMessage result = {};
    result.type = NetworkMessageType_From_Player_FETCH_HOSTS;
    result.hosts_storage = hosts_storage;
    return result;
}

internal NetworkMessage 
CreateCouldNotConnectToServerMessage()
{
    NetworkMessage result = {};
    result.type = NetworkMessageType_From_Server_COULD_NOT_CONNECT_TO_SERVER;
    return result;
}
