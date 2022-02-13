
internal
NetworkMessage CreateConnectToServerMessage(M_Arena *arena, s8 server_address)
{
    NetworkMessage result = {};
    result.type = NetworkMessageType_From_Player_CONNECT_TO_SERVER;
    result.server_address = StringCopy(arena, server_address);
    return result;
}

internal
NetworkMessage CreateNewGameSessionMessage()
{
    NetworkMessage result = {};
    result.type = NetworkMessageType_From_Player_NEW_GAME_SESSION;
    // TODO(fakhri): get the server adress;
    return result;
}

internal
NetworkMessage CreateUsernameRequest(s8 username)
{
    NetworkMessage result = {};
    result.type     = NetworkMessageType_From_Player_USERNAME;
    result.player_username = username;
    return result;
}
