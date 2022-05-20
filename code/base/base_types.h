/* date = April 8th 2022 11:08 pm */

#ifndef BASE_TYPES_H
#define BASE_TYPES_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

////////////////////////////////
//~ NOTE(fakhri): "Keywords"

#define global         static
#define internal       static
#define local_persist  static

#if LANG_CPP
# if OS_WINDOWS
#  define exported extern "C" __declspec(dllexport)
# else
#  define exported extern "C"
# endif
#else
# if OS_WINDOWS
#  define exported __declspec(dllexport)
# else
#  define exported
# endif
#endif

#if LANG_CPP
# if OS_WINDOWS
#  define imported extern "C" __declspec(dllimport)
# else
#  define imported extern "C"
# endif
#else
# if OS_WINDOWS
#  define imported __declspec(dllimport)
# else
#  define imported
# endif
#endif

#if COMPILER_CL
# define per_thread __declspec(thread)
#elif COMPILER_CLANG
# define per_thread __thread
#elif COMPILER_GCC
# define per_thread __thread
#endif

#if COMPILER_CL && COMPILER_CL_YEAR < 2015
# define inline_internal static
#else
# define inline_internal inline static
#endif

#if COMPILER_CL && COMPILER_CL_YEAR < 2015
# define this_internal_name "unknown"
#else
# define this_internal_name __func__
#endif

#define fallthrough

#if OS_WINDOWS
# pragma section(".roglob", read)
# define read_only __declspec(allocate(".roglob"))
#else
// TODO(rjf): figure out if this benefit is possible on non-Windows
# define read_only
#endif


////////////////////////////////
//~ NOTE(fakhri): Helper Macros

#define MemoryCopy memcpy
#define MemoryMove memmove
#define MemorySet  memset

#define MemoryCopyStruct(d,s) do { Assert(sizeof(*(d))==sizeof(*(s))); MemoryCopy((d),(s),sizeof(*(d))); } while(0)
#define MemoryCopyArray(d,s) do{ Assert(sizeof(d)==sizeof(s)); MemoryCopy((d),(s),sizeof(s)); }while(0)

#define MemoryZero(p,s) MemorySet((p), 0, (s))
#define MemoryZeroStruct(p) MemoryZero((p), sizeof(*(p)))
#define MemoryZeroArray(a) MemoryZero((a), sizeof(a))

#define ArrayCount(a) (sizeof(a) / sizeof((a)[0]))
#define IntFromPtr(p) (U64)(((U8*)p) - 0)
#define PtrFromInt(i) (void*)(((U8*)0) + i)
#define Member(S,m) ((S*)0)->m
#define OffsetOf(S,m) IntFromPtr(&Member(S,m))
#define CastFromMember(S,m,p) (S*)(((U8*)p) - OffsetOf(S,m))
#define MemberFromOffset(ptr, off, type) *(type *)((U8 *)(ptr) + off)
#define UnusedVariable(name) (void)name

#define Bytes(n)      (n)
#define Kilobytes(n)  (n << 10)
#define Megabytes(n)  (n << 20)
#define Gigabytes(n)  (((U64)n) << 30)
#define Terabytes(n)  (((U64)n) << 40)

#define Thousand(x) ((x)*1000)
#define Million(x) ((x)*1000000)
#define Billion(x) ((x)*1000000000LL)

#define MilliSeconds(n) (1000.0 * (n))
#define Seconds(n) (n)
#define MinutesInSeconds(n) (n / 60.0)
#define HourseInSeconds(n) (MinutesInSeconds(n) / 60.0)

#define Meter(x) (x)
#define CentiMeter(x) ((x) / 100.0f)
#define MiliMeter(x) ((x) / 1000.0f)

#define HasFlag(fl,fi) ((fl)&(fi))
#define SetFlag(fl,fi) ((fl)|=(fi))
#define ClearFlag(fl,fi) ((fl)&=~(fi))
#define ToggleFlag(fl,fi) ((fl)^=(fi))

#define Swap(T,a,b) do{ T t__ = a; a = b; b = t__; }while(0)

////////////////////////////////
//~ NOTE(fakhri): Basic Types

