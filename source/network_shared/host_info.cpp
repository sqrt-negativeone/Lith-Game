
internal b32
ReceiveHostInfo(SOCKET sender_socket, Host_Info *host_info)
{
    b32 result = false;
    if (!ReceiveBuffer(sender_socket, host_info->ip, sizeof(host_info->ip)))
    {
        // NOTE(fakhri): we couldn't send
        return result;
    }
    
    if (!ReceiveBuffer(sender_socket, host_info->hostname, sizeof(host_info->hostname)))
    {
        // NOTE(fakhri): we couldn't send
        return result;
    }
    
    if (!ReceiveBuffer(sender_socket, &host_info->port, sizeof(host_info->port)))
    {
        // NOTE(fakhri): we couldn't send
        return result;
    }
    
    host_info->port = ntohs(host_info->port);
    
    result = true;
    return result;
}

internal b32
SendHostInfo(SOCKET client_socket, Host_Info *host_info)
{
    b32 result = false;
    if (!SendBuffer(client_socket, host_info->ip, sizeof(host_info->ip)))
    {
        // NOTE(fakhri): we couldn't send
        return result;
    }
    
    if (!SendBuffer(client_socket, host_info->hostname, sizeof(host_info->hostname)))
    {
        // NOTE(fakhri): we couldn't send
        return result;
    }
    
    if (!SendBuffer(client_socket, &host_info->port, sizeof(host_info->port)))
    {
        // NOTE(fakhri): we couldn't send
        return result;
    }
    result = true;
    return result;
}
