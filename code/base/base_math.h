/* date = March 26th 2022 6:06 pm */

#ifndef BASE_MATH_H
#define BASE_MATH_H
// NOTE(fakhri): this is a modified version of HandmadeMath

/* let's figure out if SSE is really available (unless disabled anyway)
   (it isn't on non-x86/x86_64 platforms or even x86 without explicit SSE support)
   => only use "#ifdef HANDMADE_MATH__USE_SSE" to check for SSE support below this block! */
#ifndef HANDMADE_MATH_NO_SSE
# ifdef COMPILER_CL
/* MSVC supports SSE in amd64 mode or _M_IX86_FP >= 1 (2 means SSE2) */
#  if defined(ARCH_X64) || ( defined(_M_IX86_FP) && _M_IX86_FP >= 1 )
#   define HANDMADE_MATH__USE_SSE 1
#  endif
# else /* not MSVC, probably GCC, clang, icc or something that doesn't support SSE anyway */
#  ifdef __SSE__ /* they #define __SSE__ if it's supported */
#   define HANDMADE_MATH__USE_SSE 1
#  endif /*  __SSE__ */
# endif /* not _MSC_VER */

#endif /* #ifndef HANDMADE_MATH_NO_SSE */

#ifdef HANDMADE_MATH__USE_SSE
#include <xmmintrin.h>
#endif

#include <math.h>

#define PI32 3.14159265359f
#define PI 3.14159265358979323846

#define Min(a, b) ((a) > (b) ? (b) : (a))
#define Max(a, b) ((a) < (b) ? (b) : (a))
#define ClampTop(x,a) Min(x,a)
#define ClampBot(a,x) Max(a,x)
#define AbsVal(a) (((a) >= 0)? (a):-(a))

#define Mod(a, m) (((a) % (m)) >= 0 ? ((a) % (m)) : (((a) % (m)) + (m)))
#define Square(x) ((x) * (x))

//~
// Floating-point math functions
//

internal inline f32 
SinF(f32 Radians)
{
    
    f32 Result = sinf(Radians);
    
    return (Result);
}

internal inline f32
CosF(f32 Radians)
{
    f32 Result = cosf(Radians);
    
    return (Result);
}

internal inline f32
TanF(f32 Radians)
{
    f32 Result = tanf(Radians);
    
    return (Result);
}

internal inline f32
ACosF(f32 Radians)
{
    f32 Result = acosf(Radians);
    
    return (Result);
}

internal inline f32 
ATanF(f32 Radians)
{
    f32 Result = atanf(Radians);
    
    return (Result);
}

internal inline f32 
ATan2F(f32 Left, f32 Right)
{
    f32 Result = atan2f(Left, Right);
    
    return (Result);
}

internal inline f32 
ExpF(f32 Float)
{
    f32 Result = expf(Float);
    
    return (Result);
}

internal inline f32 
LogF(f32 Float)
{
    f32 Result = logf(Float);
    
    return (Result);
}

internal inline f32 
SquareRootF(f32 Float)
{
    f32 Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    __m128 In = _mm_set_ss(Float);
    __m128 Out = _mm_sqrt_ss(In);
    Result = _mm_cvtss_f32(Out);
#else
    Result = sqrtf(Float);
#endif
    
    return(Result);
}

internal inline f32 
RSquareRootF(f32 Float)
{
    f32 Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    __m128 In = _mm_set_ss(Float);
    __m128 Out = _mm_rsqrt_ss(In);
    Result = _mm_cvtss_f32(Out);
#else
    Result = 1.0f/SquareRootF(Float);
#endif
    
    return(Result);
}

internal inline f32 
Power(f32 Base, int Exponent)
{
    f32 Result = 1.0f;
    f32 Mul = Exponent < 0 ? 1.f / Base : Base;
    int X = Exponent < 0 ? -Exponent : Exponent;
    while (X)
    {
        if (X & 1)
        {
            Result *= Mul;
        }
        
        Mul *= Mul;
        X >>= 1;
    }
    
    return (Result);
}

internal inline f32 
PowerF(f32 Base, f32 Exponent)
{
    f32 Result = expf(Exponent * logf(Base));
    
    return (Result);
}


//~
//Utility functions
//

internal inline f32 
ToRadians(f32 Degrees)
{
    f32 Result = Degrees * (PI32 / 180.0f);
    
    return (Result);
}

internal inline f32 
Lerp(f32 A, f32 Time, f32 B)
{
    f32 Result = (1.0f - Time) * A + Time * B;
    
    return (Result);
}

internal inline f32 
Clamp(f32 min, f32 Value, f32 max)
{
    f32 Result = Value;
    
    if(Result < min)
    {
        Result = min;
    }
    
    if(Result > max)
    {
        Result = max;
    }
    
    return (Result);
}


//~
// Vec2f32 stuff
//

typedef union vec2f32 v2;
union vec2f32
{
    struct
    {
        f32 X, Y;
    };
    struct
    {
        f32 x, y;
    };
    
    struct
    {
        f32 U, V;
    };
    
    struct
    {
        f32 u, v;
    };
    
    struct
    {
        f32 Left, Right;
    };
    
    struct
    {
        f32 left, right;
    };
    
    struct
    {
        f32 Width, Height;
    };
    
    struct
    {
        f32 width, height;
    };
    
    f32 Elements[2];
    
#ifdef LANG_CPP
    inline f32 &operator[](const int &Index)
    {
        return Elements[Index];
    }
#endif
};

internal inline vec2f32 
Vec2f32(f32 x, f32 y)
{
    vec2f32 Result;
    
    Result.x = x;
    Result.y = y;
    
    return (Result);
}

internal inline vec2f32 
Vec2(f32 x, f32 y)
{
    vec2f32 Result = Vec2f32(x, y);
    return (Result);
}


internal inline vec2f32 
AddVec2(vec2f32 Left, vec2f32 Right)
{
    vec2f32 Result;
    
    Result.X = Left.X + Right.X;
    Result.Y = Left.Y + Right.Y;
    
    return (Result);
}


internal inline vec2f32 
SubtractVec2(vec2f32 Left, vec2f32 Right)
{
    vec2f32 Result;
    
    Result.X = Left.X - Right.X;
    Result.Y = Left.Y - Right.Y;
    
    return (Result);
}


internal inline vec2f32 
HadamardMultiplyVec2(vec2f32 Left, vec2f32 Right)
{
    vec2f32 Result;
    
    Result.X = Left.X * Right.X;
    Result.Y = Left.Y * Right.Y;
    
    return (Result);
}

internal inline vec2f32 
MultiplyVec2f(vec2f32 Left, f32 Right)
{
    vec2f32 Result;
    
    Result.X = Left.X * Right;
    Result.Y = Left.Y * Right;
    
    return (Result);
}

internal inline vec2f32 
DivideVec2(vec2f32 Left, vec2f32 Right)
{
    vec2f32 Result;
    
    Result.X = Left.X / Right.X;
    Result.Y = Left.Y / Right.Y;
    
    return (Result);
}

internal inline vec2f32 
DivideVec2f(vec2f32 Left, f32 Right)
{
    vec2f32 Result;
    
    Result.X = Left.X / Right;
    Result.Y = Left.Y / Right;
    
    return (Result);
}

internal inline bool 
EqualsVec2(vec2f32 Left, vec2f32 Right)
{
    bool Result = (Left.X == Right.X && Left.Y == Right.Y);
    
    return (Result);
}

internal inline f32 
DotVec2(vec2f32 VecOne, vec2f32 VecTwo)
{
    f32 Result = (VecOne.X * VecTwo.X) + (VecOne.Y * VecTwo.Y);
    
    return (Result);
}

internal inline f32 
LengthSquaredVec2(vec2f32 A)
{
    f32 Result = DotVec2(A, A);
    
    return (Result);
}

internal inline f32 
LengthVec2(vec2f32 A)
{
    f32 Result = SquareRootF(LengthSquaredVec2(A));
    
    return (Result);
}

internal inline vec2f32 
NormalizeVec2(vec2f32 A)
{
    vec2f32 Result = {0};
    
    f32 VectorLength = LengthVec2(A);
    
    /* NOTE(kiljacken): We need a zero check to not divide-by-zero */
    if (VectorLength != 0.0f)
    {
        Result.X = A.X * (1.0f / VectorLength);
        Result.Y = A.Y * (1.0f / VectorLength);
    }
    
    return (Result);
}

internal inline vec2f32 
FastNormalizeVec2(vec2f32 A)
{
    return MultiplyVec2f(A, RSquareRootF(DotVec2(A, A)));
}

#if LANG_CPP
internal inline f32 
Length(vec2f32 A)
{
    f32 Result = LengthVec2(A);
    return (Result);
}

