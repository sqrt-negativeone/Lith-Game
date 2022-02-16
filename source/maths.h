/* date = October 10th 2021 10:34 am */

#ifndef MATH_H
#define MATH_H

#include "HandmadeMath.h"
#undef near
#undef far

typedef hmm_v2 v2;
typedef hmm_dvec2 dv2;
typedef hmm_ivec2 iv2;
typedef hmm_v3 v3;
typedef hmm_v4 v4;
typedef hmm_m4 m4;
typedef hmm_quaternion quaternion;

internal
f32 sin_f(f32 radians)
{
    f32 result = HMM_SinF(radians);
    return result;
}

internal
f32 cos_f(f32 radians)
{
    f32 result = HMM_CosF(radians);
    return result;
}

internal
f32 tan_f(f32 radians)
{
    f32 result = HMM_TanF(radians);
    return result;
}

internal
f32 acos_f(f32 radians)
{
    f32 result = HMM_ACosF(radians);
    return result;
}

internal
f32 atan_f(f32 radians)
{
    f32 result = HMM_ATanF(radians);
    return result;
}

internal
f32 atan2_f(f32 left, f32 right)
{
    f32 result = HMM_ATan2F(left, right);
    return result;
}

internal
f32 exp_f(f32 value)
{
    f32 result = HMM_ExpF(value);
    return result;
}

internal
f32 log_f(f32 value)
{
    f32 result = HMM_LogF(value);
    return result;
}

internal
f32 square_f(f32 value)
{
    f32 result = value * value;
    return result;
}

internal
f32 square_root_f(f32 value)
{
    f32 result = HMM_SquareRootF(value);
    return result;
}

internal
f32 r_square_root_f(f32 value)
{
    f32 result = HMM_RSquareRootF(value);
    return result;
}

internal
f32 power(f32 base, i32 exponent)
{
    f32 result = HMM_Power(base, exponent);
    return result;
}

internal
f32 power_f(f32 base, f32 exponent)
{
    f32 result = HMM_PowerF(base, exponent);
    return result;
}

internal
f32 to_radians(f32 degrees)
{
    f32 result = HMM_ToRadians(degrees);
    return result;
}

internal
f32 lerp(f32 a, f32 time, f32 b)
{
    f32 result = HMM_Lerp(a, time, b);
    return result;
}

internal
f32 Clamp(f32 min, f32 value, f32 max)
{
    f32 result = HMM_Clamp(min, value, max);
    return result;
}

internal
v2 vec2(f32 x, f32 y)
{
    v2 result = HMM_Vec2(x, y);
    return result;
}

internal
v2 vec2i(i32 x, i32 y)
{
    v2 result = HMM_Vec2i(x, y);
    return result;
}

internal
v2 vec2iv(iv2 v)
{
    v2 result = vec2i(v.x, v.y);
    return result;
}

internal
v3 vec3(f32 x, f32 y, f32 z)
{
    v3 result = HMM_Vec3(x, y ,z);
    return result;
}

internal
v3 vec3(v2 xy, f32 z)
{
    v3 result = HMM_Vec3(xy.x, xy.y ,z);
    return result;
}

internal
v3 vec3i(int x, int y, int z)
{
    v3 result = HMM_Vec3i(x, y , z);
    return result;
}

internal
v4 vec4(float d)
{
    v4 result = HMM_Vec4(d, d, d, d);
    return result;
}

internal
v4 vec4(float x, float y, float z, float w)
{
    v4 result = HMM_Vec4(x, y, z, w);
    return result;
}

internal
v4 vec4i(int x, int y, int z, int w)
{
    v4 result = HMM_Vec4i(x, y, z, w);
    return result;
}

internal
v4 vec4v(v3 vector, float w)
{
    v4 result = HMM_Vec4v(vector, w);
    return result;
}

internal
f32 DotVec2(hmm_vec2 vec_1, hmm_vec2 vec_2)
{
    f32 result = HMM_DotVec2(vec_1, vec_2);
    return result;
}

internal
f32 DotVec3(hmm_vec3 vec_1, hmm_vec3 vec_2)
{
    f32 result = HMM_DotVec3(vec_1, vec_2);
    return result;
}

internal
f32 DotVec4(hmm_vec4 vec_1, hmm_vec4 vec_2)
{
    f32 result = HMM_DotVec4(vec_1, vec_2);
    return result;
}

internal
v3 Cross(hmm_vec3 vec_1, hmm_vec3 vec_2)
{
    v3 result = HMM_Cross(vec_1, vec_2);
    return result;
}

internal
f32 LengthSquaredVec2(hmm_vec2 a)
{
    f32 result = HMM_LengthSquaredVec2(a);
    return result;
}

internal
f32 LengthSquaredVec3(hmm_vec3 a)
{
    f32 result = HMM_LengthSquaredVec3(a);
    return result;
}

internal
f32 LengthSquaredVec4(hmm_vec4 a)
{
    f32 result = HMM_LengthSquaredVec4(a);
    return result;
}

internal
f32 LengthVec2(hmm_vec2 a)
{
    f32 result = HMM_LengthVec2(a);
    return result;
}

internal
f32 LengthVec3(hmm_vec3 a)
{
    f32 result = HMM_LengthVec3(a);
    return result;
}

internal
f32 LengthVec4(hmm_vec4 a)
{
    f32 result = HMM_LengthVec4(a);
    return result;
}

