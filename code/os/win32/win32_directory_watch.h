/* date = March 30th 2022 4:33 pm */

#ifndef WIN32_DIRECTORY_WATCH_H
#define WIN32_DIRECTORY_WATCH_H


// NOTE(fakhri): directory watcher
struct Directory_Watcher
{
    HANDLE dir_handle;
    OVERLAPPED *overlapped;
    char buffer[1024];
    DWORD bytes_filled;
};

internal void W32_BeginWatchDirectory(Directory_Watcher *watcher, M_Arena *arena, String8 dir_path);

internal String8 W32_CheckDirectoryChanges(M_Arena *arena, Directory_Watcher *dir_watch);
#endif //WIN32_DIRECTORY_WATCH_H
