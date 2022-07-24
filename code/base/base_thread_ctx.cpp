
per_thread Thread_Ctx *tl_tctx = 0;

internal Thread_Ctx
MakeTCTX(void)
{
    Thread_Ctx tctx = {0};
    for(U64 arena_idx = 0; arena_idx < ArrayCount(tctx.arenas); arena_idx += 1)
    {
        tctx.arenas[arena_idx] = M_ArenaAlloc(Gigabytes(8));
    }
    return tctx;
}

internal void
SetTCTX(Thread_Ctx *tctx)
{
    tl_tctx = tctx;
}

internal Thread_Ctx *
GetTCTX(void)
{
    Thread_Ctx *result = tl_tctx;
    return result;
}

internal void
DeleteTCTX(Thread_Ctx *tctx)
{
    for(U64 arena_idx = 0; arena_idx < ArrayCount(tctx->arenas); arena_idx += 1)
    {
        M_ArenaRelease(tctx->arenas[arena_idx]);
    }
}

internal M_Temp
GetScratch(M_Arena **conflicts, U64 conflict_count)
{
    M_Temp scratch = {0};
    Thread_Ctx *tctx = GetTCTX();
    
    for(u64 arena_index_in_tctx = 0;
        arena_index_in_tctx < ArrayCount(tctx->arenas);
        ++arena_index_in_tctx)
    {
        M_Arena *arena_in_tctx = tctx->arenas[arena_index_in_tctx];
        b32 is_conflicting = false;
        
        for(u64 arena_index_in_conflect = 0;
            arena_index_in_conflect< conflict_count;
            ++arena_index_in_conflect)
        {
            M_Arena *conflict_arena = conflicts[arena_index_in_conflect];
            if(conflict_arena == arena_in_tctx)
            {
                is_conflicting = true;
                break;
            }
        }
        if(!is_conflicting)
        {
            scratch.arena = arena_in_tctx;
            scratch.pos = scratch.arena->pos;
            break;
        }
    }
    return scratch;
}
