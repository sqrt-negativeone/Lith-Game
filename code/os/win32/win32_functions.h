/* date = March 27th 2022 1:34 pm */

#ifndef WIN32_FUNCTIONS_H
#define WIN32_FUNCTIONS_H

global b32 w32_got_system_info;
global SYSTEM_INFO w32_system_info;

//- NOTE(fakhri): Memory internals
internal void *W32_HeapAlloc(u32 size);
internal void  W32_HeapFree(void *data);
internal void  W32_Release(void *memory);
internal void *W32_Reserve(u64 size);
internal void  W32_Commit(void *memory, u64 size);
internal void  W32_Decommit(void *memory, u64 size);
internal U64   W32_PageSize(void);

//- NOTE(fakhri): output error internal
internal void W32_OutputError(char *title, char *format, ...);

//- NOTE(fakhri): file internals
internal String8 W32_LoadEntireFile(M_Arena *arena, String8 path);
internal B32     W32_SaveToFile(String8 path, String8List data);
internal b32     W32_AppendToFile(String8 path, String8List data);
internal void    W32_DeleteFile(String8 path);
internal B32     W32_MakeDirectory(String8 path);

//- NOTE(fakhri): Networking internals
internal b32          W32_IsHostMessageQueueEmpty();
internal Message     *W32_BeginHostMessageQueueRead();
internal void         W32_EndHostMessageQueueRead();
internal Message     *W32_BeginPlayerMessageQueueWrite();
internal void         W32_EndPlayerMessageQueueWrite();

internal Socket_Handle W32_AcceptSocket(Socket_Handle s, void *addr, int *addrlen);
internal void          W32_CloseSocket(Socket_Handle s);
internal Socket_Handle W32_ConnectToServer(char *server_address, char *port);
internal Socket_Handle W32_OpenListenSocket(char *port);
internal b32           W32_SendBuffer(Socket_Handle s, void *data, i32 len);
internal b32           W32_ReceiveBuffer(Socket_Handle s, void *data, i32 len);
internal b32           W32_SendString(Socket_Handle s, String8 data);
internal b32           W32_ReceiveString(Socket_Handle s, String8 *data);

//- NOTE(fakhri): Os Handles
internal void         W32_WaitForSemaphore(Semaphore_Handle semaphore);
internal void         W32_ReleaseSemaphore(Semaphore_Handle semaphore);
internal void         W32_WaitForMutex(Mutex_Handle mutex);
internal Mutex_Handle W32_CreateMutex();
internal void         W32_ReleaseMutex(Mutex_Handle mutex);

//- NOTE(fakhri): work queue functions
internal OS_PopQueueResult W32_PopQueueEntry();
internal b32               W32_ProcessOneWorkQueueEntry();
internal b32               W32_PushWorkQueueEntrySP(OS_WorkThreadWork *work, void *data);
internal b32               W32_PushWorkQueueEntry(OS_WorkThreadWork *work, void *data);
internal b32               W32_IsWorkQueueEmpty();

#endif //WIN32_FUNCTIONS_H
