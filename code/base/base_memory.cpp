
////////////////////////////////
//~ NOTE(fakhri): Arena Functions

internal M_Arena *
M_NilArena(void)
{
    return &m_g_nil_arena;
}

internal B32
M_ArenaIsNil(M_Arena *arena)
{
    return arena == 0 || arena == &m_g_nil_arena;
}

internal M_Arena *
M_ArenaAlloc(U64 cap)
{
    M_Arena *result = (M_Arena *)os->Reserve(cap);
    os->Commit(result, M_COMMIT_SIZE);
    
    result->memory      = result + sizeof(M_Arena);
    result->max         = cap;
    result->pos         = sizeof(M_Arena);
    result->commit_pos  = M_COMMIT_SIZE;
    result->align       = 8;
    return result;
}

internal M_Arena *
M_ArenaAllocDefault(void)
{
    return M_ArenaAlloc(Gigabytes(4));
}

internal void
M_ArenaRelease(M_Arena *arena)
{
    os->Release(arena);
}

internal void *
M_ArenaPushAligned(M_Arena *arena, U64 size, U64 align)
{
    void *memory = 0;
    align = ClampBot(arena->align, align);
    
    U64 pos = arena->pos;
    U64 pos_address = IntFromPtr(arena) + pos;
    U64 aligned_address = pos_address + align - 1;
    aligned_address -= aligned_address%align;
    
    U64 alignment_size = aligned_address - pos_address;
    if (pos + alignment_size + size <= arena->max)
    {
        U8 *mem_base = (U8*)arena;
        memory = mem_base + pos + alignment_size;
        U64 new_pos = pos + alignment_size + size;
        arena->pos = new_pos;
        
        if (new_pos > arena->commit_pos)
        {
            U64 commit_grow = new_pos - arena->commit_pos;
            commit_grow += M_COMMIT_SIZE - 1;
            commit_grow -= commit_grow%M_COMMIT_SIZE;
            os->Commit(mem_base + arena->commit_pos, commit_grow);
            arena->commit_pos += commit_grow;
        }
    }
    
    return(memory);
}

internal void *
M_ArenaPush(M_Arena *arena, U64 size)
{
    return M_ArenaPushAligned(arena, size, arena->align);
}

internal void *
M_ArenaPushZero(M_Arena *arena, U64 size)
{
    void *memory = M_ArenaPush(arena, size);
    MemoryZero(memory, size);
    return memory;
}

internal void
M_ArenaSetPosBack(M_Arena *arena, U64 pos)
{
    pos = Max(pos, sizeof(*arena));
    if(arena->pos > pos)
    {
        arena->pos = pos;
        
        U64 decommit_pos = pos;
        decommit_pos += M_COMMIT_SIZE - 1;
        decommit_pos -= decommit_pos%M_COMMIT_SIZE;
        U64 over_committed = arena->commit_pos - decommit_pos;
        over_committed -= over_committed%M_COMMIT_SIZE;
        if(decommit_pos > 0 && over_committed >= M_DECOMMIT_THRESHOLD)
        {
            os->Decommit((U8*)arena + decommit_pos, over_committed);
            arena->commit_pos -= over_committed;
        }
    }
}

internal void
M_ArenaSetAutoAlign(M_Arena *arena, U64 align)
{
    arena->align = align;
}

internal void
M_ArenaPop(M_Arena *arena, U64 size)
{
    M_ArenaSetPosBack(arena, arena->pos-size);
}

internal void
M_ArenaClear(M_Arena *arena)
{
    M_ArenaPop(arena, arena->pos);
}

internal U64
M_ArenaGetPos(M_Arena *arena)
{
    return arena->pos;
}

////////////////////////////////
//~ NOTE(fakhri): Temp

internal M_Temp
M_BeginTemp(M_Arena *arena)
{
    M_Temp result = {arena, arena->pos};
    return(result);
}

internal void
M_EndTemp(M_Temp temp)
{
    M_ArenaSetPosBack(temp.arena, temp.pos);
}
