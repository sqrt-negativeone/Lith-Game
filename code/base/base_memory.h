/* date = March 26th 2022 4:26 pm */

#ifndef BASE_MEMORY_H
#define BASE_MEMORY_H

////////////////////////////////
//~ NOTE(fakhri): Limits

#if !defined(M_COMMIT_SIZE)
#define M_COMMIT_SIZE Kilobytes(4)
#endif

#if !defined(M_DECOMMIT_THRESHOLD)
#define M_DECOMMIT_THRESHOLD Kilobytes(64)
#endif

////////////////////////////////
//~ NOTE(fakhri): Arena

typedef struct M_Arena M_Arena;
struct M_Arena
{
    void *memory;
    U64 commit_pos;
    U64 max;
    U64 pos;
    U64 align;
};

////////////////////////////////
//~ NOTE(fakhri): Arena Helpers

typedef struct M_Temp M_Temp;
struct M_Temp
{
    M_Arena *arena;
    U64 pos;
};

////////////////////////////////
//~ NOTE(fakhri): Globals

read_only global M_Arena m_g_nil_arena = {};

////////////////////////////////
//~ NOTE(fakhri): Arena Functions

#define M_CheckNilArena(p) (M_ArenaIsNil(p))
#define M_SetNilArena(p) ((p) = M_NilArena())
internal M_Arena *M_NilArena(void);
internal B32      M_ArenaIsNil(M_Arena *arena);
internal M_Arena *M_ArenaAlloc(U64 cap);
internal M_Arena *M_ArenaAllocDefault(void);
internal void     M_ArenaRelease(M_Arena *arena);
internal void    *M_ArenaPushAligned(M_Arena *arena, U64 size, U64 align);
internal void    *M_ArenaPush(M_Arena *arena, U64 size);
internal void    *M_ArenaPushZero(M_Arena *arena, U64 size);
internal void     M_ArenaSetPosBack(M_Arena *arena, U64 pos);
internal void     M_ArenaSetAutoAlign(M_Arena *arena, U64 align);
internal void     M_ArenaPop(M_Arena *arena, U64 size);
internal void     M_ArenaClear(M_Arena *arena);
internal U64      M_ArenaGetPos(M_Arena *arena);
#define PushArray(a,T,c)     (T*)M_ArenaPush((a), sizeof(T)*(c))
#define PushArrayZero(a,T,c) (T*)M_ArenaPushZero((a), sizeof(T)*(c))
#define PushStruct(a, T)     PushArray(a, T, 1)
#define PushStructZero(a, T)     PushArrayZero(a, T, 1)

////////////////////////////////
//~ NOTE(fakhri): Temp

internal M_Temp M_BeginTemp(M_Arena *arena);
internal void M_EndTemp(M_Temp temp);

#endif //BASE_MEMORY_H