internal inline vec2f32 
Normalize(vec2f32 A)
{
    vec2f32 Result = NormalizeVec2(A);
    
    return (Result);
}


internal inline vec2f32 
FastNormalize(vec2f32 A)
{
    vec2f32 Result = FastNormalizeVec2(A);
    
    return (Result);
}

internal inline f32 
Dot(vec2f32 VecOne, vec2f32 VecTwo)
{
    f32 Result = DotVec2(VecOne, VecTwo);
    
    return (Result);
}

internal inline vec2f32 
Add(vec2f32 Left, vec2f32 Right)
{
    vec2f32 Result = AddVec2(Left, Right);
    
    return (Result);
}

internal inline vec2f32 
Subtract(vec2f32 Left, vec2f32 Right)
{
    vec2f32 Result = SubtractVec2(Left, Right);
    
    return (Result);
}

internal inline vec2f32 
Multiply(vec2f32 Left, f32 Right)
{
    vec2f32 Result = MultiplyVec2f(Left, Right);
    
    return (Result);
}

internal inline vec2f32 
Divide(vec2f32 Left, vec2f32 Right)
{
    vec2f32 Result = DivideVec2(Left, Right);
    
    return (Result);
}

internal inline vec2f32 
Divide(vec2f32 Left, f32 Right)
{
    vec2f32 Result = DivideVec2f(Left, Right);
    
    return (Result);
}

internal inline bool 
Equals(vec2f32 Left, vec2f32 Right)
{
    bool Result = EqualsVec2(Left, Right);
    
    return (Result);
}

internal inline vec2f32 
operator+(vec2f32 Left, vec2f32 Right)
{
    vec2f32 Result = AddVec2(Left, Right);
    return (Result);
}

internal inline vec2f32 
operator-(vec2f32 Left, vec2f32 Right)
{
    vec2f32 Result = SubtractVec2(Left, Right);
    return (Result);
}

internal inline vec2f32 
operator*(vec2f32 Left, f32 Right)
{
    
    vec2f32 Result = MultiplyVec2f(Left, Right);
    
    return (Result);
}

internal inline vec2f32 
operator*(f32 Left, vec2f32 Right)
{
    
    vec2f32 Result = MultiplyVec2f(Right, Left);
    
    return (Result);
}

internal inline vec2f32 
operator/(vec2f32 Left, vec2f32 Right)
{
    
    vec2f32 Result = DivideVec2(Left, Right);
    
    return (Result);
}

internal inline vec2f32 
operator/(vec2f32 Left, f32 Right)
{
    
    vec2f32 Result = DivideVec2f(Left, Right);
    
    return (Result);
}

internal inline vec2f32 &
operator-=(vec2f32 &Left, vec2f32 Right)
{
    
    return (Left = Left - Right);
}

internal inline vec2f32 &
operator*=(vec2f32 &Left, f32 Right)
{
    
    return (Left = Left * Right);
}

internal inline vec2f32 &
operator/=(vec2f32 &Left, vec2f32 Right)
{
    
    return (Left = Left / Right);
}

internal inline vec2f32 &
operator/=(vec2f32 &Left, f32 Right)
{
    
    return (Left = Left / Right);
}

internal inline vec2f32 &
operator+=(vec2f32 &Left, vec2f32 Right)
{
    return (Left = Left + Right);
}

internal inline bool 
operator==(vec2f32 Left, vec2f32 Right)
{
    
    return EqualsVec2(Left, Right);
}

internal inline bool 
operator!=(vec2f32 Left, vec2f32 Right)
{
    
    return !EqualsVec2(Left, Right);
}

internal inline vec2f32 
operator-(vec2f32 In)
{
    
    vec2f32 Result;
    Result.X = -In.X;
    Result.Y = -In.Y;
    return(Result);
}

#endif // LANG_CPP
//~
// Vec2i32 stuff
//

typedef union vec2i32 v2i32;
union vec2i32
{
    struct
    {
        i32 X, Y;
    };
    struct
    {
        i32 x, y;
    };
    
    struct
    {
        i32 U, V;
    };
    
    struct
    {
        i32 u, v;
    };
    
    struct
    {
        i32 Left, Right;
    };
    
    struct
    {
        i32 left, right;
    };
    
    struct
    {
        i32 Width, Height;
    };
    
    struct
    {
        i32 width, height;
    };
    
    i32 Elements[2];
    
#ifdef LANG_CPP
    inline i32 &operator[](const i32 &Index)
    {
        return Elements[Index];
    }
#endif
};


internal inline vec2i32 
Vec2i32(i32 x, i32 y)
{
    vec2i32 Result;
    
    Result.x = x;
    Result.y = y;
    
    return (Result);
}

//~
// Vec3f32 stuff
//

typedef union vec3f32 v3;
union vec3f32
{
    struct
    {
        f32 X, Y, Z;
    };
    
    struct
    {
        f32 U, V, W;
    };
    
    struct
    {
        f32 R, G, B;
    };
    
    struct
    {
        f32 x, y, z;
    };
    
    struct
    {
        f32 u, v, w;
    };
    
    struct
    {
        f32 r, g, b;
    };
    
    struct
    {
        union
        {
            vec2f32 XY;
            vec2f32 xy;
        };
        f32 Ignored0_;
    };
    
    struct
    {
        f32 Ignored1_;
        union
        {
            vec2f32 yz;
            vec2f32 YZ;
        };
    };
    
    struct
    {
        union
        {
            vec2f32 UV;
            vec2f32 uv;
        };
        f32 Ignored2_;
    };
    
    struct
    {
        f32 Ignored3_;
        vec2f32 VW;
    };
    
    f32 Elements[3];
    
    f32 elements[3];
    
#ifdef LANG_CPP
    inline f32 &operator[](const int &Index)
    {
        return Elements[Index];
    }
#endif
};


internal inline vec3f32 
Vec3f32(f32 X, f32 Y, f32 Z)
{
    vec3f32 Result;
    
    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;
    
    return (Result);
}

internal inline vec3f32 
AddVec3(vec3f32 Left, vec3f32 Right)
{
    vec3f32 Result;
    
    Result.X = Left.X + Right.X;
    Result.Y = Left.Y + Right.Y;
    Result.Z = Left.Z + Right.Z;
    
    return (Result);
}

internal inline vec3f32 
SubtractVec3(vec3f32 Left, vec3f32 Right)
{
    vec3f32 Result;
    
    Result.X = Left.X - Right.X;
    Result.Y = Left.Y - Right.Y;
    Result.Z = Left.Z - Right.Z;
    
    return (Result);
}

internal inline vec3f32 
HadamardMultiplyVec3(vec3f32 Left, vec3f32 Right)
{
    vec3f32 Result;
    
    Result.X = Left.X * Right.X;
    Result.Y = Left.Y * Right.Y;
    Result.Z = Left.Z * Right.Z;
    
    return (Result);
}

internal inline vec3f32 
MultiplyVec3f(vec3f32 Left, f32 Right)
{
    vec3f32 Result;
    
    Result.X = Left.X * Right;
    Result.Y = Left.Y * Right;
    Result.Z = Left.Z * Right;
    
    return (Result);
}

internal inline vec3f32 
DivideVec3(vec3f32 Left, vec3f32 Right)
{
    vec3f32 Result;
    
    Result.X = Left.X / Right.X;
    Result.Y = Left.Y / Right.Y;
    Result.Z = Left.Z / Right.Z;
    
    return (Result);
}

internal inline vec3f32 
DivideVec3f(vec3f32 Left, f32 Right)
{
    vec3f32 Result;
    
    Result.X = Left.X / Right;
    Result.Y = Left.Y / Right;
    Result.Z = Left.Z / Right;
    
    return (Result);
}

internal inline bool 
EqualsVec3(vec3f32 Left, vec3f32 Right)
{
    bool Result = (Left.X == Right.X && Left.Y == Right.Y && Left.Z == Right.Z);
    
    return (Result);
}

internal inline f32 
DotVec3(vec3f32 VecOne, vec3f32 VecTwo)
{
    f32 Result = (VecOne.X * VecTwo.X) + (VecOne.Y * VecTwo.Y) + (VecOne.Z * VecTwo.Z);
    
    return (Result);
}

internal inline vec3f32 
Cross(vec3f32 VecOne, vec3f32 VecTwo)
{
    vec3f32 Result;
    
    Result.X = (VecOne.Y * VecTwo.Z) - (VecOne.Z * VecTwo.Y);
    Result.Y = (VecOne.Z * VecTwo.X) - (VecOne.X * VecTwo.Z);
    Result.Z = (VecOne.X * VecTwo.Y) - (VecOne.Y * VecTwo.X);
    
    return (Result);
}



internal inline f32
LengthSquaredVec3(vec3f32 A)
{
    float Result = DotVec3(A, A);
    return (Result);
}

