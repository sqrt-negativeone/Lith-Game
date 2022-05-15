
internal Rand_Ctx 
MakeRandCtx(u32 seed, u32 a, u32 c, u32 m)
{
    Rand_Ctx result;
    result.random_number = seed;
    result.a = a;
    result.c = c;
    result.m = m;
    NextRandomNumber(&result);
    return result;
}

internal Rand_Ctx
MakeLineraRandomGenerator(u32 seed)
{
    // NOTE(fakhri): linear congruential generator
    // see https://en.wikipedia.org/wiki/Linear_congruential_generator
    // paramters for the glibc used by GCC
    u32 a = 1103515245;
    u32 c = 12345;
    u32 m = 1u << 31;
    Rand_Ctx result = MakeRandCtx(seed, a, c, m);
    return result;
}

internal u32 
NextRandomNumber(Rand_Ctx *rand_ctx)
{
    rand_ctx->random_number = (rand_ctx->a * rand_ctx->random_number + rand_ctx->c) % rand_ctx->m; 
    u32 result = rand_ctx->random_number;
    return result;
}

internal f32
NextRandomNumberNF(Rand_Ctx *rand_ctx)
{
    u32 random_number = NextRandomNumber(rand_ctx);
    f32 normalized_float = (f32)random_number / (f32)rand_ctx->m;
    return normalized_float;
}
