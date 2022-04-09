/* date = March 27th 2022 2:23 pm */

#ifndef BASE_THREAD_CTX_H
#define BASE_THREAD_CTX_H

#define ARENAS_PER_THREAD_CTX_COUNT 2
struct Thread_Ctx
{
    M_Arena *arenas[ARENAS_PER_THREAD_CTX_COUNT];
};

internal Thread_Ctx MakeTCTX(void);
internal void SetTCTX(Thread_Ctx *tctx);
internal Thread_Ctx *GetTCTX(void);

internal M_Temp GetScratch(M_Arena **conflicts, U64 conflict_count);
#define ReleaseScratch(temp) M_EndTemp(temp)

#endif //BASE_THREAD_CTX_H
