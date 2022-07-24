
internal b32
W32_BeginWatchDirectory(M_Arena *arena, Directory_Watcher *watcher, String8 dir_path)
{
    b32 result = 0;
    
    M_Temp scratch = GetScratch(&arena, 1);
    String16 path16 = Str16From8(scratch.arena, dir_path);
    watcher->dir_handle = CreateFileW((WCHAR*)path16.str,
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
                                  ArrayCount(watcher->buffer),
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
            Assert(!"ReadDirectoryChangesW failed\n");
        }
    }
    else
    {
        LogError("couldn't open directory %s\n", dir_path.str);
    }
    
    ReleaseScratch(scratch);
    return result;
}


internal String8
W32_CheckDirectoryChanges(M_Arena *arena, Directory_Watcher *dir_watch)
{
    String8 result = {};
    if (HasOverlappedIoCompleted(dir_watch->overlapped))
    {
        if (GetOverlappedResult(dir_watch->dir_handle, 
                                dir_watch->overlapped,
                                &(dir_watch->bytes_filled),
                                0)
            && (dir_watch->bytes_filled != 0))
        {
            // NOTE(fakhri): a file has changed
            
            FILE_NOTIFY_INFORMATION *information = (FILE_NOTIFY_INFORMATION *)dir_watch->buffer;
            result = Str8From16(arena, Str16C((u16 *)information->FileName));
            // NOTE(fakhri): make sure we only got one file change notification
            Assert(information->NextEntryOffset == 0);
            
            MemoryZero(dir_watch->buffer, sizeof(dir_watch->buffer));
            b32 read_result = ReadDirectoryChangesW(dir_watch->dir_handle,
                                                    dir_watch->buffer,
                                                    (DWORD)ArrayCount(dir_watch->buffer),
                                                    FALSE,
                                                    FILE_NOTIFY_CHANGE_LAST_WRITE,
                                                    0,
                                                    dir_watch->overlapped,
                                                    0);
            Assert(read_result != 0);
        }
        else
        {
            InvalidCodePath;
        }
    }
    return result;
}