internal inline f32 
LengthVec3(vec3f32 A)
{
    float Result = SquareRootF(LengthSquaredVec3(A));
    return (Result);
}

internal inline vec3f32 
NormalizeVec3(vec3f32 A)
{
    vec3f32 Result = {0};
    
    f32 VectorLength = LengthVec3(A);
    
    /* NOTE(kiljacken): We need a zero check to not divide-by-zero */
    if (VectorLength != 0.0f)
    {
        
        Result.X = A.X * (1.0f / VectorLength);
        Result.Y = A.Y * (1.0f / VectorLength);
        Result.Z = A.Z * (1.0f / VectorLength);
    }
    
    return (Result);
}

internal inline vec3f32 
FastNormalizeVec3(vec3f32 A)
{
    return MultiplyVec3f(A, RSquareRootF(DotVec3(A, A)));
}

#ifdef LANG_CPP

internal inline vec3f32
Vec3(f32 x, f32 y, f32 z)
{
    vec3f32 result = Vec3f32(x, y, z);
    return result;
}

internal inline vec3f32
Vec3(vec2f32 xy, f32 z)
{
    vec3f32 result = Vec3f32(xy.x, xy.y, z);
    return result;
}

internal inline f32 
Length(vec3f32 A)
{
    f32 Result = LengthVec3(A);
    
    return (Result);
}

internal inline f32 
LengthSquared(vec3f32 A)
{
    f32 Result = LengthSquaredVec3(A);
    
    return (Result);
}

internal inline vec3f32 
Normalize(vec3f32 A)
{
    vec3f32 Result = NormalizeVec3(A);
    
    return (Result);
}

internal inline vec3f32 
FastNormalize(vec3f32 A)
{
    vec3f32 Result = FastNormalizeVec3(A);
    
    return (Result);
}

internal inline f32 
Dot(vec3f32 VecOne, vec3f32 VecTwo)
{
    f32 Result = DotVec3(VecOne, VecTwo);
    
    return (Result);
}

internal inline vec3f32 
Add(vec3f32 Left, vec3f32 Right)
{
    vec3f32 Result = AddVec3(Left, Right);
    
    return (Result);
}

internal inline vec3f32 
Subtract(vec3f32 Left, vec3f32 Right)
{
    vec3f32 Result = SubtractVec3(Left, Right);
    
    return (Result);
}

internal inline vec3f32 
Multiply(vec3f32 Left, f32 Right)
{
    vec3f32 Result = MultiplyVec3f(Left, Right);
    
    return (Result);
}

internal inline vec3f32 
Divide(vec3f32 Left, vec3f32 Right)
{
    vec3f32 Result = DivideVec3(Left, Right);
    
    return (Result);
}

internal inline vec3f32 
Divide(vec3f32 Left, f32 Right)
{
    vec3f32 Result = DivideVec3f(Left, Right);
    
    return (Result);
}

internal inline bool 
Equals(vec3f32 Left, vec3f32 Right)
{
    bool Result = EqualsVec3(Left, Right);
    
    return (Result);
}

internal inline vec3f32 
operator+(vec3f32 Left, vec3f32 Right)
{
    
    vec3f32 Result = AddVec3(Left, Right);
    
    return (Result);
}

internal inline vec3f32 
operator-(vec3f32 Left, vec3f32 Right)
{
    
    vec3f32 Result = SubtractVec3(Left, Right);
    
    return (Result);
}

internal inline vec3f32 
operator*(vec3f32 Left, f32 Right)
{
    
    vec3f32 Result = MultiplyVec3f(Left, Right);
    
    return (Result);
}

internal inline vec3f32 
operator*(f32 Left, vec3f32 Right)
{
    
    vec3f32 Result = MultiplyVec3f(Right, Left);
    
    return (Result);
}

internal inline vec3f32 
operator/(vec3f32 Left, vec3f32 Right)
{
    
    vec3f32 Result = DivideVec3(Left, Right);
    
    return (Result);
}

internal inline vec3f32 
operator/(vec3f32 Left, f32 Right)
{
    
    vec3f32 Result = DivideVec3f(Left, Right);
    
    return (Result);
}

internal inline vec3f32 &
operator+=(vec3f32 &Left, vec3f32 Right)
{
    
    return (Left = Left + Right);
}

internal inline vec3f32 &
operator-=(vec3f32 &Left, vec3f32 Right)
{
    
    return (Left = Left - Right);
}

internal inline vec3f32 &
operator*=(vec3f32 &Left, f32 Right)
{
    
    return (Left = Left * Right);
}

internal inline vec3f32 &
operator/=(vec3f32 &Left, vec3f32 Right)
{
    
    return (Left = Left / Right);
}

internal inline vec3f32 &
operator/=(vec3f32 &Left, f32 Right)
{
    
    return (Left = Left / Right);
}

internal inline bool 
operator==(vec3f32 Left, vec3f32 Right)
{
    
    return EqualsVec3(Left, Right);
}

internal inline bool 
operator!=(vec3f32 Left, vec3f32 Right)
{
    
    return !EqualsVec3(Left, Right);
}

internal inline vec3f32 
operator-(vec3f32 In)
{
    
    vec3f32 Result;
    Result.X = -In.X;
    Result.Y = -In.Y;
    Result.Z = -In.Z;
    return(Result);
}


#endif // LANG_CPP

//~
// Vec4f32 stuff
//

typedef union vec4f32 v4;
union vec4f32
{
    struct
    {
        union
        {
            vec3f32 XYZ;
            vec3f32 xyz;
            struct
            {
                f32 X, Y, Z;
            };
        };
        
        f32 W;
    };
    struct
    {
        union
        {
            vec3f32 RGB;
            struct
            {
                f32 R, G, B;
            };
        };
        
        f32 A;
    };
    
    struct
    {
        vec2f32 XY;
        f32 Ignored0_;
        f32 Ignored1_;
    };
    
    struct
    {
        f32 Ignored2_;
        vec2f32 YZ;
        f32 Ignored3_;
    };
    
    struct
    {
        f32 Ignored4_;
        f32 Ignored5_;
        vec2f32 ZW;
    };
    
    f32 Elements[4];
    
#ifdef HANDMADE_MATH__USE_SSE
    __m128 InternalElementsSSE;
#endif
    
#ifdef LANG_CPP
    inline f32 &operator[](const int &Index)
    {
        return Elements[Index];
    }
#endif
};


internal inline vec4f32 
Vec4(f32 X, f32 Y, f32 Z, f32 W)
{
    vec4f32 Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    Result.InternalElementsSSE = _mm_setr_ps(X, Y, Z, W);
#else
    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;
    Result.W = W;
#endif
    
    return (Result);
}

internal inline vec4f32 
Vec4i(int X, int Y, int Z, int W)
{
    vec4f32 Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    Result.InternalElementsSSE = _mm_setr_ps((f32)X, (f32)Y, (f32)Z, (f32)W);
#else
    Result.X = (f32)X;
    Result.Y = (f32)Y;
    Result.Z = (f32)Z;
    Result.W = (f32)W;
#endif
    
    return (Result);
}

internal inline vec4f32 
Vec4v(vec3f32 Vector, f32 W)
{
    vec4f32 Result;
    
    Result = Vec4(Vector.X, Vector.Y, Vector.Z, W);
    
    return (Result);
}

internal inline vec4f32 
AddVec4(vec4f32 Left, vec4f32 Right)
{
    vec4f32 Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    Result.InternalElementsSSE = _mm_add_ps(Left.InternalElementsSSE, Right.InternalElementsSSE);
#else
    Result.X = Left.X + Right.X;
    Result.Y = Left.Y + Right.Y;
    Result.Z = Left.Z + Right.Z;
    Result.W = Left.W + Right.W;
#endif
    
    return (Result);
}

internal inline vec4f32 
SubtractVec4(vec4f32 Left, vec4f32 Right)
{
    vec4f32 Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    Result.InternalElementsSSE = _mm_sub_ps(Left.InternalElementsSSE, Right.InternalElementsSSE);
#else
    Result.X = Left.X - Right.X;
    Result.Y = Left.Y - Right.Y;
    Result.Z = Left.Z - Right.Z;
    Result.W = Left.W - Right.W;
#endif
    
    return (Result);
}

internal inline vec4f32 
HadamardMultiplyVec4(vec4f32 Left, vec4f32 Right)
{
    vec4f32 Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    Result.InternalElementsSSE = _mm_mul_ps(Left.InternalElementsSSE, Right.InternalElementsSSE);
#else
    Result.X = Left.X * Right.X;
    Result.Y = Left.Y * Right.Y;
    Result.Z = Left.Z * Right.Z;
    Result.W = Left.W * Right.W;
#endif
    
    return (Result);
}

