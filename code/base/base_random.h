/* date = May 15th 2022 1:22 pm */

#ifndef BASE_RANDOM_H
#define BASE_RANDOM_H

struct Rand_Ctx
{
    u32 random_number;
    u32 a;
    u32 c;
    u32 m;
};

internal Rand_Ctx MakeRandCtx(u32 seed, u32 a, u32 c, u32 m);
internal Rand_Ctx MakeLineraGenerator(u32 seed);
internal u32      NextRandomNumber(Rand_Ctx *rand_ctx);
// NOTE(fakhri): get normalized random float number
internal f32      NextRandomNumberNF(Rand_Ctx *rand_ctx);

#endif //BASE_RANDOM_H
