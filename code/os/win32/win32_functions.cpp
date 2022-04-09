
//~ NOTE(fakhri): exposed  API

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
//~ rjf: File System

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
