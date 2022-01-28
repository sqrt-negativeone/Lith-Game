
internal char *
W32_FrameCStringFromString(String8 string)
{
    char *buffer = 0;
    buffer = (char *)M_ArenaPushZero(&os->frame_arena, string.size+1);
    MemoryCopy(buffer, string.str, string.size);
    return buffer;
}

internal void
W32_SaveToFile(String8 path, void *data, u64 data_len)
{
    HANDLE file = {0};
    {
        DWORD desired_access = GENERIC_READ | GENERIC_WRITE;
        DWORD share_mode = 0;
        SECURITY_ATTRIBUTES security_attributes = {
            (DWORD)sizeof(SECURITY_ATTRIBUTES),
            0,
            0,
        };
        DWORD creation_disposition = CREATE_ALWAYS;
        DWORD flags_and_attributes = 0;
        HANDLE template_file = 0;
        
        if((file = CreateFile(W32_FrameCStringFromString(path),
                              desired_access,
                              share_mode,
                              &security_attributes,
                              creation_disposition,
                              flags_and_attributes,
                              template_file)) != INVALID_HANDLE_VALUE)
        {
            
            void *data_to_write = data;
            DWORD data_to_write_size = (DWORD)data_len;
            DWORD bytes_written = 0;
            
            WriteFile(file, data_to_write, data_to_write_size, &bytes_written, 0);
            
            CloseHandle(file);
        }
        else
        {
            W32_OutputError("File I/O Error", "Could not save to \"%s\"", path);
        }
    }
}

internal void
W32_AppendToFile(String8 path, void *data, u64 data_len)
{
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
        
        if((file = CreateFileA(W32_FrameCStringFromString(path),
                               desired_access,
                               share_mode,
                               &security_attributes,
                               creation_disposition,
                               flags_and_attributes,
                               template_file)) != INVALID_HANDLE_VALUE)
        {
            
            void *data_to_write = data;
            DWORD data_to_write_size = (DWORD)data_len;
            DWORD bytes_written = 0;
            
            SetFilePointer(file, 0, 0, FILE_END);
            WriteFile(file, data_to_write, data_to_write_size, &bytes_written, 0);
            
            CloseHandle(file);
        }
        else
        {
            W32_OutputError("File I/O Error", "Could not save to \"%s\"", path);
        }
    }
}

internal void
W32_LoadEntireFile(M_Arena *arena, String8 path, void **data, u64 *data_len)
{
    *data = 0;
    *data_len = 0;
    
    HANDLE file = {0};
    
    {
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
        
        if((file = CreateFile(W32_FrameCStringFromString(path), desired_access, share_mode, &security_attributes, creation_disposition, flags_and_attributes, template_file)) != INVALID_HANDLE_VALUE)
        {
            
            DWORD read_bytes = GetFileSize(file, 0);
            if(read_bytes)
            {
                void *read_data = M_ArenaPush(arena, read_bytes+1);
                DWORD bytes_read = 0;
                OVERLAPPED overlapped = {0};
                
                ReadFile(file, read_data, read_bytes, &bytes_read, &overlapped);
                
                ((u8 *)read_data)[read_bytes] = 0;
                
                *data = read_data;
                *data_len = (u64)bytes_read;
            }
            CloseHandle(file);
        }
    }
}

internal char *
W32_LoadEntireFileAndNullTerminate(M_Arena *arena, String8 path)
{
    char *result = 0;
    
    HANDLE file = {0};
    {
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
        
        if((file = CreateFile(W32_FrameCStringFromString(path), desired_access, share_mode, &security_attributes, creation_disposition, flags_and_attributes, template_file)) != INVALID_HANDLE_VALUE)
        {
            
            DWORD read_bytes = GetFileSize(file, 0);
            if(read_bytes)
            {
                result = (char *)M_ArenaPush(arena, read_bytes+1);
                DWORD bytes_read = 0;
                OVERLAPPED overlapped = {0};
                
                ReadFile(file, result, read_bytes, &bytes_read, &overlapped);
                
                result[read_bytes] = 0;
            }
            CloseHandle(file);
        }
        else
        {
            W32_OutputError("File I/O Error", "Could not read from \"%s\"", path);
        }
    }
    
    return result;
}