internal inline vec4f32 
MultiplyVec4f(vec4f32 Left, f32 Right)
{
    vec4f32 Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    __m128 Scalar = _mm_set1_ps(Right);
    Result.InternalElementsSSE = _mm_mul_ps(Left.InternalElementsSSE, Scalar);
#else
    Result.X = Left.X * Right;
    Result.Y = Left.Y * Right;
    Result.Z = Left.Z * Right;
    Result.W = Left.W * Right;
#endif
    
    return (Result);
}

internal inline vec4f32 
DivideVec4(vec4f32 Left, vec4f32 Right)
{
    vec4f32 Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    Result.InternalElementsSSE = _mm_div_ps(Left.InternalElementsSSE, Right.InternalElementsSSE);
#else
    Result.X = Left.X / Right.X;
    Result.Y = Left.Y / Right.Y;
    Result.Z = Left.Z / Right.Z;
    Result.W = Left.W / Right.W;
#endif
    
    return (Result);
}

internal inline vec4f32 
DivideVec4f(vec4f32 Left, f32 Right)
{
    vec4f32 Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    __m128 Scalar = _mm_set1_ps(Right);
    Result.InternalElementsSSE = _mm_div_ps(Left.InternalElementsSSE, Scalar);
#else
    Result.X = Left.X / Right;
    Result.Y = Left.Y / Right;
    Result.Z = Left.Z / Right;
    Result.W = Left.W / Right;
#endif
    
    return (Result);
}

internal inline bool 
EqualsVec4(vec4f32 Left, vec4f32 Right)
{
    bool Result = (Left.X == Right.X && Left.Y == Right.Y && Left.Z == Right.Z && Left.W == Right.W);
    
    return (Result);
}

internal inline f32 
DotVec4(vec4f32 VecOne, vec4f32 VecTwo)
{
    f32 Result;
    
    // NOTE(zak): IN the future if we wanna check what version SSE is support
    // we can use _mm_dp_ps (4.3) but for now we will use the old way.
    // Or a r = _mm_mul_ps(v1, v2), r = _mm_hadd_ps(r, r), r = _mm_hadd_ps(r, r) for SSE3
#ifdef HANDMADE_MATH__USE_SSE
    __m128 SSEResultOne = _mm_mul_ps(VecOne.InternalElementsSSE, VecTwo.InternalElementsSSE);
    __m128 SSEResultTwo = _mm_shuffle_ps(SSEResultOne, SSEResultOne, _MM_SHUFFLE(2, 3, 0, 1));
    SSEResultOne = _mm_add_ps(SSEResultOne, SSEResultTwo);
    SSEResultTwo = _mm_shuffle_ps(SSEResultOne, SSEResultOne, _MM_SHUFFLE(0, 1, 2, 3));
    SSEResultOne = _mm_add_ps(SSEResultOne, SSEResultTwo);
    _mm_store_ss(&Result, SSEResultOne);
#else
    Result = (VecOne.X * VecTwo.X) + (VecOne.Y * VecTwo.Y) + (VecOne.Z * VecTwo.Z) + (VecOne.W * VecTwo.W);
#endif
    
    return (Result);
}

internal inline f32 
LengthSquaredVec4(vec4f32 A)
{
    f32 Result = DotVec4(A, A);
    
    return (Result);
}

internal inline f32 
LengthVec4(vec4f32 A)
{
    f32 Result = SquareRootF(LengthSquaredVec4(A));
    
    return(Result);
}

internal inline vec4f32 
NormalizeVec4(vec4f32 A)
{
    vec4f32 Result = {0};
    
    f32 VectorLength = LengthVec4(A);
    
    /* NOTE(kiljacken): We need a zero check to not divide-by-zero */
    if (VectorLength != 0.0f)
    {
        
        f32 Multiplier = 1.0f / VectorLength;
        
#ifdef HANDMADE_MATH__USE_SSE
        __m128 SSEMultiplier = _mm_set1_ps(Multiplier);
        Result.InternalElementsSSE = _mm_mul_ps(A.InternalElementsSSE, SSEMultiplier);
#else
        Result.X = A.X * Multiplier;
        Result.Y = A.Y * Multiplier;
        Result.Z = A.Z * Multiplier;
        Result.W = A.W * Multiplier;
#endif
    }
    
    return (Result);
}

internal inline vec4f32 
FastNormalizeVec4(vec4f32 A)
{
    return MultiplyVec4f(A, RSquareRootF(DotVec4(A, A)));
}

#if LANG_CPP

internal inline f32 
Length(vec4f32 A)
{
    f32 Result = LengthVec4(A);
    
    return (Result);
}

internal inline f32 
LengthSquared(vec4f32 A)
{
    f32 Result = LengthSquaredVec4(A);
    
    return (Result);
}

internal inline vec4f32 
Normalize(vec4f32 A)
{
    vec4f32 Result = NormalizeVec4(A);
    
    return (Result);
}

internal inline vec4f32 
FastNormalize(vec4f32 A)
{
    vec4f32 Result = FastNormalizeVec4(A);
    
    return (Result);
}

internal inline f32 
Dot(vec4f32 VecOne, vec4f32 VecTwo)
{
    f32 Result = DotVec4(VecOne, VecTwo);
    
    return (Result);
}

internal inline vec4f32 
Add(vec4f32 Left, vec4f32 Right)
{
    vec4f32 Result = AddVec4(Left, Right);
    
    return (Result);
}

internal inline vec4f32 
Subtract(vec4f32 Left, vec4f32 Right)
{
    vec4f32 Result = SubtractVec4(Left, Right);
    
    return (Result);
}

internal inline vec4f32 
Multiply(vec4f32 Left, f32 Right)
{
    vec4f32 Result = MultiplyVec4f(Left, Right);
    
    return (Result);
}

internal inline vec4f32 
Divide(vec4f32 Left, vec4f32 Right)
{
    vec4f32 Result = DivideVec4(Left, Right);
    
    return (Result);
}

internal inline vec4f32 
Divide(vec4f32 Left, f32 Right)
{
    vec4f32 Result = DivideVec4f(Left, Right);
    
    return (Result);
}

internal inline bool 
Equals(vec4f32 Left, vec4f32 Right)
{
    bool Result = EqualsVec4(Left, Right);
    
    return (Result);
}

internal inline vec4f32 
operator+(vec4f32 Left, vec4f32 Right)
{
    
    vec4f32 Result = AddVec4(Left, Right);
    
    return (Result);
}

internal inline vec4f32 
operator-(vec4f32 Left, vec4f32 Right)
{
    
    vec4f32 Result = SubtractVec4(Left, Right);
    
    return (Result);
}

internal inline vec4f32 
operator-(vec4f32 In)
{
    
    vec4f32 Result;
#if HANDMADE_MATH__USE_SSE
    Result.InternalElementsSSE = _mm_xor_ps(In.InternalElementsSSE, _mm_set1_ps(-0.0f));
#else
    Result.X = -In.X;
    Result.Y = -In.Y;
    Result.Z = -In.Z;
    Result.W = -In.W;
#endif
    return(Result);
}

internal inline vec4f32 
operator*(vec4f32 Left, f32 Right)
{
    
    vec4f32 Result = MultiplyVec4f(Left, Right);
    
    return (Result);
}

internal inline vec4f32 
operator*(f32 Left, vec4f32 Right)
{
    
    vec4f32 Result = MultiplyVec4f(Right, Left);
    
    return (Result);
}

internal inline vec4f32 
operator/(vec4f32 Left, f32 Right)
{
    
    vec4f32 Result = DivideVec4f(Left, Right);
    
    return (Result);
}

internal inline vec4f32 &
operator+=(vec4f32 &Left, vec4f32 Right)
{
    
    return (Left = Left + Right);
}

internal inline vec4f32 &
operator*=(vec4f32 &Left, f32 Right)
{
    
    return (Left = Left * Right);
}

internal inline vec4f32 &
operator/=(vec4f32 &Left, f32 Right)
{
    
    return (Left = Left / Right);
}

internal inline bool 
operator==(vec4f32 Left, vec4f32 Right)
{
    
    return EqualsVec4(Left, Right);
}

internal inline bool 
operator!=(vec4f32 Left, vec4f32 Right)
{
    
    return !EqualsVec4(Left, Right);
}

internal inline vec4f32 &
operator-=(vec4f32 &Left, vec4f32 Right)
{
    
    return (Left = Left - Right);
}

#endif // LANG_CPP
//~
// Mat4f32 stuff
//

typedef union mat4f32 m4;
union mat4f32
{
    f32 Elements[4][4];
    f32 elements[4][4];
    
#ifdef HANDMADE_MATH__USE_SSE
    __m128 Columns[4];
#endif
    
#ifdef LANG_CPP
    inline vec4f32 operator[](const int &Index)
    {
        vec4f32 Result;
        f32* Column = Elements[Index];
        
        
        Result.Elements[0] = Column[0];
        Result.Elements[1] = Column[1];
        Result.Elements[2] = Column[2];
        Result.Elements[3] = Column[3];
        
        return Result;
    }
#endif
};


