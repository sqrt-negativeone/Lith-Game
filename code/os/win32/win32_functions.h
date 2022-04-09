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
#endif //WIN32_FUNCTIONS_H