typedef int8_t   S8;
typedef int16_t  S16;
typedef int32_t  S32;
typedef int64_t  S64;
typedef uint8_t  U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;
typedef S8       B8;
typedef S16      B16;
typedef S32      B32;
typedef S64      B64;
typedef float    F32;
typedef double   F64;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef S8       b8;
typedef S16      b16;
typedef S32      b32;
typedef S64      b64;
typedef float    f32;
typedef double   f64;
typedef u8      *byte_ptr;
typedef void VoidFunction(void);
////////////////////////////////
//~ rjf: Limits

read_only global U8 U8Max = 0xFF;
read_only global U8 U8Min = 0;

read_only global U16 U16Max = 0xFFFF;
read_only global U16 U16Min = 0;

read_only global U32 U32Max = 0xFFFFFFFF;
read_only global U32 U32Min = 0;

read_only global U64 U64Max = 0xFFFFFFFFFFFFFFFF;
read_only global U64 U64Min = 0;

read_only global S8 S8Max = 0x7F;
read_only global S8 S8Min = -1 - 0x7F;

read_only global S16 S16Max = 0x7FFF;
read_only global S16 S16Min = -1 - 0x7FFF;

read_only global S32 S32Max = 0x7FFFFFFF;
read_only global S32 S32Min = -1 - 0x7FFFFFFF;

read_only global S64 S64Max = 0x7FFFFFFFFFFFFFFF;
read_only global S64 S64Min = -1 - 0x7FFFFFFFFFFFFFFF;

read_only global U32 SignF32 = 0x80000000;
read_only global U32 ExponentF32 = 0x7F800000;
read_only global U32 MantissaF32 = 0x7FFFFF;

////////////////////////////////
//~ NOTE(fakhri): Constants

read_only global U64 Bitmask[] =
{
    0x0,
    0x1,
    0x3,
    0x7,
    0xF,
    0x1F,
    0x3F,
    0x7F,
    0xFF,
    0x1FF,
    0x3FF,
    0x7FF,
    0xFFF,
    0x1FFF,
    0x3FFF,
    0x7FFF,
    0xFFFF,
    0x1FFFF,
    0x3FFFF,
    0x7FFFF,
    0xFFFFF,
    0x1FFFFF,
    0x3FFFFF,
    0x7FFFFF,
    0xFFFFFF,
    0x1FFFFFF,
    0x3FFFFFF,
    0x7FFFFFF,
    0xFFFFFFF,
    0x1FFFFFFF,
    0x3FFFFFFF,
    0x7FFFFFFF,
    0xFFFFFFFF,
    0x1FFFFFFFF,
    0x3FFFFFFFF,
    0x7FFFFFFFF,
    0xFFFFFFFFF,
    0x1FFFFFFFFF,
    0x3FFFFFFFFF,
    0x7FFFFFFFFF,
    0xFFFFFFFFFF,
    0x1FFFFFFFFFF,
    0x3FFFFFFFFFF,
    0x7FFFFFFFFFF,
    0xFFFFFFFFFFF,
    0x1FFFFFFFFFFF,
    0x3FFFFFFFFFFF,
    0x7FFFFFFFFFFF,
    0xFFFFFFFFFFFF,
    0x1FFFFFFFFFFFF,
    0x3FFFFFFFFFFFF,
    0x7FFFFFFFFFFFF,
    0xFFFFFFFFFFFFF,
    0x1FFFFFFFFFFFFF,
    0x3FFFFFFFFFFFFF,
    0x7FFFFFFFFFFFFF,
    0xFFFFFFFFFFFFFF,
    0x1FFFFFFFFFFFFFF,
    0x3FFFFFFFFFFFFFF,
    0x7FFFFFFFFFFFFFF,
    0xFFFFFFFFFFFFFFF,
    0x1FFFFFFFFFFFFFFF,
    0x3FFFFFFFFFFFFFFF,
    0x7FFFFFFFFFFFFFFF,
    0xFFFFFFFFFFFFFFFF,
};

