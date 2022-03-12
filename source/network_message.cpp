// TODO(fakhri): Message is getting huge, we probably want to switch to 
// passing a pointer to the message we want to fill instead of return the
// whole struct

#if 1
internal Message
CreateConnectToServerMessage(s8 server_address)
{
    Message result = {};
    result.type = MessageType_From_Player_Connect_To_Host;
    result.server_address = server_address;
    return result;
}

internal Message
CreateUsernameMessage(s8 username)
{
    Message result = {};
    result.type            = MessageType_From_Player_USERNAME;
    Assert(username.size < ArrayCount(result.players[0].username));
    MemoryCopy(result.players[0].username, username.str, username.size);
    return result;
}

internal Message
CreateFetchAvailableHostsMessage(Hosts_Storage *hosts_storage)
{
    Message result = {};
    result.type = MessageType_From_Player_Fetch_Hosts;
    result.hosts_storage = hosts_storage;
    return result;
}

#endif