internal
v2 NormalizeVec2(hmm_vec2 a)
{
    v2 result = HMM_NormalizeVec2(a);
    return result;
}

internal
v3 NormalizeVec3(hmm_vec3 a)
{
    v3 result = HMM_NormalizeVec3(a);
    return result;
}

internal
v4 normalize_vec4(hmm_vec4 a)
{
    v4 result = HMM_NormalizeVec4(a);
    return result;
}

internal
v2 FastNormalizeVec2(hmm_vec2 a)
{
    v2 result = HMM_FastNormalizeVec2(a);
    return result;
}

internal
v3 FastNormalizeVec3(hmm_vec3 a)
{
    v3 result = HMM_FastNormalizeVec3(a);
    return result;
}

internal
v4 FastNormalizeVec4(hmm_vec4 a)
{
    v4 result = HMM_FastNormalizeVec4(a);
    return result;
}

internal
m4 Mat4()
{
    m4 result = HMM_Mat4();
    return result;
}

internal
m4 mat4d(f32 diagonal)
{
    m4 result = HMM_Mat4d(diagonal);
    return result;
}

internal
m4 Transpose(hmm_mat4 matrix)
{
    m4 result = HMM_Transpose(matrix);
    return result;
}

internal
m4 m4_orthographic(float left, float right, float bottom, float top, float near, float far)
{
    m4 result = HMM_Orthographic(left, right, bottom, top, near, far);
    return result;
}

internal
m4 Perspective(float FOV, float AspectRatio, float Near, float Far)
{
    m4 result = HMM_Perspective(FOV, AspectRatio, Near, Far);
    return result;
}

internal
m4 m4_translate(hmm_vec3 translation)
{
    m4 result = HMM_Translate(translation);
    return result;
}

internal
m4 m4_rotate(float Angle, hmm_vec3 Axis)
{
    m4 result = HMM_Rotate(Angle, Axis);
    return result;
}

internal
m4 m4_scale(hmm_vec3 s)
{
    m4 result = HMM_Scale(s);
    return result;
}

internal
m4 m4_look_at(hmm_vec3 Eye, hmm_vec3 Center, hmm_vec3 Up)
{
    m4 result = HMM_LookAt(Eye, Center, Up);
    return result;
}

internal
quaternion Quaternion(float x, float y, float z, float w)
{
    quaternion result = HMM_Quaternion(x, y ,z ,w);
    return result;
}

internal
quaternion QuaternionV4(hmm_vec4 vector)
{
    quaternion result = HMM_QuaternionV4(vector);
    return result;
}

internal
f32 DotQuaternion(hmm_quaternion left, hmm_quaternion right)
{
    f32 result = HMM_DotQuaternion(left, right);
    return result;
}

internal
quaternion InverseQuaternion(hmm_quaternion left)
{
    quaternion result = HMM_InverseQuaternion(left);
    return result;
}

internal
quaternion NormalizeQuaternion(hmm_quaternion left)
{
    quaternion result = HMM_NormalizeQuaternion(left);
    return result;
}

internal
quaternion NLerp(hmm_quaternion left, float time, hmm_quaternion right)
{
    quaternion result = HMM_NLerp(left, time, right);
    return result;
}

internal
quaternion Slerp(hmm_quaternion left, float time, hmm_quaternion right)
{
    quaternion result = HMM_Slerp(left, time, right);
    return result;
}

internal
m4 QuaternionToMat4(hmm_quaternion left)
{
    m4 result = HMM_QuaternionToMat4(left);
    return result;
}

internal
quaternion Mat4ToQuaternion(hmm_mat4 m)
{
    quaternion result = HMM_Mat4ToQuaternion(m);
    return result;
}

internal
quaternion QuaternionFromAxisAngle(hmm_vec3 axis, float AngleOfRotation)
{
    quaternion result = HMM_QuaternionFromAxisAngle(axis, AngleOfRotation);
    return result;
}

struct Rectangle2D
{
    v2 min_p;
    v2 max_p;
};

inline Rectangle2D
RectMinMax(v2 min_p, v2 max_p)
{
    Rectangle2D result;
    result.min_p = min_p;
    result.max_p = max_p;
    return result;
}

inline Rectangle2D
RectTwoPoints(v2 a, v2 b)
{
    Rectangle2D result;
    result.min_p = vec2(Min(a.x, b.x), Min(a.y, b.y));
    result.max_p = vec2(Max(a.x, b.x), Max(a.y, b.y));
    return result;
}

inline Rectangle2D
RectCentHalfDim(v2 center, v2 half_dim)
{
    Rectangle2D result = RectMinMax(center - half_dim, center + half_dim);
    return result;
}

inline Rectangle2D
RectCentDim(v2 center, v2 dim)
{
    Rectangle2D result = RectCentHalfDim(center, 0.5f * dim);
    return result;
}

inline b32
IsInsideRect(Rectangle2D rect, v2 pos)
{
    b32 result = ((pos.x >= rect.min_p.x && pos.x < rect.max_p.x) &&
                  (pos.y >= rect.min_p.y && pos.y < rect.max_p.y));
    return result;
}

inline v2
ClampInsideRect(Rectangle2D rect, v2 p)
{
    v2 result;
    result.x = Clamp(rect.min_p.x, p.x, rect.max_p.x);
    result.y = Clamp(rect.min_p.y, p.y, rect.max_p.y);
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

#endif //MATH_H
