
//~ NOTE(fakhri): exposed  API

////////////////////////////////
//- NOTE(fakhri): Memory internals

internal void *
W32_HeapAlloc(u32 size)
{
    return HeapAlloc(GetProcessHeap(), 0, size);
}

internal void
W32_HeapFree(void *data)
{
    HeapFree(GetProcessHeap(), 0, data);
}

internal void
W32_Release(void *memory)
{
    VirtualFree(memory, 0, MEM_RELEASE);
}

internal void *
W32_Reserve(u64 size)
{
    void *memory = VirtualAlloc(0, size, MEM_RESERVE, PAGE_NOACCESS);
    return memory;
}

internal void
W32_Commit(void *memory, u64 size)
{
    VirtualAlloc(memory, size, MEM_COMMIT, PAGE_READWRITE);
}

internal void
W32_Decommit(void *memory, u64 size)
{
    VirtualFree(memory, size, MEM_DECOMMIT);
}

internal U64
W32_PageSize(void)
{
    if(w32_got_system_info == 0)
    {
        w32_got_system_info = 1;
        GetSystemInfo(&w32_system_info);
    }
    return w32_system_info.dwPageSize;
}

////////////////////////////////
//- NOTE(fakhri): output error internal

internal void
W32_OutputError(char *title, char *format, ...)
{
    va_list args;
    va_start(args, format);
    u32 required_characters = vsnprintf(0, 0, format, args)+1;
    va_end(args);
    
    char text[4096] = {0};
    if(required_characters > 4096)
    {
        required_characters = 4096;
    }
    
    va_start(args, format);
    vsnprintf(text, required_characters, format, args);
    va_end(args);
    
    text[required_characters-1] = 0;
    MessageBoxA(0, text, title, MB_OK);
}

////////////////////////////////
//- NOTE(fakhri): file internals

internal String8
W32_LoadEntireFile(M_Arena *arena, String8 path)
{
    String8 result = {0};
    
    DWORD desired_access = GENERIC_READ | GENERIC_WRITE;
    DWORD share_mode = 0;
    SECURITY_ATTRIBUTES security_attributes = {
        (DWORD)sizeof(SECURITY_ATTRIBUTES),
        0,
        0,
    };
    DWORD creation_disposition = OPEN_EXISTING;
    DWORD flags_and_attributes = 0;
    HANDLE template_file = 0;
    
    M_Temp scratch = GetScratch(&arena, 1);
    String16 path16 = Str16From8(scratch.arena, path);
    HANDLE file = CreateFileW((WCHAR*)path16.str,
                              desired_access,
                              share_mode,
                              &security_attributes,
                              creation_disposition,
                              flags_and_attributes,
                              template_file);
    
    if(file != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER size_int;
        if(GetFileSizeEx(file, &size_int) && size_int.QuadPart > 0)
        {
            U64 size = size_int.QuadPart;
            void *data = PushArray(arena, U8, size + 1);
            W32_ReadWholeBlock(file, data, size);
            ((U8*)data)[size] = 0;
            result = Str8((U8 *)data, size);
        }
        else
        {
            result.str = PushArrayZero(arena, U8, 1);
            result.size = 0;
        }
        CloseHandle(file);
    }
    
    ReleaseScratch(scratch);
    return(result);
}

internal B32
W32_SaveToFile(String8 path, String8List data)
{
    B32 result = 0;
    HANDLE file = {0};
    {
        DWORD desired_access = GENERIC_READ | GENERIC_WRITE;
        DWORD share_mode = 0;
        SECURITY_ATTRIBUTES security_attributes = { (DWORD)sizeof(SECURITY_ATTRIBUTES) };
        DWORD creation_disposition = CREATE_ALWAYS;
        DWORD flags_and_attributes = 0;
        HANDLE template_file = 0;
        M_Temp scratch = GetScratch(0, 0);
        String16 path16 = Str16From8(scratch.arena, path);
        if((file = CreateFileW((WCHAR*)path16.str,
                               desired_access,
                               share_mode,
                               &security_attributes,
                               creation_disposition,
                               flags_and_attributes,
                               template_file)) != INVALID_HANDLE_VALUE)
        {
            result = 1;
            W32_WriteWholeBlock(file, data);
            CloseHandle(file);
        }
        ReleaseScratch(scratch);
    }
    return result;
}