internal void
W32_FreeFileMemory(void *data)
{
    W32_HeapFree(data);
}

internal void
W32_DeleteFile(String8 path)
{
    DeleteFileA(W32_FrameCStringFromString(path));
}

internal b32
W32_MakeDirectory(String8 path)
{
    b32 result = 1;
    if(!CreateDirectoryA(W32_FrameCStringFromString(path), 0))
    {
        result = 0;
    }
    return result;
}

internal b32
W32_DoesFileExist(String8 path)
{
    b32 found = GetFileAttributesA(W32_FrameCStringFromString(path)) != INVALID_FILE_ATTRIBUTES;
    return found;
}

internal b32
W32_DoesDirectoryExist(String8 path)
{
    DWORD file_attributes = GetFileAttributesA(W32_FrameCStringFromString(path));
    b32 found = (file_attributes != INVALID_FILE_ATTRIBUTES &&
                 !!(file_attributes & FILE_ATTRIBUTE_DIRECTORY));
    return found;
}

internal b32
W32_CopyFile(String8 dest, String8 source)
{
    b32 success = 0;
    success = CopyFile(W32_FrameCStringFromString(source), W32_FrameCStringFromString(dest), 0);
    return success;
}

internal OS_DirectoryList
W32_DirectoryListLoad(M_Arena *arena, String8 path, i32 flags)
{
    OS_DirectoryList list = {0};
    
    return list;
}


internal b32
W32_BeginWatchDirectory(Directory_Watcher *watcher, M_Arena *arena, s8 dir_path)
{
    b32 result = 0;
    
    watcher->dir_handle = CreateFileA(dir_path.str,
                                      GENERIC_READ,
                                      FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE,
                                      0, 
                                      OPEN_EXISTING,
                                      FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED ,
                                      0);
    
    
    if (watcher->buffer && watcher->dir_handle != INVALID_HANDLE_VALUE)
    {
        watcher->overlapped = (OVERLAPPED *)M_ArenaPush(arena, sizeof(OVERLAPPED));
        
        if (ReadDirectoryChangesW(watcher->dir_handle,
                                  watcher->buffer,
                                  (DWORD)WATCHER_BUFFER_SIZE,
                                  FALSE,
                                  FILE_NOTIFY_CHANGE_LAST_WRITE,
                                  0,
                                  watcher->overlapped,
                                  0))
        {
            // NOTE(fakhri): good
            result = 1;
        }
        else
        {
            LogError("ReadDirectoryChangesW failed\n");
        }
    }
    else
    {
        LogError("couldn't open directory %s\n", dir_path.str);
    }
    
    return result;
}


internal s8
W32_CheckDirectoryChanges(M_Arena *arena, Directory_Watcher *dir_watch)
{
    s8 result = {};
    if (HasOverlappedIoCompleted(dir_watch->overlapped))
    {
        if (GetOverlappedResult(dir_watch->dir_handle, 
                                dir_watch->overlapped,
                                &(dir_watch->bytes_filled),
                                0)
            && (dir_watch->bytes_filled != 0))
        {
            // NOTE(fakhri): some file changed in the directory !
            FILE_NOTIFY_INFORMATION *information = (FILE_NOTIFY_INFORMATION *)dir_watch->buffer;
            u32 changed_cnt = 0;
            for (;;)
            {
                ++changed_cnt;
                {
                    u32 len = information->FileNameLength / sizeof(information->FileName);
                    result = StringReserve(arena, len);
                    for (u32 i = 0; i < result.size; ++i)
                    {
                        result.str[i] = (char)information->FileName[i];
                    }
                    if (information->NextEntryOffset == 0)
                    {
                        break;
                    }
                }
                information += information->NextEntryOffset;
            }
            // NOTE(fakhri): make sure we get only one notification at a time ?
            HardAssert(changed_cnt == 1);
            if (ReadDirectoryChangesW(dir_watch->dir_handle,
                                      dir_watch->buffer,
                                      (DWORD)WATCHER_BUFFER_SIZE,
                                      FALSE,
                                      FILE_NOTIFY_CHANGE_LAST_WRITE,
                                      0,
                                      dir_watch->overlapped,
                                      0))
            {
                // NOTE(fakhri): watch again
            }
        }
        else
        {
        }
    }
    return result;
}