internal inline mat4f32 
Mat4(void)
{
    mat4f32 Result = {0};
    
    return (Result);
}

internal inline mat4f32 
Mat4d(f32 Diagonal)
{
    mat4f32 Result = Mat4();
    
    Result.Elements[0][0] = Diagonal;
    Result.Elements[1][1] = Diagonal;
    Result.Elements[2][2] = Diagonal;
    Result.Elements[3][3] = Diagonal;
    
    return (Result);
}

internal inline mat4f32 
Transpose(mat4f32 Matrix)
{
    mat4f32 Result = Matrix;
    
#ifdef HANDMADE_MATH__USE_SSE
    _MM_TRANSPOSE4_PS(Result.Columns[0], Result.Columns[1], Result.Columns[2], Result.Columns[3]);
#else
    int Columns;
    for(Columns = 0; Columns < 4; ++Columns)
    {
        int Rows;
        for(Rows = 0; Rows < 4; ++Rows)
        {
            Result.Elements[Rows][Columns] = Matrix.Elements[Columns][Rows];
        }
    }
#endif
    
    
    return (Result);
}

internal inline mat4f32 
AddMat4(mat4f32 Left, mat4f32 Right)
{
    mat4f32 Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    Result.Columns[0] = _mm_add_ps(Left.Columns[0], Right.Columns[0]);
    Result.Columns[1] = _mm_add_ps(Left.Columns[1], Right.Columns[1]);
    Result.Columns[2] = _mm_add_ps(Left.Columns[2], Right.Columns[2]);
    Result.Columns[3] = _mm_add_ps(Left.Columns[3], Right.Columns[3]);
#else
    int Columns;
    for(Columns = 0; Columns < 4; ++Columns)
    {
        int Rows;
        for(Rows = 0; Rows < 4; ++Rows)
        {
            Result.Elements[Columns][Rows] = Left.Elements[Columns][Rows] + Right.Elements[Columns][Rows];
        }
    }
#endif
    
    
    return (Result);
}

internal inline mat4f32 
SubtractMat4(mat4f32 Left, mat4f32 Right)
{
    mat4f32 Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    Result.Columns[0] = _mm_sub_ps(Left.Columns[0], Right.Columns[0]);
    Result.Columns[1] = _mm_sub_ps(Left.Columns[1], Right.Columns[1]);
    Result.Columns[2] = _mm_sub_ps(Left.Columns[2], Right.Columns[2]);
    Result.Columns[3] = _mm_sub_ps(Left.Columns[3], Right.Columns[3]);
#else
    int Columns;
    for(Columns = 0; Columns < 4; ++Columns)
    {
        int Rows;
        for(Rows = 0; Rows < 4; ++Rows)
        {
            Result.Elements[Columns][Rows] = Left.Elements[Columns][Rows] - Right.Elements[Columns][Rows];
        }
    }
#endif
    
    return (Result);
}

#ifdef HANDMADE_MATH__USE_SSE
internal inline __m128 
LinearCombineSSE(__m128 Left, mat4f32 Right)
{
    __m128 Result;
    Result = _mm_mul_ps(_mm_shuffle_ps(Left, Left, 0x00), Right.Columns[0]);
    Result = _mm_add_ps(Result, _mm_mul_ps(_mm_shuffle_ps(Left, Left, 0x55), Right.Columns[1]));
    Result = _mm_add_ps(Result, _mm_mul_ps(_mm_shuffle_ps(Left, Left, 0xaa), Right.Columns[2]));
    Result = _mm_add_ps(Result, _mm_mul_ps(_mm_shuffle_ps(Left, Left, 0xff), Right.Columns[3]));
    
    return (Result);
}
#endif

internal inline mat4f32 
MultiplyMat4(mat4f32 Left, mat4f32 Right)
{
    mat4f32 Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    Result.Columns[0] = LinearCombineSSE(Right.Columns[0], Left);
    Result.Columns[1] = LinearCombineSSE(Right.Columns[1], Left);
    Result.Columns[2] = LinearCombineSSE(Right.Columns[2], Left);
    Result.Columns[3] = LinearCombineSSE(Right.Columns[3], Left);
#else
    int Columns;
    for(Columns = 0; Columns < 4; ++Columns)
    {
        int Rows;
        for(Rows = 0; Rows < 4; ++Rows)
        {
            f32 Sum = 0;
            int CurrentMatrice;
            for(CurrentMatrice = 0; CurrentMatrice < 4; ++CurrentMatrice)
            {
                Sum += Left.Elements[CurrentMatrice][Rows] * Right.Elements[Columns][CurrentMatrice];
            }
            
            Result.Elements[Columns][Rows] = Sum;
        }
    }
#endif
    
    return (Result);
}


internal inline mat4f32 
MultiplyMat4f(mat4f32 Matrix, f32 Scalar)
{
    mat4f32 Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    __m128 SSEScalar = _mm_set1_ps(Scalar);
    Result.Columns[0] = _mm_mul_ps(Matrix.Columns[0], SSEScalar);
    Result.Columns[1] = _mm_mul_ps(Matrix.Columns[1], SSEScalar);
    Result.Columns[2] = _mm_mul_ps(Matrix.Columns[2], SSEScalar);
    Result.Columns[3] = _mm_mul_ps(Matrix.Columns[3], SSEScalar);
#else
    int Columns;
    for(Columns = 0; Columns < 4; ++Columns)
    {
        int Rows;
        for(Rows = 0; Rows < 4; ++Rows)
        {
            Result.Elements[Columns][Rows] = Matrix.Elements[Columns][Rows] * Scalar;
        }
    }
#endif
    
    return (Result);
}

internal inline vec4f32 
MultiplyMat4ByVec4(mat4f32 Matrix, vec4f32 Vector)
{
    vec4f32 Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    Result.InternalElementsSSE = LinearCombineSSE(Vector.InternalElementsSSE, Matrix);
#else
    int Columns, Rows;
    for(Rows = 0; Rows < 4; ++Rows)
    {
        f32 Sum = 0;
        for(Columns = 0; Columns < 4; ++Columns)
        {
            Sum += Matrix.Elements[Columns][Rows] * Vector.Elements[Columns];
        }
        
        Result.Elements[Rows] = Sum;
    }
#endif
    
    return (Result);
}


internal inline mat4f32 
DivideMat4f(mat4f32 Matrix, f32 Scalar)
{
    mat4f32 Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    __m128 SSEScalar = _mm_set1_ps(Scalar);
    Result.Columns[0] = _mm_div_ps(Matrix.Columns[0], SSEScalar);
    Result.Columns[1] = _mm_div_ps(Matrix.Columns[1], SSEScalar);
    Result.Columns[2] = _mm_div_ps(Matrix.Columns[2], SSEScalar);
    Result.Columns[3] = _mm_div_ps(Matrix.Columns[3], SSEScalar);
#else
    int Columns;
    for(Columns = 0; Columns < 4; ++Columns)
    {
        int Rows;
        for(Rows = 0; Rows < 4; ++Rows)
        {
            Result.Elements[Columns][Rows] = Matrix.Elements[Columns][Rows] / Scalar;
        }
    }
#endif
    
    return (Result);
}

#if LANG_CPP

internal inline mat4f32 
Add(mat4f32 Left, mat4f32 Right)
{
    mat4f32 Result = AddMat4(Left, Right);
    
    return (Result);
}

internal inline mat4f32 
Subtract(mat4f32 Left, mat4f32 Right)
{
    mat4f32 Result = SubtractMat4(Left, Right);
    
    return (Result);
}

internal inline mat4f32 
Multiply(mat4f32 Left, mat4f32 Right)
{
    mat4f32 Result = MultiplyMat4(Left, Right);
    
    return (Result);
}

internal inline mat4f32 
Multiply(mat4f32 Left, f32 Right)
{
    mat4f32 Result = MultiplyMat4f(Left, Right);
    
    return (Result);
}

internal inline vec4f32 
Multiply(mat4f32 Matrix, vec4f32 Vector)
{
    vec4f32 Result = MultiplyMat4ByVec4(Matrix, Vector);
    
    return (Result);
}

internal inline mat4f32 
Divide(mat4f32 Left, f32 Right)
{
    mat4f32 Result = DivideMat4f(Left, Right);
    
    return (Result);
}

internal inline mat4f32 
operator+(mat4f32 Left, mat4f32 Right)
{
    
    mat4f32 Result = AddMat4(Left, Right);
    
    return (Result);
}

internal inline mat4f32 
operator-(mat4f32 Left, mat4f32 Right)
{
    
    mat4f32 Result = SubtractMat4(Left, Right);
    
    return (Result);
}

