#define M_ARENA_MAX          Gigabytes(4)
#define M_ARENA_COMMIT_SIZE  Kilobytes(4)

internal M_Arena
M_ArenaInitialize(void)
{
    M_Arena arena = {0};
    arena.max = M_ARENA_MAX;
    arena.base = os->Reserve(arena.max);
    arena.alloc_position = 0;
    arena.commit_position = 0;
    return arena;
}

internal void *
M_ArenaPush(M_Arena *arena, u64 size)
{
    void *memory = 0;
    if(arena->alloc_position + size > arena->commit_position)
    {
        u64 commit_size = size;
        commit_size += M_ARENA_COMMIT_SIZE-1;
        commit_size -= commit_size % M_ARENA_COMMIT_SIZE;
        os->Commit((u8 *)arena->base + arena->commit_position, commit_size);
        arena->commit_position += commit_size;
    }
    memory = (u8 *)arena->base + arena->alloc_position;
    arena->alloc_position += size;
    return memory;
}

internal void *
M_ArenaPushAligned(M_Arena *arena, u64 size, u64 alignment)
{
    void *memory = M_ArenaPush(arena, size + alignment);
    if (memory)
    {
        u64 reminder = (u64)memory % alignment;
        memory = (void *)((u64)memory + alignment - reminder);
    }
    return memory;
}

internal void *
M_ArenaPushZero(M_Arena *arena, u64 size)
{
    void *memory = M_ArenaPush(arena, size);
    MemorySet(memory, 0, size);
    return memory;
}

internal void
M_ArenaPop(M_Arena *arena, u64 size)
{
    if(size > arena->alloc_position)
    {
        size = arena->alloc_position;
    }
    arena->alloc_position -= size;
}

internal void
M_ArenaClear(M_Arena *arena)
{
    M_ArenaPop(arena, arena->alloc_position);
}

internal void
M_ArenaRelease(M_Arena *arena)
{
    os->Release(arena->base);
}