internal b32
W32_AppendToFile(String8 path, String8List data)
{
    b32 result = 0;
    HANDLE file = {0};
    {
        DWORD desired_access = FILE_APPEND_DATA;
        DWORD share_mode = 0;
        SECURITY_ATTRIBUTES security_attributes = {
            (DWORD)sizeof(SECURITY_ATTRIBUTES),
            0,
            0,
        };
        DWORD creation_disposition = OPEN_ALWAYS;
        DWORD flags_and_attributes = 0;
        HANDLE template_file = 0;
        M_Temp scratch = GetScratch(0, 0);
        String16 path16 = Str16From8(scratch.arena, path);
        if((file = CreateFileW((WCHAR*)path16.str,
                               desired_access,
                               share_mode,
                               &security_attributes,
                               creation_disposition,
                               flags_and_attributes,
                               template_file)) != INVALID_HANDLE_VALUE)
        {
            result = 1;
            SetFilePointer(file, 0, 0, FILE_END);
            W32_WriteWholeBlock(file, data);
            
            CloseHandle(file);
        }
        else
        {
            W32_OutputError("File I/O Error", "Could not save to \"%s\"", path);
        }
        ReleaseScratch(scratch);
    }
    return result;
}

internal void
W32_DeleteFile(String8 path)
{
    M_Temp scratch = GetScratch(0, 0);
    String16 path16 = Str16From8(scratch.arena, path);
    DeleteFileW((WCHAR*)path16.str);
    ReleaseScratch(scratch);
}

internal B32
W32_MakeDirectory(String8 path)
{
    M_Temp scratch = GetScratch(0, 0);
    String16 path16 = Str16From8(scratch.arena, path);
    B32 result = 1;
    if(!CreateDirectoryW((WCHAR *)path16.str, 0))
    {
        result = 0;
    }
    ReleaseScratch(scratch);
    return result;
}


////////////////////////////////
//- NOTE(fakhri): Networking internals

internal b32
W32_IsHostMessageQueueEmpty()
{
    b32 result = W32_IsMessageQueueEmpty(&network_message_queue);
    return result;
}

internal Message *
W32_BeginHostMessageQueueRead()
{
    Message *result = W32_BeginMessageQueueRead(&network_message_queue);
    return result;
}

internal void
W32_EndHostMessageQueueRead()
{
    W32_EndMessageQueueRead(&network_message_queue);
}

internal Message *
W32_BeginPlayerMessageQueueWrite()
{
    Message *result = W32_BeginMessageQueueWrite(&player_message_queue);
    return result;
}

internal void
W32_EndPlayerMessageQueueWrite()
{
    W32_EndMessageQueueWrite(&player_message_queue);
    u64 completion_key = NetworkMessageSource_Player;
    PostQueuedCompletionStatus(network_thread_iocp_handle, 0, completion_key, 0);
}

internal Socket_Handle 
W32_AcceptSocket(Socket_Handle s, void *addr, int *addrlen)
{
    Socket_Handle result = accept(s, (sockaddr*)addr, addrlen);
    return result;
}

internal void 
W32_CloseSocket(Socket_Handle s)
{
    closesocket(s);
}

internal Socket_Handle 
W32_ConnectToServer(char *server_address, char *port)
{
    Socket_Handle result = INVALID_SOCKET;
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

#define LISTENQ 1024

internal Socket_Handle 
W32_OpenListenSocket(char *port)
{
    Socket_Handle listenfd = INVALID_SOCKET;
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
W32_SendBuffer(Socket_Handle s, void *data, i32 len)
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
            int last_error = WSAGetLastError();
            LogError("send call failed with error %d", last_error);
            result = false;
            break;
        }
        total_bytes_sent += bytes_sent;
        bytes_to_send -= bytes_sent;
    }
    
    return result;
}

internal b32
W32_ReceiveBuffer(Socket_Handle s, void *data, i32 len)
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
            int last_error = WSAGetLastError();
            LogError("receive call failed with error %d", last_error);
            result = false;
            break;
        }
        if (bytes_received == 0)
        {
            Log("socket closed from other end");
            result = false;
            break;
        }
        
        total_bytes_received += bytes_received;
        bytes_to_receive -= bytes_received;
    }
    
    return result;
}
internal b32
W32_SendString(Socket_Handle s, String8 data)
{
    b32 result = W32_SendBuffer(s, &data.len, sizeof(data.len)) && W32_SendBuffer(s, data.str, (i32)data.len);
    return result;
}