internal inline mat4f32 
operator*(mat4f32 Left, mat4f32 Right)
{
    
    mat4f32 Result = MultiplyMat4(Left, Right);
    
    return (Result);
}

internal inline mat4f32 
operator*(mat4f32 Left, f32 Right)
{
    
    mat4f32 Result = MultiplyMat4f(Left, Right);
    
    return (Result);
}

internal inline mat4f32 
operator*(f32 Left, mat4f32 Right)
{
    
    mat4f32 Result = MultiplyMat4f(Right, Left);
    
    return (Result);
}

internal inline vec4f32 
operator*(mat4f32 Matrix, vec4f32 Vector)
{
    
    vec4f32 Result = MultiplyMat4ByVec4(Matrix, Vector);
    
    return (Result);
}

internal inline mat4f32 
operator/(mat4f32 Left, f32 Right)
{
    
    mat4f32 Result = DivideMat4f(Left, Right);
    
    return (Result);
}

internal inline mat4f32 &
operator+=(mat4f32 &Left, mat4f32 Right)
{
    
    return (Left = Left + Right);
}

internal inline mat4f32 &
operator-=(mat4f32 &Left, mat4f32 Right)
{
    
    return (Left = Left - Right);
}

internal inline mat4f32 &
operator*=(mat4f32 &Left, f32 Right)
{
    
    return (Left = Left * Right);
}

internal inline mat4f32 &
operator/=(mat4f32 &Left, f32 Right)
{
    
    return (Left = Left / Right);
}

#endif // LANG_CPP

//~
// Quaternion stuff
//

union quaternion
{
    struct
    {
        union
        {
            vec3f32 XYZ;
            struct
            {
                f32 X, Y, Z;
            };
        };
        
        f32 W;
    };
    
    f32 Elements[4];
    
#ifdef HANDMADE_MATH__USE_SSE
    __m128 InternalElementsSSE;
#endif
};

internal inline quaternion 
Quaternion(f32 X, f32 Y, f32 Z, f32 W)
{
    quaternion Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    Result.InternalElementsSSE = _mm_setr_ps(X, Y, Z, W);
#else
    Result.X = X;
    Result.Y = Y;
    Result.Z = Z;
    Result.W = W;
#endif
    
    return (Result);
}

internal inline quaternion 
QuaternionV4(vec4f32 Vector)
{
    quaternion Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    Result.InternalElementsSSE = Vector.InternalElementsSSE;
#else
    Result.X = Vector.X;
    Result.Y = Vector.Y;
    Result.Z = Vector.Z;
    Result.W = Vector.W;
#endif
    
    return (Result);
}

internal inline quaternion 
AddQuaternion(quaternion Left, quaternion Right)
{
    quaternion Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    Result.InternalElementsSSE = _mm_add_ps(Left.InternalElementsSSE, Right.InternalElementsSSE);
#else
    
    Result.X = Left.X + Right.X;
    Result.Y = Left.Y + Right.Y;
    Result.Z = Left.Z + Right.Z;
    Result.W = Left.W + Right.W;
#endif
    
    return (Result);
}

internal inline quaternion 
SubtractQuaternion(quaternion Left, quaternion Right)
{
    quaternion Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    Result.InternalElementsSSE = _mm_sub_ps(Left.InternalElementsSSE, Right.InternalElementsSSE);
#else
    
    Result.X = Left.X - Right.X;
    Result.Y = Left.Y - Right.Y;
    Result.Z = Left.Z - Right.Z;
    Result.W = Left.W - Right.W;
#endif
    
    return (Result);
}

internal inline quaternion 
MultiplyQuaternion(quaternion Left, quaternion Right)
{
    quaternion Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    __m128 SSEResultOne = _mm_xor_ps(_mm_shuffle_ps(Left.InternalElementsSSE, Left.InternalElementsSSE, _MM_SHUFFLE(0, 0, 0, 0)), _mm_setr_ps(0.f, -0.f, 0.f, -0.f));
    __m128 SSEResultTwo = _mm_shuffle_ps(Right.InternalElementsSSE, Right.InternalElementsSSE, _MM_SHUFFLE(0, 1, 2, 3));
    __m128 SSEResultThree = _mm_mul_ps(SSEResultTwo, SSEResultOne);
    
    SSEResultOne = _mm_xor_ps(_mm_shuffle_ps(Left.InternalElementsSSE, Left.InternalElementsSSE, _MM_SHUFFLE(1, 1, 1, 1)) , _mm_setr_ps(0.f, 0.f, -0.f, -0.f));
    SSEResultTwo = _mm_shuffle_ps(Right.InternalElementsSSE, Right.InternalElementsSSE, _MM_SHUFFLE(1, 0, 3, 2));
    SSEResultThree = _mm_add_ps(SSEResultThree, _mm_mul_ps(SSEResultTwo, SSEResultOne));
    
    SSEResultOne = _mm_xor_ps(_mm_shuffle_ps(Left.InternalElementsSSE, Left.InternalElementsSSE, _MM_SHUFFLE(2, 2, 2, 2)), _mm_setr_ps(-0.f, 0.f, 0.f, -0.f));
    SSEResultTwo = _mm_shuffle_ps(Right.InternalElementsSSE, Right.InternalElementsSSE, _MM_SHUFFLE(2, 3, 0, 1));
    SSEResultThree = _mm_add_ps(SSEResultThree, _mm_mul_ps(SSEResultTwo, SSEResultOne));
    
    SSEResultOne = _mm_shuffle_ps(Left.InternalElementsSSE, Left.InternalElementsSSE, _MM_SHUFFLE(3, 3, 3, 3));
    SSEResultTwo = _mm_shuffle_ps(Right.InternalElementsSSE, Right.InternalElementsSSE, _MM_SHUFFLE(3, 2, 1, 0));
    Result.InternalElementsSSE = _mm_add_ps(SSEResultThree, _mm_mul_ps(SSEResultTwo, SSEResultOne));
#else
    Result.X = (Left.X * Right.W) + (Left.Y * Right.Z) - (Left.Z * Right.Y) + (Left.W * Right.X);
    Result.Y = (-Left.X * Right.Z) + (Left.Y * Right.W) + (Left.Z * Right.X) + (Left.W * Right.Y);
    Result.Z = (Left.X * Right.Y) - (Left.Y * Right.X) + (Left.Z * Right.W) + (Left.W * Right.Z);
    Result.W = (-Left.X * Right.X) - (Left.Y * Right.Y) - (Left.Z * Right.Z) + (Left.W * Right.W);
#endif
    
    return (Result);
}

internal inline quaternion 
MultiplyQuaternionF(quaternion Left, f32 Multiplicative)
{
    quaternion Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    __m128 Scalar = _mm_set1_ps(Multiplicative);
    Result.InternalElementsSSE = _mm_mul_ps(Left.InternalElementsSSE, Scalar);
#else
    Result.X = Left.X * Multiplicative;
    Result.Y = Left.Y * Multiplicative;
    Result.Z = Left.Z * Multiplicative;
    Result.W = Left.W * Multiplicative;
#endif
    
    return (Result);
}

internal inline quaternion 
DivideQuaternionF(quaternion Left, f32 Dividend)
{
    quaternion Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    __m128 Scalar = _mm_set1_ps(Dividend);
    Result.InternalElementsSSE = _mm_div_ps(Left.InternalElementsSSE, Scalar);
#else
    Result.X = Left.X / Dividend;
    Result.Y = Left.Y / Dividend;
    Result.Z = Left.Z / Dividend;
    Result.W = Left.W / Dividend;
#endif
    
    return (Result);
}

internal inline f32 
DotQuaternion(quaternion Left, quaternion Right)
{
    f32 Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    __m128 SSEResultOne = _mm_mul_ps(Left.InternalElementsSSE, Right.InternalElementsSSE);
    __m128 SSEResultTwo = _mm_shuffle_ps(SSEResultOne, SSEResultOne, _MM_SHUFFLE(2, 3, 0, 1));
    SSEResultOne = _mm_add_ps(SSEResultOne, SSEResultTwo);
    SSEResultTwo = _mm_shuffle_ps(SSEResultOne, SSEResultOne, _MM_SHUFFLE(0, 1, 2, 3));
    SSEResultOne = _mm_add_ps(SSEResultOne, SSEResultTwo);
    _mm_store_ss(&Result, SSEResultOne);
#else
    Result = (Left.X * Right.X) + (Left.Y * Right.Y) + (Left.Z * Right.Z) + (Left.W * Right.W);
#endif
    
    return (Result);
}


internal inline quaternion 
InverseQuaternion(quaternion Left)
{
    quaternion Result;
    
    Result.X = -Left.X;
    Result.Y = -Left.Y;
    Result.Z = -Left.Z;
    Result.W = Left.W;
    
    Result = DivideQuaternionF(Result, (DotQuaternion(Left, Left)));
    
    return (Result);
}