read_only global U32 Bit1  = 1u << 0;
read_only global U32 Bit2  = 1u << 1;
read_only global U32 Bit3  = 1u << 2;
read_only global U32 Bit4  = 1u << 3;
read_only global U32 Bit5  = 1u << 4;
read_only global U32 Bit6  = 1u << 5;
read_only global U32 Bit7  = 1u << 6;
read_only global U32 Bit8  = 1u << 7;
read_only global U32 Bit9  = 1u << 8;
read_only global U32 Bit10 = 1u << 9;
read_only global U32 Bit11 = 1u << 10;
read_only global U32 Bit12 = 1u << 11;
read_only global U32 Bit13 = 1u << 12;
read_only global U32 Bit14 = 1u << 13;
read_only global U32 Bit15 = 1u << 14;
read_only global U32 Bit16 = 1u << 15;
read_only global U32 Bit17 = 1u << 16;
read_only global U32 Bit18 = 1u << 17;
read_only global U32 Bit19 = 1u << 18;
read_only global U32 Bit20 = 1u << 19;
read_only global U32 Bit21 = 1u << 20;
read_only global U32 Bit22 = 1u << 21;
read_only global U32 Bit23 = 1u << 22;
read_only global U32 Bit24 = 1u << 23;
read_only global U32 Bit25 = 1u << 24;
read_only global U32 Bit26 = 1u << 25;
read_only global U32 Bit27 = 1u << 26;
read_only global U32 Bit28 = 1u << 27;
read_only global U32 Bit29 = 1u << 28;
read_only global U32 Bit30 = 1u << 29;
read_only global U32 Bit31 = 1u << 30;
read_only global U32 Bit32 = 1u << 31;

read_only global f32 F32Max = 3.4028234664e+38f;
read_only global f32 F32Min = -3.4028234664e+38f;
read_only global f32 F32SmallestPositive = 1.1754943508e-38f;
read_only global f32 F32Epsilon = 5.96046448e-8f;
read_only global f32 F32Tau = 6.28318530718f;
read_only global f32 F32Pi = 3.14159265359f;


////////////////////////////////
//~ NOTE(fakhri): Assertions

#undef Assert
#define Assert(b) do { if(!(b)) { (*(volatile int *)0 = 0); } } while(0)
#define StaticAssert(c,label) U8 static_assert_##label[(c)?(1):(-1)]
#define NotImplemented Assert(!"Not Implemented")
#define StopExecution Assert(!"Stopping Execution")
#define InvalidPath Assert(!"Invalid Path")

////////////////////////////////
//~ NOTE(fakhri): Logging
#define Log(...)         _DebugLog(0,           __FILE__, __LINE__, __VA_ARGS__)
#define LogWarning(...)  _DebugLog(Log_Warning, __FILE__, __LINE__, __VA_ARGS__)
#define LogError(...)    _DebugLog(Log_Error,   __FILE__, __LINE__, __VA_ARGS__)

#define Log_Warning (1<<0)
#define Log_Error   (1<<1)

internal void _DebugLog(i32 flags, char *file, int line, char *format, ...);

////////////////////////////////
//~ rjf: Bit Patterns

inline_internal F32
AbsoluteValueF32(F32 f)
{
    union { U32 u; F32 f; } x;
    x.f = f;
    x.u = x.u & ~SignF32;
    return(x.f);
}

////////////////////////////////
//~ rjf: Base Enums

typedef enum Side
{
    Side_Invalid = -1,
    Side_Min,
    Side_Max,
    Side_COUNT
}
Side;

typedef enum Axis2
{
    Axis2_X,
    Axis2_Y,
    Axis2_COUNT
}
Axis2;

typedef enum Axis3
{
    Axis3_X,
    Axis3_Y,
    Axis3_Z,
    Axis3_COUNT
}
Axis3;

typedef enum Axis4
{
    Axis4_X,
    Axis4_Y,
    Axis4_Z,
    Axis4_W,
    Axis4_COUNT
}
Axis4;

typedef enum DimensionAxis
{
    Dimension_X,
    Dimension_Y,
    Dimension_Z,
    Dimension_W,
}
DimensionAxis;

#endif //BASE_TYPES_H
