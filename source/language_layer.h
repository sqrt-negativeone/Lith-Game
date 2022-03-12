
//~ NOTE(rjf): C Standard Library

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#define MemoryCopy memcpy
#define MemoryMove memmove
#define MemorySet  memset
#define CalculateCStringLength (u32)strlen
#define FMod fmodf
#define AbsoluteValue fabsf
#define SquareRoot sqrtf
#define Sin sinf
#define Cos cosf
#define Tan tanf
#define CStringToI32(s)            ((i32)atoi(s))
#define CStringToI16(s)            ((i16)atoi(s))
#define CStringToF32(s)            ((f32)atof(s))

//~ NOTE(rjf): Base Types

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef i8       b8;
typedef i16      b16;
typedef i32      b32;
typedef i64      b64;
typedef float    f32;
typedef double   f64;

//~ NOTE(rjf): Helper Macros

#define global         static
#define internal       static
#define local_persist  static
#define ArrayCount(a) (sizeof(a) / sizeof((a)[0]))
#define Bytes(n)      (n)
#define Kilobytes(n)  (n << 10)
#define Megabytes(n)  (n << 20)
#define Gigabytes(n)  (((u64)n) << 30)
#define Terabytes(n)  (((u64)n) << 40)
#define PI (3.1415926535897f)
#define SetFlag(flags, flag) ((flags) |= (flag))
#define ClearFlag(flags, flag) ((flags) &= ~(flag))
#define IsFlagSet(flags, flag) ((flags) & (flag))
#define Min(a,b) (((a) < (b))? (a):(b))
#define Max(a,b) (((a) > (b))? (a):(b))

#define USE_VSYNC_FOR_FPS_LIMIT 1

//~ NOTE(rjf): Random Number Generation

internal void
SeedRandomNumberGeneratorWithTime(void)
{
    // TODO(rjf): Get rid of rand()
    srand((unsigned int)time(0));
}

internal void
SeedRandomNumberGenerator(unsigned int seed)
{
    // TODO(rjf): Get rid of rand()
    srand(seed);
}

internal f32
RandomF32(f32 low, f32 high)
{
    // TODO(rjf): Get rid of rand()
    return low + (high - low) * ((rand() % 10000) / 10000.f);
}

//~ NOTE(rjf): Perlin Noise

#define BreakDebugger() _DebugBreak_Internal_()
#define Log(...)         _DebugLog(0,           __FILE__, __LINE__, __VA_ARGS__)
#define LogWarning(...)  _DebugLog(Log_Warning, __FILE__, __LINE__, __VA_ARGS__)
#define LogError(...)    _DebugLog(Log_Error,   __FILE__, __LINE__, __VA_ARGS__)

#define Log_Warning (1<<0)
#define Log_Error   (1<<1)

#undef Assert
#define AssertStatement HardAssert
#define Assert HardAssert
#define HardAssert(b) do { if(!(b)) { _AssertFailure(#b, __LINE__, __FILE__, 1); } } while(0)
#define SoftAssert(b) do { if(!(b)) { _AssertFailure(#b, __LINE__, __FILE__, 0); } } while(0)


void _AssertFailure(char *expression, int line, char *file, int crash);
void _DebugLog(i32 flags, char *file, int line, char *format, ...);
void _DebugBreak_Internal_(void);
void _BeginTimer(char *file, int line, char *format, ...);
void _EndTimer(void);
