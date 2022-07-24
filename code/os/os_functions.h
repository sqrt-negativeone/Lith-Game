/* date = March 26th 2022 7:26 pm */

#ifndef OS_FUNCTOINS_H
#define OS_FUNCTOINS_H

//~ NOTE(fakhri): Memory Function types
typedef void  OS_Release_Fucntion(void *);
typedef void *OS_Reserve_Function(u64);
typedef void  OS_Commit_Function(void *, u64);
typedef void  OS_Decommit_Function(void *, u64);
typedef U64   OS_PageSize_Function(void);

typedef void  OS_OutputError_Function(char *, char *, ...);

// NOTE(fakhri): File Functions
typedef String8  OS_LoadEntireFile_Function(M_Arena *, String8);
typedef b32      OS_SaveToFile_Function(String8, String8List);
typedef b32      OS_AppendToFile_Function(String8, String8List);
typedef void     OS_DeleteFile_Function(String8);
typedef b32      OS_MakeDirectory_Function(String8);

// NOTE(fakhri): network functions
typedef b32      OS_IsHostMessageQueueEmpty_Function(void);
typedef Message *OS_BeginHostMessageQueueRead_Function(void);
typedef void     OS_EndHostMessageQueueRead_Function(void);
typedef Message *OS_BeginPlayerMessageQueueWrite_Function(void);
typedef void     OS_EndPlayerMessageQueueWrite_Function(void);

typedef Socket_Handle OS_AcceptSocket_Function(Socket_Handle s, void *addr, int *addrlen);
typedef void          OS_CloseSocket_Function(Socket_Handle s);
typedef Socket_Handle OS_ConnectToServer_Function(char *server_address, char *port);
typedef Socket_Handle OS_OpenListenSocket_Function(char *port);
typedef b32           OS_SendBuffer_Function(Socket_Handle s, void *data, i32 len);
typedef b32           OS_ReceiveBuffer_Function(Socket_Handle s, void *data, i32 len);
typedef b32           OS_SendString_Function(Socket_Handle s, String8 data);
typedef b32           OS_ReceiveString_Function(Socket_Handle s, String8 *data);

// NOTE(fakhri): OS Handles
typedef void         OS_WaitForSemaphore_Function(Semaphore_Handle semaphore);
typedef void         OS_ReleaseSemaphore_Function(Semaphore_Handle semaphore);
typedef void         OS_WaitForMutex_Function(Mutex_Handle mutex);
typedef Mutex_Handle OS_CreateMutex_Function();
typedef void         OS_ReleaseMutex_Function(Mutex_Handle mutex);

//- NOTE(fakhri): work queue functions
typedef OS_PopQueueResult OS_PopQueueEntry_Function();
typedef b32               OS_ProcessOneWorkQueueEntry_Function();
typedef b32               OS_PushWorkQueueEntrySP_Function(OS_WorkThreadWork *work, void *data);
typedef b32               OS_PushWorkQueueEntry_Function(OS_WorkThreadWork *work, void *data);
typedef b32               OS_IsWorkQueueEmpty_Function();

//- NOTE(fakhri): clipboard functions
typedef void              OS_CopyStringToClipboard_Function(String text);

#endif //OS_FUNCTOINS_H