internal inline quaternion 
NormalizeQuaternion(quaternion Left)
{
    quaternion Result;
    
    f32 Length = SquareRootF(DotQuaternion(Left, Left));
    Result = DivideQuaternionF(Left, Length);
    
    return (Result);
}

internal inline quaternion 
NLerp(quaternion Left, f32 Time, quaternion Right)
{
    quaternion Result;
    
#ifdef HANDMADE_MATH__USE_SSE
    __m128 ScalarLeft = _mm_set1_ps(1.0f - Time);
    __m128 ScalarRight = _mm_set1_ps(Time);
    __m128 SSEResultOne = _mm_mul_ps(Left.InternalElementsSSE, ScalarLeft);
    __m128 SSEResultTwo = _mm_mul_ps(Right.InternalElementsSSE, ScalarRight);
    Result.InternalElementsSSE = _mm_add_ps(SSEResultOne, SSEResultTwo);
#else
    Result.X = Lerp(Left.X, Time, Right.X);
    Result.Y = Lerp(Left.Y, Time, Right.Y);
    Result.Z = Lerp(Left.Z, Time, Right.Z);
    Result.W = Lerp(Left.W, Time, Right.W);
#endif
    Result = NormalizeQuaternion(Result);
    
    return (Result);
}

internal inline quaternion 
SLerp(quaternion Left, f32 Time, quaternion Right)
{
    quaternion Result;
    quaternion QuaternionLeft;
    quaternion QuaternionRight;
    
    f32 Cos_Theta = DotQuaternion(Left, Right);
    f32 Angle = ACosF(Cos_Theta);
    
    f32 S1 = SinF((1.0f - Time) * Angle);
    f32 S2 = SinF(Time * Angle);
    f32 Is = 1.0f / SinF(Angle);
    
    QuaternionLeft = MultiplyQuaternionF(Left, S1);
    QuaternionRight = MultiplyQuaternionF(Right, S2);
    
    Result = AddQuaternion(QuaternionLeft, QuaternionRight);
    Result = MultiplyQuaternionF(Result, Is);
    
    return (Result);
}

internal inline mat4f32 
QuaternionToMat4(quaternion Left)
{
    mat4f32 Result;
    
    quaternion NormalizedQuaternion = NormalizeQuaternion(Left);
    
    f32 XX, YY, ZZ,
    XY, XZ, YZ,
    WX, WY, WZ;
    
    XX = NormalizedQuaternion.X * NormalizedQuaternion.X;
    YY = NormalizedQuaternion.Y * NormalizedQuaternion.Y;
    ZZ = NormalizedQuaternion.Z * NormalizedQuaternion.Z;
    XY = NormalizedQuaternion.X * NormalizedQuaternion.Y;
    XZ = NormalizedQuaternion.X * NormalizedQuaternion.Z;
    YZ = NormalizedQuaternion.Y * NormalizedQuaternion.Z;
    WX = NormalizedQuaternion.W * NormalizedQuaternion.X;
    WY = NormalizedQuaternion.W * NormalizedQuaternion.Y;
    WZ = NormalizedQuaternion.W * NormalizedQuaternion.Z;
    
    Result.Elements[0][0] = 1.0f - 2.0f * (YY + ZZ);
    Result.Elements[0][1] = 2.0f * (XY + WZ);
    Result.Elements[0][2] = 2.0f * (XZ - WY);
    Result.Elements[0][3] = 0.0f;
    
    Result.Elements[1][0] = 2.0f * (XY - WZ);
    Result.Elements[1][1] = 1.0f - 2.0f * (XX + ZZ);
    Result.Elements[1][2] = 2.0f * (YZ + WX);
    Result.Elements[1][3] = 0.0f;
    
    Result.Elements[2][0] = 2.0f * (XZ + WY);
    Result.Elements[2][1] = 2.0f * (YZ - WX);
    Result.Elements[2][2] = 1.0f - 2.0f * (XX + YY);
    Result.Elements[2][3] = 0.0f;
    
    Result.Elements[3][0] = 0.0f;
    Result.Elements[3][1] = 0.0f;
    Result.Elements[3][2] = 0.0f;
    Result.Elements[3][3] = 1.0f;
    
    return (Result);
}

// This method taken from Mike Day at Insomniac Games.
// https://d3cw3dd2w32x2b.cloudfront.net/wp-content/uploads/2015/01/matrix-to-quat.pdf
//
// Note that as mentioned at the top of the paper, the paper assumes the matrix
// would be *post*-multiplied to a vector to rotate it, meaning the matrix is
// the transpose of what we're dealing with. But, because our matrices are
// stored in column-major order, the indices *appear* to match the paper.
//
// For example, m12 in the paper is row 1, column 2. We need to transpose it to
// row 2, column 1. But, because the column comes first when referencing
// elements, it looks like M.Elements[1][2].
//
// Don't be confused! Or if you must be confused, at least trust this
// comment. :)
internal inline quaternion 
Mat4ToQuaternion(mat4f32 M)
{
    float T;
    quaternion Q;
    
    if (M.Elements[2][2] < 0.0f) {
        if (M.Elements[0][0] > M.Elements[1][1]) {
            
            T = 1 + M.Elements[0][0] - M.Elements[1][1] - M.Elements[2][2];
            Q = Quaternion(
                           T,
                           M.Elements[0][1] + M.Elements[1][0],
                           M.Elements[2][0] + M.Elements[0][2],
                           M.Elements[1][2] - M.Elements[2][1]
                           );
        } else {
            
            T = 1 - M.Elements[0][0] + M.Elements[1][1] - M.Elements[2][2];
            Q = Quaternion(
                           M.Elements[0][1] + M.Elements[1][0],
                           T,
                           M.Elements[1][2] + M.Elements[2][1],
                           M.Elements[2][0] - M.Elements[0][2]
                           );
        }
    } else {
        if (M.Elements[0][0] < -M.Elements[1][1]) {
            
            T = 1 - M.Elements[0][0] - M.Elements[1][1] + M.Elements[2][2];
            Q = Quaternion(
                           M.Elements[2][0] + M.Elements[0][2],
                           M.Elements[1][2] + M.Elements[2][1],
                           T,
                           M.Elements[0][1] - M.Elements[1][0]
                           );
        } else {
            
            T = 1 + M.Elements[0][0] + M.Elements[1][1] + M.Elements[2][2];
            Q = Quaternion(
                           M.Elements[1][2] - M.Elements[2][1],
                           M.Elements[2][0] - M.Elements[0][2],
                           M.Elements[0][1] - M.Elements[1][0],
                           T
                           );
        }
    }
    
    Q = MultiplyQuaternionF(Q, 0.5f / SquareRootF(T));
    
    return Q;
}

internal inline quaternion 
QuaternionFromAxisAngle(vec3f32 Axis, f32 AngleOfRotation)
{
    quaternion Result;
    
    vec3f32 AxisNormalized = NormalizeVec3(Axis);
    f32 SineOfRotation = SinF(AngleOfRotation / 2.0f);
    
    Result.XYZ = MultiplyVec3f(AxisNormalized, SineOfRotation);
    Result.W = CosF(AngleOfRotation / 2.0f);
    
    return (Result);
}

#ifdef LANG_CPP

internal inline quaternion 
Multiply(quaternion Left, quaternion Right)
{
    quaternion Result = MultiplyQuaternion(Left, Right);
    
    return (Result);
}

internal inline quaternion 
Multiply(quaternion Left, f32 Right)
{
    quaternion Result = MultiplyQuaternionF(Left, Right);
    
    return (Result);
}

internal inline quaternion 
Normalize(quaternion A)
{
    quaternion Result = NormalizeQuaternion(A);
    
    return (Result);
}

internal inline f32 
Dot(quaternion QuatOne, quaternion QuatTwo)
{
    f32 Result = DotQuaternion(QuatOne, QuatTwo);
    
    return (Result);
}

internal inline quaternion 
Add(quaternion Left, quaternion Right)
{
    quaternion Result = AddQuaternion(Left, Right);
    
    return (Result);
}

internal inline quaternion 
Subtract(quaternion Left, quaternion Right)
{
    quaternion Result = SubtractQuaternion(Left, Right);
    
    return (Result);
}

internal inline quaternion 
Divide(quaternion Left, f32 Right)
{
    quaternion Result = DivideQuaternionF(Left, Right);
    
    return (Result);
}


internal inline quaternion 
operator+(quaternion Left, quaternion Right)
{
    
    quaternion Result = AddQuaternion(Left, Right);
    
    return (Result);
}

internal inline quaternion 
operator-(quaternion Left, quaternion Right)
{
    
    quaternion Result = SubtractQuaternion(Left, Right);
    
    return (Result);
}

