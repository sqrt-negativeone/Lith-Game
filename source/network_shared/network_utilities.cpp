
internal SOCKET
ConnectToServer(char *server_address, char *port)
{
    SOCKET result = INVALID_SOCKET;
    struct addrinfo hints, *addrinfo_result, *p;
    hints = {};
    
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    
    getaddrinfo(server_address, port, &hints, &addrinfo_result);
    for (p = addrinfo_result; p; p = p->ai_next)
    {
        
        result = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (result == INVALID_SOCKET) 
        {
            LogError("socket failed with error: %ld", WSAGetLastError());
            continue;
        }
        
        // NOTE(fakhri): connect to the socket
        if (connect(result, p->ai_addr, (int)p->ai_addrlen) == SOCKET_ERROR) 
        {
            LogError("socket failed with error: %ld", WSAGetLastError());
            closesocket(result);
            continue;
        }
        break;
    }
    freeaddrinfo(addrinfo_result);
    if (!p) result = INVALID_SOCKET;
    return result;
}

internal SOCKET
OpenListenSocket(char *port)
{
    SOCKET listenfd = INVALID_SOCKET;
    const char optionval = 1;
    struct addrinfo hints = {}, *addrinfo_result, *p;
    
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    
    if (getaddrinfo(0, port, &hints, &addrinfo_result) == 0)
    {
        
        for (p = addrinfo_result; p; p = p->ai_next)
        {
            listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (listenfd == INVALID_SOCKET) continue;
            setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optionval, sizeof(i32));
            if (bind(listenfd, p->ai_addr, (i32)p->ai_addrlen) != SOCKET_ERROR) break;
            closesocket(listenfd);
        }
        
        freeaddrinfo(addrinfo_result);
        if (!p) return INVALID_SOCKET;
        
        if (listen(listenfd, LISTENQ) == SOCKET_ERROR )
        {
            closesocket(listenfd);
            listenfd = INVALID_SOCKET;
        }
    }
    return listenfd;
}

internal b32
SendBuffer(SOCKET s, void *data, i32 len)
{
    char *buffer = (char *)data;
    b32 result = true;
    i32 bytes_to_send = len;
    i32 total_bytes_sent = 0;
    
    while(bytes_to_send)
    {
        i32 bytes_sent = send(s, buffer + total_bytes_sent, bytes_to_send, 0);
        if (bytes_sent == SOCKET_ERROR)
        {
            LogError("send call failed");
            result = false;
            break;
        }
        total_bytes_sent += bytes_sent;
        bytes_to_send -= bytes_sent;
    }
    
    return result;
}

internal b32
ReceiveBuffer(SOCKET s, void *data, i32 len)
{
    char *buffer = (char *)data;
    b32 result = true;
    i32 bytes_to_receive = len;
    i32 total_bytes_received = 0;
    
    while(bytes_to_receive)
    {
        i32 bytes_received = recv(s, buffer + total_bytes_received, bytes_to_receive, 0);
        if (bytes_received == SOCKET_ERROR)
        {
            LogError("send call failed");
            result = false;
            break;
        }
        total_bytes_received += bytes_received;
        bytes_to_receive -= bytes_received;
    }
    
    return result;
}
