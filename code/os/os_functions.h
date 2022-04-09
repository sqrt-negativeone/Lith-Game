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
typedef String8       OS_LoadEntireFile_Function(M_Arena *, String8);
typedef b32           OS_SaveToFile_Function(String8, String8List);
typedef b32           OS_AppendToFile_Function(String8, String8List);
typedef void          OS_DeleteFile_Function(String8);
typedef b32           OS_MakeDirectory_Function(String8);
typedef MessageResult OS_GetNextNetworkAMessageIfAvailable_Function(void);
typedef void          OS_PushNetworkMessage_Function(Message);

#endif //OS_FUNCTOINS_H