internal inline quaternion 
operator*(quaternion Left, quaternion Right)
{
    
    quaternion Result = MultiplyQuaternion(Left, Right);
    
    return (Result);
}

internal inline quaternion 
operator*(quaternion Left, f32 Right)
{
    
    quaternion Result = MultiplyQuaternionF(Left, Right);
    
    return (Result);
}

internal inline quaternion 
operator*(f32 Left, quaternion Right)
{
    
    quaternion Result = MultiplyQuaternionF(Right, Left);
    
    return (Result);
}

internal inline quaternion 
operator/(quaternion Left, f32 Right)
{
    
    quaternion Result = DivideQuaternionF(Left, Right);
    
    return (Result);
}

internal inline quaternion &
operator+=(quaternion &Left, quaternion Right)
{
    
    return (Left = Left + Right);
}

internal inline quaternion &
operator-=(quaternion &Left, quaternion Right)
{
    
    return (Left = Left - Right);
}

internal inline quaternion &
operator*=(quaternion &Left, f32 Right)
{
    
    return (Left = Left * Right);
}

internal inline quaternion &
operator/=(quaternion &Left, f32 Right)
{
    
    return (Left = Left / Right);
}

#endif // LANG_CPP

//~
// Rectangle2D stuff
//


union Rectangle2D
{
    struct
    {
        v2 min_p;
        v2 max_p;
    };
    
    v4 compact_rect;
    
    struct
    {
        f32 x0; f32 y0;
        f32 x1; f32 y1;
    };
    
    f32 elements[4];
};

internal inline Rectangle2D
RectMinMax(v2 min_p, v2 max_p)
{
    Rectangle2D result;
    result.min_p = min_p;
    result.max_p = max_p;
    return result;
}

internal inline Rectangle2D
RectTwoPoints(v2 a, v2 b)
{
    Rectangle2D result;
    result.min_p = Vec2f32(Min(a.x, b.x), Min(a.y, b.y));
    result.max_p = Vec2f32(Max(a.x, b.x), Max(a.y, b.y));
    return result;
}

internal inline Rectangle2D
RectCentHalfDim(v2 center, v2 half_dim)
{
    Rectangle2D result = RectMinMax(center - half_dim, center + half_dim);
    return result;
}

internal inline Rectangle2D
RectCentDim(v2 center, v2 dim)
{
    Rectangle2D result = RectCentHalfDim(center, 0.5f * dim);
    return result;
}

internal inline b32
IsInsideRect(Rectangle2D rect, v2 pos)
{
    b32 result = ((pos.x >= rect.min_p.x && pos.x < rect.max_p.x) &&
                  (pos.y >= rect.min_p.y && pos.y < rect.max_p.y));
    return result;
}

internal inline v2
ClampInsideRect(Rectangle2D rect, v2 p)
{
    v2 result;
    result.x = Clamp(rect.min_p.x, p.x, rect.max_p.x);
    result.y = Clamp(rect.min_p.y, p.y, rect.max_p.y);
    return result;
}

internal inline v2
RectDim(Rectangle2D rect)
{
    v2 result = Vec2(AbsVal(rect.max_p.x - rect.min_p.x),
                     AbsVal(rect.max_p.y - rect.min_p.y));
    return result;
}


//~
// Common graphics transformations
//

internal inline mat4f32 
Translate(vec3f32 Translation)
{
    mat4f32 Result = Mat4d(1.0f);
    
    Result.Elements[3][0] = Translation.X;
    Result.Elements[3][1] = Translation.Y;
    Result.Elements[3][2] = Translation.Z;
    
    return (Result);
}

internal inline mat4f32 
Orthographic(f32 Left, f32 Right, f32 Bottom, f32 Top, f32 Near, f32 Far)
{
    mat4f32 Result = Mat4();
    
    Result.Elements[0][0] = 2.0f / (Right - Left);
    Result.Elements[1][1] = 2.0f / (Top - Bottom);
    Result.Elements[2][2] = 2.0f / (Near - Far);
    Result.Elements[3][3] = 1.0f;
    
    Result.Elements[3][0] = (Left + Right) / (Left - Right);
    Result.Elements[3][1] = (Bottom + Top) / (Bottom - Top);
    Result.Elements[3][2] = (Far + Near) / (Near - Far);
    
    return (Result);
}

internal inline mat4f32 
Perspective(f32 FOV, f32 AspectRatio, f32 Near, f32 Far)
{
    mat4f32 Result = Mat4();
    
    // See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml
    
    f32 Cotangent = 1.0f / TanF(FOV * (PI32 / 360.0f));
    
    Result.Elements[0][0] = Cotangent / AspectRatio;
    Result.Elements[1][1] = Cotangent;
    Result.Elements[2][3] = -1.0f;
    Result.Elements[2][2] = (Near + Far) / (Near - Far);
    Result.Elements[3][2] = (2.0f * Near * Far) / (Near - Far);
    Result.Elements[3][3] = 0.0f;
    
    return (Result);
}

internal inline mat4f32 
Rotate(f32 Angle, vec3f32 Axis)
{
    mat4f32 Result = Mat4d(1.0f);
    
    Axis = NormalizeVec3(Axis);
    
    f32 SinTheta = SinF(Angle);
    f32 CosTheta = CosF(Angle);
    f32 CosValue = 1.0f - CosTheta;
    
    Result.Elements[0][0] = (Axis.X * Axis.X * CosValue) + CosTheta;
    Result.Elements[0][1] = (Axis.X * Axis.Y * CosValue) + (Axis.Z * SinTheta);
    Result.Elements[0][2] = (Axis.X * Axis.Z * CosValue) - (Axis.Y * SinTheta);
    
    Result.Elements[1][0] = (Axis.Y * Axis.X * CosValue) - (Axis.Z * SinTheta);
    Result.Elements[1][1] = (Axis.Y * Axis.Y * CosValue) + CosTheta;
    Result.Elements[1][2] = (Axis.Y * Axis.Z * CosValue) + (Axis.X * SinTheta);
    
    Result.Elements[2][0] = (Axis.Z * Axis.X * CosValue) + (Axis.Y * SinTheta);
    Result.Elements[2][1] = (Axis.Z * Axis.Y * CosValue) - (Axis.X * SinTheta);
    Result.Elements[2][2] = (Axis.Z * Axis.Z * CosValue) + CosTheta;
    
    return (Result);
}



internal inline mat4f32 
Scale(vec3f32 Scale)
{
    mat4f32 Result = Mat4d(1.0f);
    
    Result.Elements[0][0] = Scale.X;
    Result.Elements[1][1] = Scale.Y;
    Result.Elements[2][2] = Scale.Z;
    
    return (Result);
}

internal inline mat4f32 
LookAt(vec3f32 Eye, vec3f32 Center, vec3f32 Up)
{
    mat4f32 Result;
    
    vec3f32 F = NormalizeVec3(SubtractVec3(Center, Eye));
    vec3f32 S = NormalizeVec3(Cross(F, Up));
    vec3f32 U = Cross(S, F);
    
    Result.Elements[0][0] = S.X;
    Result.Elements[0][1] = U.X;
    Result.Elements[0][2] = -F.X;
    Result.Elements[0][3] = 0.0f;
    
    Result.Elements[1][0] = S.Y;
    Result.Elements[1][1] = U.Y;
    Result.Elements[1][2] = -F.Y;
    Result.Elements[1][3] = 0.0f;
    
    Result.Elements[2][0] = S.Z;
    Result.Elements[2][1] = U.Z;
    Result.Elements[2][2] = -F.Z;
    Result.Elements[2][3] = 0.0f;
    
    Result.Elements[3][0] = -DotVec3(S, Eye);
    Result.Elements[3][1] = -DotVec3(U, Eye);
    Result.Elements[3][2] = DotVec3(F, Eye);
    Result.Elements[3][3] = 1.0f;
    
    return (Result);
}


//~
// Converting
//

internal vec2f32
Vec2f32FromVec2i32(vec2i32 a)
{
    vec2f32 result = Vec2((f32)a.x, (f32)a.y);
    return result;
}



inline v2
Vec2MoveTowards(v2 start, v2 end, f32 amount)
{
    v2 result = start + amount * NormalizeVec2(end - start);
    result = ClampInsideRect(RectTwoPoints(start, end), result);
    return result;
}

inline f32
MoveTowards(f32 start, f32 end, f32 amount)
{
    if (start > end)
    {
        amount = -amount;
    }
    f32 result = start + amount;
    result = Clamp(Min(start, end),result, Max(start, end));
    return result;
}


internal inline v3 
Lerp(v3 A, f32 Time, v3 B)
{
    v3 Result = (1.0f - Time) * A + Time * B;
    
    return (Result);
}

#endif //BASE_MATH_H
