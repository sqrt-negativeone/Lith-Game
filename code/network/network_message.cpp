// TODO(fakhri): Message is getting huge, we probably want to switch to 
// passing a pointer to the message we want to fill instead of return the
// whole struct

internal void
PushCreateConnectToServerMessage(String8 server_address)
{
    Message *message = os->BeginPlayerMessageQueueWrite();
    message->type = PlayerMessage_ConnectToHost;
    Assert(server_address.size < ArrayCount(message->buffer));
    MemoryCopy(message->buffer, server_address.str, server_address.size);
    message->server_address = Str8(message->buffer, server_address.size);
    os->EndPlayerMessageQueueWrite();
}

internal void
PushUsernameNetworkMessage(String8 username)
{
    Message *message = os->BeginPlayerMessageQueueWrite();
    Assert(username.size < ArrayCount(message->buffer));
    MemoryCopy(message->buffer, username.str, username.size);
    message->type     = PlayerMessage_Username;
    message->username = Str8(message->buffer, username.size);
    os->EndPlayerMessageQueueWrite();
}

#if 0
internal Message
CreateFetchAvailableHostsMessage(Hosts_Storage *hosts_storage)
{
    Message result = {};
    result.type = PlayerMessage_FetchHosts;
    result.hosts_storage = hosts_storage;
    return result;
}

#endif