internal b32
W32_ReceiveString(Socket_Handle s, String8 *data)
{
    b32 result = W32_ReceiveBuffer(s, &data->len, sizeof(data->len)) && W32_ReceiveBuffer(s, data->str, (i32)data->len);
    return result;
}

// NOTE(fakhri): OS handles
internal void
W32_WaitForSemaphore(Semaphore_Handle semaphore)
{
    WaitForSingleObject(semaphore, INFINITE);
}

internal void
W32_ReleaseSemaphore(Semaphore_Handle semaphore)
{
    ReleaseSemaphore(semaphore, 1, 0);
}

internal void
W32_WaitForMutex(Mutex_Handle mutex)
{
    WaitForSingleObject(mutex, INFINITE);
}

internal Mutex_Handle
W32_CreateMutex()
{
    Mutex_Handle result = CreateMutexA(0, FALSE, 0);
    return result;
}

internal void
W32_ReleaseMutex(Mutex_Handle mutex)
{
    ReleaseMutex(mutex);
}

//- NOTE(fakhri): work queue functions
internal b32
W32_ProcessOneWorkQueueEntry()
{
    b32 result = false;
    while(!W32_IsWorkQueueEmpty())
    {
        OS_PopQueueResult pop_result = W32_PopQueueEntry();
        if (pop_result.valid)
        {
            result = true;
            OS_WorkQueue_Entry work_entry = pop_result.work_entry;
            work_entry.work(work_entry.data);
            break;
        }
    }
    return result;
}

internal b32
W32_PushWorkQueueEntrySP(OS_WorkThreadWork *work, void *data)
{
    // NOTE(fakhri): safe to call this without mutex if single producer
    b32 result = false;
    if (w32_work_queue.tail - w32_work_queue.head < ArrayCount(w32_work_queue.queue))
    {
        result = true;
        u64 entry_index = w32_work_queue.tail % ArrayCount(w32_work_queue.queue);
        w32_work_queue.queue[entry_index].work = work;
        w32_work_queue.queue[entry_index].data = data;
        MemoryBarrier();
        ++w32_work_queue.tail;
        // NOTE(fakhri): wake a sleeping thread if there are any
        ReleaseSemaphore(w32_work_queue.waiting_worker_threads_semaphore, 1, 0);
    }
    return result;
}

internal b32
W32_PushWorkQueueEntry(OS_WorkThreadWork *work, void *data)
{
    b32 result = false;
    
    WaitForSingleObject(w32_work_queue.producer_mutex, INFINITE);
    result = W32_PushWorkQueueEntrySP(work, data);
    ReleaseMutex(w32_work_queue.producer_mutex);
    
    return result;
}

internal OS_PopQueueResult
W32_PopQueueEntry()
{
    OS_PopQueueResult result = {};
    u64 old_head = w32_work_queue.head;
    u64 head_index = old_head % ArrayCount(w32_work_queue.queue);
    result.work_entry = w32_work_queue.queue[head_index];
    volatile u64 *destination = (volatile u64*)&w32_work_queue.head;
    if (InterlockedCompareExchange(destination, old_head + 1, old_head) == old_head)
    {
        result.valid = true;
    }
    return result;
}

internal b32
W32_IsWorkQueueEmpty()
{
    b32 result = (w32_work_queue.tail == w32_work_queue.head);
    return result;
}

internal void
W32_CopyStringToClipboard(String8 text)
{
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, text.size + 1);
    char *cstr = (char *)GlobalLock(hMem);
    memcpy(cstr, text.cstr, text.size);
    cstr[text.size] = 0;
    GlobalUnlock(hMem);
    OpenClipboard(w32_window_handle);
    EmptyClipboard();
    SetClipboardData(CF_TEXT, hMem);
    CloseClipboard();
}

internal String
W32_GetStringFromClipboard(M_Arena *arena)
{
    OpenClipboard(w32_window_handle);
    HANDLE clipboard_data = GetClipboardData(CF_TEXT);
    char *cstr = (char *)GlobalLock(clipboard_data);
    String Result = PushStr8Copy(arena, Str8C(cstr));
    GlobalUnlock(clipboard_data);
    CloseClipboard();
    return Result;
}
