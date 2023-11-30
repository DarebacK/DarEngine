#pragma once

/**
 * @brief Left handed coordinate system, row major matrices, row vectors (pre-multiplication).
 */

#include <algorithm>
#include <cmath>

#include "Core/Core.hpp"

constexpr float Pi = 3.14159265358979323846f;

template<typename T>
inline T cotan(T degreeInRadians) noexcept
{
  return cos(degreeInRadians) / sin(degreeInRadians);
}

constexpr inline float lerp(float v1, float v2, float t)
{
  return (1.f - t)*v1 + t*v2;
}

struct Vec2f
{
  float x, y;
};
constexpr inline Vec2f operator-(const Vec2f& v) noexcept { return { -v.x, -v.y }; }
constexpr inline Vec2f operator-(const Vec2f& left, const Vec2f& right) noexcept 
{
  return Vec2f{ left.x - right.x, left.y - right.y };
}
constexpr inline Vec2f operator+(const Vec2f& left, const Vec2f& right) noexcept
{
  return Vec2f{ left.x + right.x, left.y + right.y };
}
constexpr inline Vec2f operator*(float left, const Vec2f& right) noexcept
{
  return Vec2f{ left * right.x, left * right.y };
}
constexpr inline Vec2f operator*(const Vec2f& left, float right) noexcept
{
  return right * left;
}
constexpr inline bool operator==(const Vec2f& left, const Vec2f& right) noexcept 
{
  return left.x == right.x && left.y == right.y;
}
constexpr inline bool operator!=(const Vec2f& left, const Vec2f& right) noexcept { return !(left == right); }
struct Vec3f
{
  static constexpr Vec3f up() noexcept { return { 0.f, 1.f, 0.f }; }

  Vec3f& operator+=(const Vec3f& rhs) noexcept
  {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
  }
  Vec3f& operator*=(float rhs) noexcept
  {
    x *= rhs;
    y *= rhs;
    z *= rhs;
    return *this;
  }

  float x, y, z;
};
constexpr inline Vec3f operator-(const Vec3f& v) noexcept { return { -v.x, -v.y, -v.z }; }
constexpr inline Vec3f operator-(const Vec3f& left, const Vec3f& right) noexcept
{
  return Vec3f{ left.x - right.x, left.y - right.y, left.z - right.z };
}
constexpr inline Vec3f operator+(const Vec3f& left, const Vec3f& right) noexcept
{
  return Vec3f{ left.x + right.x, left.y + right.y, left.z + right.z };
}
constexpr inline Vec3f operator*(float left, const Vec3f& right) noexcept
{
  return Vec3f{ left * right.x, left * right.y, left * right.z };
}
constexpr inline Vec3f operator*(const Vec3f& left, float right) noexcept
{
  return right * left;
}
constexpr inline bool operator==(const Vec3f& left, const Vec3f& right) noexcept
{
  return left.x == right.x && left.y == right.y && left.z == right.z;
}
constexpr inline bool operator!=(const Vec3f& left, const Vec3f& right) noexcept { return !(left == right); }
constexpr inline Vec3f lerp(const Vec3f& v1, const Vec3f& v2, float t, float oneMinusT) noexcept
{
  return
  {
    oneMinusT*v1.x + t*v2.x,
    oneMinusT*v1.y + t*v2.y,
    oneMinusT*v1.z + t*v2.z,
  };
}
constexpr inline Vec3f lerp(const Vec3f& v1, const Vec3f& v2, float t) noexcept
{
  return lerp(v1, v2, t, 1.f - t);
}
inline Vec3f min(const Vec3f& v1, const Vec3f& v2) noexcept
{
  return { std::min(v1.x, v2.x), std::min(v1.y, v2.y), std::min(v1.z, v2.z) };
}
inline Vec3f max(const Vec3f& v1, const Vec3f& v2) noexcept
{
  return { std::max(v1.x, v2.x), std::max(v1.y, v2.y), std::max(v1.z, v2.z)};
}

struct Vec4f
{
  float x, y, z, w;
};
constexpr inline Vec4f operator-(const Vec4f& v) noexcept { return { -v.x, -v.y, -v.z, -v.w }; }
constexpr inline Vec4f operator-(const Vec4f& left, const Vec4f& right) noexcept
{
  return Vec4f{ left.x - right.x, left.y - right.y, left.z - right.z, left.w - right.w };
}
constexpr inline Vec4f operator+(const Vec4f& left, const Vec4f& right) noexcept
{
  return Vec4f{ left.x + right.x, left.y + right.y, left.z + right.z, left.w + right.w };
}
constexpr inline Vec4f operator*(float left, const Vec4f& right) noexcept
{
  return Vec4f{ left * right.x, left * right.y, left * right.z, left * right.w };
}
constexpr inline Vec4f operator*(const Vec4f& left, float right) noexcept
{
  return right * left;
}
constexpr inline Vec4f operator/(const Vec4f& left, float right)
{
  return Vec4f{ left.x / right, left.y / right, left.z / right, left.w / right };
}
constexpr inline Vec4f& operator/=(Vec4f& left, float right)
{
  left.x /= right;
  left.y /= right;
  left.z /= right; 
  left.w /= right;
  return left;
};
constexpr inline bool operator==(const Vec4f& left, const Vec4f& right) noexcept
{
  return left.x == right.x && left.y == right.y && left.z == right.z && left.w == right.w;
}
constexpr inline bool operator!=(const Vec4f& left, const Vec4f& right) noexcept { return !(left == right); }

constexpr inline float dot(const Vec2f& v1, const Vec2f& v2) noexcept
{
  return (v1.x * v2.x) + (v1.y * v2.y);
}
constexpr inline float dot(const Vec3f& v1, const Vec3f& v2) noexcept
{
  return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
}
constexpr inline float dot(const Vec4f& v1, const Vec4f& v2) noexcept
{
  return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z) + (v1.w * v2.w);
}
inline __m128 dotSimd(__m128 v1, __m128 v2) noexcept
{
  // From https://github.com/microsoft/DirectXMath/blob/83634c742a85d1027765af53fbe79506fd72e0c3/Inc/DirectXMathVector.inl
  __m128 vTemp2 = v2;
  __m128 vTemp = _mm_mul_ps(v1, vTemp2);
  vTemp2 = _mm_shuffle_ps(vTemp2, vTemp, _MM_SHUFFLE(1, 0, 0, 0)); // Copy X to the Z position and Y to the W position
  vTemp2 = _mm_add_ps(vTemp2, vTemp);          // Add Z = X+Z; W = Y+W;
  vTemp = _mm_shuffle_ps(vTemp, vTemp2, _MM_SHUFFLE(0, 3, 0, 0));  // Copy W to the Z position
  vTemp = _mm_add_ps(vTemp, vTemp2);           // Add Z and W together
  return PERMUTE_PS(vTemp, _MM_SHUFFLE(2, 2, 2, 2));    // Splat Z and return
}

constexpr inline Vec3f cross(const Vec3f& v1, const Vec3f& v2) noexcept
{
  return {(v1.y * v2.z) - (v1.z * v2.y), (v1.z * v2.x) - (v1.x * v2.z), (v1.x * v2.y) - (v1.y * v2.x)};
}

struct Vec2i
{
  int x, y;
};
struct Vec3i
{
  Vec3i& operator+=(const Vec3i& rhs) noexcept
  {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
  }

  int x, y, z;
};
constexpr inline Vec3f toVec3f(const Vec3i& v) noexcept { return { float(v.x), float(v.y), float(v.z) }; }
constexpr inline Vec3f toVec3f(const Vec4f& v) { return { v.x, v.y, v.z }; }
constexpr inline Vec4f toVec4f(const Vec3i& v, float w) { return { float(v.x), float(v.y), float(v.z), w }; }
constexpr inline Vec4f toVec4f(const Vec3f& v, float w) { return { v.x, v.y, v.z, w }; }
inline Vec3i toVec3iRounded(const Vec3f& v) noexcept { return { int(std::round(v.x)), int(std::round(v.y)), int(std::round(v.z)) }; }
inline Vec3i toVec3iRounded(const Vec4f& v) noexcept { return { int(std::round(v.x)), int(std::round(v.y)), int(std::round(v.z)) }; }
constexpr inline Vec3i operator-(const Vec3i& v) noexcept { return { -v.x, -v.y, -v.z }; }
constexpr inline Vec3i operator-(const Vec3i& left, const Vec3i& right) noexcept
{
  return Vec3i{ left.x - right.x, left.y - right.y, left.z - right.z };
}
constexpr inline Vec3i operator+(const Vec3i& left, const Vec3i& right) noexcept
{
  return Vec3i{ left.x + right.x, left.y + right.y, left.z + right.z };
}
constexpr inline Vec3i operator*(int left, const Vec3i& right) noexcept
{
  return Vec3i{ left * right.x, left * right.y, left * right.z };
}
constexpr inline Vec3i operator*(const Vec3i& left, int right) noexcept
{
  return right * left;
}
constexpr inline bool operator==(const Vec3i& left, const Vec3i& right) noexcept
{
  return left.x == right.x && left.y == right.y && left.z == right.z;
}
constexpr inline bool operator!=(const Vec3i& left, const Vec3i& right) noexcept { return !(left == right); }

inline auto length(const Vec2f& v) noexcept
{
  return std::sqrt(dot(v, v));
}
inline auto length(const Vec3f& v) noexcept
{
  return std::sqrt(dot(v, v));
}
inline auto length(const Vec4f& v) noexcept
{
  return std::sqrt(dot(v, v));
}

inline Vec2f normalized(const Vec2f& v, float length) noexcept
{
  return {v.x / length, v.y / length};
}
inline Vec2f normalized(const Vec2f& v) noexcept
{
  return normalized(v, length(v));
}
inline Vec3f normalized(const Vec3f& v, float length) noexcept
{
  return { v.x / length, v.y / length, v.z / length };
}
inline Vec3f normalized(const Vec3f& v) noexcept
{
  return normalized(v, length(v));
}
inline Vec4f normalized(const Vec4f& v, float length) noexcept
{
  return { v.x / length, v.y / length, v.z / length, v.w / length };
}
inline Vec4f normalized(const Vec4f& v) noexcept
{
  return normalized(v, length(v));
}

inline bool isNormalized(const Vec2f& v) noexcept { return abs(length(v) - 1.f) <= FLT_EPSILON; }
inline bool isNormalized(const Vec3f& v) noexcept { return abs(length(v) - 1.f) <= FLT_EPSILON; }
inline bool isNormalized(const Vec4f& v) noexcept { return abs(length(v) - 1.f) <= FLT_EPSILON; }

inline float clampAngle(float angle)
{
  while(angle >= 2 * Pi) {
    angle -= 2 * Pi;
  } /*else*/ while(angle < 0.f) {
    angle += 2 * Pi;
  }
  return angle;
}

constexpr inline float degreesToRadians(float degrees) noexcept
{
  return degrees * Pi / 180.f;
}
constexpr inline float radiansToDegrees(float radians) noexcept
{
  return radians * 180.f / Pi;
}

struct Mat3f
{
  constexpr static Mat3f identity() noexcept
  {
    return
    {{
      {1.0f, 0.0f, 0.0f},
      {0.0f, 1.0f, 0.0f},
      {0.0f, 0.0f, 1.0f}
    }};
  }
  static Mat3f rotationX(float radians) noexcept
  {
    return
    {{
     {1.f,           0.f,          0.f},
     {0.f,  cos(radians), sin(radians)},
     {0.f, -sin(radians), cos(radians)}
    }};
  }
  static Mat3f rotationY(float radians) noexcept
  {
    return
    {{
     {cos(radians), 0.f, -sin(radians)},
     {         0.f, 1.f,           0.f},
     {sin(radians), 0.f,  cos(radians)},
    }};
  }
  static Mat3f rotationZ(float radians) noexcept
  {
    return
    {{
     { cos(radians), sin(radians), 0.f},
     {-sin(radians), cos(radians), 0.f},
     {          0.f,          0.f, 1.f},
    }};
  }

  float* operator[](int index) noexcept { return values[index]; }
  const float* operator[](int index) const noexcept { return values[index]; }

  float values[3][3];
};
Vec3f operator*(const Vec3f& left, const Mat3f& right) noexcept;
Mat3f operator*(const Mat3f& left, const Mat3f& right) noexcept;
/**
 * @brief Optimization for Matrices where last column is 0.f, 0.f, 0.f, 1.f
 */
struct Mat4x3f
{
  constexpr static Mat4x3f identity() noexcept
  {
    return
    {
     1.0f, 0.0f, 0.0f,
     0.0f, 1.0f, 0.0f,
     0.0f, 0.0f, 1.0f,
     0.0f, 0.0f, 0.0f
    };
  }
  constexpr static Mat4x3f translation(float x, float y, float z) noexcept
  {
    return
    {
     1.0f, 0.0f, 0.0f,
     0.0f, 1.0f, 0.0f,
     0.0f, 0.0f, 1.0f,
        x,    y,    z
    };
  }
  constexpr static Mat4x3f translation(const Vec3f& by) noexcept
  {
    return translation(by.x, by.y, by.z);
  }
  static Mat4x3f rotationX(float radians) noexcept
  {
    return
    {
     1.f,           0.f,          0.f,
     0.f,  cos(radians), sin(radians),
     0.f, -sin(radians), cos(radians),
     0.f,           0.f,          0.f
    };
  }
  static Mat4x3f rotationY(float radians) noexcept
  {
    return
    {
     cos(radians), 0.f, -sin(radians),
              0.f, 1.f,           0.f,
     sin(radians), 0.f,  cos(radians),
              0.f, 0.f,           0.f
    };
  }
  static Mat4x3f rotationZ(float radians) noexcept
  {
    return
    {
      cos(radians), sin(radians), 0.f,
     -sin(radians), cos(radians), 0.f,
               0.f,          0.f, 1.f,
               0.f,          0.f, 0.f
    };
  }
  constexpr static Mat4x3f scale(float x, float y, float z) noexcept
  {
    return
    {
        x, 0.f, 0.f,
      0.f,   y, 0.f,
      0.f, 0.f,   z,
      0.f, 0.f, 0.f
    };
  }
  static Mat4x3f lookAt(const Vec3f& eyePosition, const Vec3f& focusPosition, const Vec3f& upDirection) noexcept
  {
    assert(eyePosition != focusPosition);
    Vec3f eyeDirection = normalized(focusPosition - eyePosition);
    return lookTo(eyePosition, eyeDirection, upDirection);
  }
  /**
   * @param eyeDirection has to be normalized
   */
  static Mat4x3f lookTo(const Vec3f& eyePosition, const Vec3f& eyeDirection, const Vec3f& upDirection) noexcept
  {
    assert(eyeDirection != (Vec3f{ 0.f, 0.f, 0.f }));
    assert(upDirection != (Vec3f{ 0.f, 0.f, 0.f }));
    assert(isNormalized(eyeDirection));
    // from https://github.com/microsoft/DirectXMath/blob/83634c742a85d1027765af53fbe79506fd72e0c3/Inc/DirectXMathMatrix.inl
    Vec3f r0 = normalized(cross(upDirection, eyeDirection));
    Vec3f r1 = cross(eyeDirection, r0);
    Vec3f negatedEyePosition = -eyePosition;
    float d0 = dot(r0, negatedEyePosition);
    float d1 = dot(r1, negatedEyePosition);
    float d2 = dot(eyeDirection, negatedEyePosition);
    return
    { {
     {r0.x, r1.x, eyeDirection.x},
     {r0.y, r1.y, eyeDirection.y},
     {r0.z, r1.z, eyeDirection.z},
     {  d0,   d1,             d2}
    } };
  }

  float* operator[](int index) noexcept { return values[index]; }
  const float* operator[](int index) const noexcept { return values[index]; }

  float values[4][3];
};
Vec4f operator*(const Vec4f& left, const Mat4x3f& right) noexcept;
Mat4x3f operator*(const Mat3f& left, const Mat4x3f& right) noexcept;
Mat4x3f operator*(const Mat4x3f& left, const Mat3f& right) noexcept;
Mat4x3f operator*(const Mat4x3f& left, const Mat4x3f& right) noexcept;
struct Mat4f
{
  constexpr static Mat4f identity() noexcept
  {
    return 
    {
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };
  }
  constexpr static Mat4f translation(float x, float y, float z) noexcept
  {
    return 
    {
      1.0f, 0.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f, 0.0f,
      0.0f, 0.0f, 1.0f, 0.0f,
         x,    y,    z, 1.0f
    };
  }
  constexpr static Mat4f translation(const Vec3f& by) noexcept
  {
    return translation(by.x, by.y, by.z);
  }
  static Mat4f rotationX(float radians) noexcept
  {
    return Mat4f
    {
      1.f,           0.f,          0.f, 0.f,
      0.f,  cos(radians), sin(radians), 0.f,
      0.f, -sin(radians), cos(radians), 0.f,
      0.f,           0.f,          0.f, 1.f
    };
  }
  static Mat4f rotationY(float radians) noexcept
  {
    return
    {
      cos(radians), 0.f, -sin(radians), 0.f,
               0.f, 1.f,           0.f, 0.f,
      sin(radians), 0.f,  cos(radians), 0.f,
               0.f, 0.f,           0.f, 1.f
    };
  }
  static Mat4f rotationZ(float radians) noexcept
  {
    return
    { 
      cos(radians), sin(radians), 0.f, 0.f,
     -sin(radians), cos(radians), 0.f, 0.f,
               0.f,          0.f, 1.f, 0.f,
               0.f,          0.f, 0.f, 1.f
    };
  }
  static Mat4f perspectiveProjectionD3d(
    float verticalFieldOfViewRadians, 
    float aspectRatio, 
    float nearZ, 
    float farZ
  ) noexcept
  {
    // from https://github.com/microsoft/DirectXMath/blob/83634c742a85d1027765af53fbe79506fd72e0c3/Inc/DirectXMathMatrix.inl
    verticalFieldOfViewRadians *= 0.5f;
    float vFovSin = sin(verticalFieldOfViewRadians);
    float vFovCos = cos(verticalFieldOfViewRadians);
    float height = vFovCos / vFovSin;
    float width = height / aspectRatio;
    float fRange = farZ / (farZ - nearZ);
    return 
    {
      width,    0.f,             0.f, 0.f,
        0.f, height,             0.f, 0.f,
        0.f,    0.f,          fRange, 1.f,
        0.f,    0.f, -fRange * nearZ, 0.f
    };
  }
  static Mat4f lookAt(const Vec3f& eyePosition, const Vec3f& focusPosition, const Vec3f& upDirection) noexcept
  {
    assert(eyePosition != focusPosition);
    Vec3f eyeDirection = normalized(focusPosition - eyePosition);
    return lookTo(eyePosition, eyeDirection, upDirection);
  }
  /**
   * @param eyeDirection has to be normalized
   */
  static Mat4f lookTo(const Vec3f& eyePosition, const Vec3f& eyeDirection, const Vec3f& upDirection) noexcept
  {
    assert(eyeDirection != (Vec3f{ 0.f, 0.f, 0.f }));
    assert(upDirection != (Vec3f{ 0.f, 0.f, 0.f }));
    assert(isNormalized(eyeDirection));
    // from https://github.com/microsoft/DirectXMath/blob/83634c742a85d1027765af53fbe79506fd72e0c3/Inc/DirectXMathMatrix.inl
    Vec3f r0 = normalized(cross(upDirection, eyeDirection));
    Vec3f r1 = cross(eyeDirection, r0);
    Vec3f negatedEyePosition = -eyePosition;
    float d0 = dot(r0, negatedEyePosition);
    float d1 = dot(r1, negatedEyePosition);
    float d2 = dot(eyeDirection, negatedEyePosition);
    return
    {
      r0.x, r1.x, eyeDirection.x, 0.f,
      r0.y, r1.y, eyeDirection.y, 0.f,
      r0.z, r1.z, eyeDirection.z, 0.f,
        d0,   d1,             d2, 1.f
    };
  }

  float* operator[](int index) noexcept {return values[index];}
  const float* operator[](int index) const noexcept {return values[index];}

  union
  {
    float values[4][4];
    __m128 vectors[4];
  };
};
Vec4f operator*(const Vec4f& left, const Mat4f& right) noexcept;
Mat4f operator*(const Mat4x3f& left, const Mat4f& right) noexcept;
Mat4f operator*(const Mat4f& left, const Mat4x3f& right) noexcept;
Mat4f operator*(const Mat4f& left, const Mat4f& right) noexcept;
inline Mat4f toMat4f(const Mat4x3f& m) noexcept 
{
  return
  {
    m[0][0], m[0][1], m[0][2], 0.f,
    m[1][0], m[1][1], m[1][2], 0.f,
    m[2][0], m[2][1], m[2][2], 0.f,
    m[3][0], m[3][1], m[3][2], 1.f
  };
}
inline Mat4f inversed(const Mat4f& matrix)
{
  // TODO: implement
   
  // from https://github.com/microsoft/DirectXMath/blob/83634c742a85d1027765af53fbe79506fd72e0c3/Inc/DirectXMathMatrix.inl

  __m128 vTemp1 = _mm_shuffle_ps(matrix.vectors[0], matrix.vectors[1], _MM_SHUFFLE(1, 0, 1, 0));
  __m128 vTemp3 = _mm_shuffle_ps(matrix.vectors[0], matrix.vectors[1], _MM_SHUFFLE(3, 2, 3, 2));
  __m128 vTemp2 = _mm_shuffle_ps(matrix.vectors[2], matrix.vectors[3], _MM_SHUFFLE(1, 0, 1, 0));
  __m128 vTemp4 = _mm_shuffle_ps(matrix.vectors[2], matrix.vectors[3], _MM_SHUFFLE(3, 2, 3, 2));

  Mat4f MT;
  MT.vectors[0] = _mm_shuffle_ps(vTemp1, vTemp2, _MM_SHUFFLE(2, 0, 2, 0));
  MT.vectors[1] = _mm_shuffle_ps(vTemp1, vTemp2, _MM_SHUFFLE(3, 1, 3, 1));
  MT.vectors[2] = _mm_shuffle_ps(vTemp3, vTemp4, _MM_SHUFFLE(2, 0, 2, 0));
  MT.vectors[3] = _mm_shuffle_ps(vTemp3, vTemp4, _MM_SHUFFLE(3, 1, 3, 1));

  __m128 V00 = PERMUTE_PS(MT.vectors[2], _MM_SHUFFLE(1, 1, 0, 0));
  __m128 V10 = PERMUTE_PS(MT.vectors[3], _MM_SHUFFLE(3, 2, 3, 2));
  __m128 V01 = PERMUTE_PS(MT.vectors[0], _MM_SHUFFLE(1, 1, 0, 0));
  __m128 V11 = PERMUTE_PS(MT.vectors[1], _MM_SHUFFLE(3, 2, 3, 2));
  __m128 V02 = _mm_shuffle_ps(MT.vectors[2], MT.vectors[0], _MM_SHUFFLE(2, 0, 2, 0));
  __m128 V12 = _mm_shuffle_ps(MT.vectors[3], MT.vectors[1], _MM_SHUFFLE(3, 1, 3, 1));

  __m128 D0 = _mm_mul_ps(V00, V10);
  __m128 D1 = _mm_mul_ps(V01, V11);
  __m128 D2 = _mm_mul_ps(V02, V12);

  V00 = PERMUTE_PS(MT.vectors[2], _MM_SHUFFLE(3, 2, 3, 2));
  V10 = PERMUTE_PS(MT.vectors[3], _MM_SHUFFLE(1, 1, 0, 0));
  V01 = PERMUTE_PS(MT.vectors[0], _MM_SHUFFLE(3, 2, 3, 2));
  V11 = PERMUTE_PS(MT.vectors[1], _MM_SHUFFLE(1, 1, 0, 0));
  V02 = _mm_shuffle_ps(MT.vectors[2], MT.vectors[0], _MM_SHUFFLE(3, 1, 3, 1));
  V12 = _mm_shuffle_ps(MT.vectors[3], MT.vectors[1], _MM_SHUFFLE(2, 0, 2, 0));

  D0 = FNMADD_PS(V00, V10, D0);
  D1 = FNMADD_PS(V01, V11, D1);
  D2 = FNMADD_PS(V02, V12, D2);
  // V11 = D0Y,D0W,D2Y,D2Y
  V11 = _mm_shuffle_ps(D0, D2, _MM_SHUFFLE(1, 1, 3, 1));
  V00 = PERMUTE_PS(MT.vectors[1], _MM_SHUFFLE(1, 0, 2, 1));
  V10 = _mm_shuffle_ps(V11, D0, _MM_SHUFFLE(0, 3, 0, 2));
  V01 = PERMUTE_PS(MT.vectors[0], _MM_SHUFFLE(0, 1, 0, 2));
  V11 = _mm_shuffle_ps(V11, D0, _MM_SHUFFLE(2, 1, 2, 1));
  // V13 = D1Y,D1W,D2W,D2W
  __m128 V13 = _mm_shuffle_ps(D1, D2, _MM_SHUFFLE(3, 3, 3, 1));
  V02 = PERMUTE_PS(MT.vectors[3], _MM_SHUFFLE(1, 0, 2, 1));
  V12 = _mm_shuffle_ps(V13, D1, _MM_SHUFFLE(0, 3, 0, 2));
  __m128 V03 = PERMUTE_PS(MT.vectors[2], _MM_SHUFFLE(0, 1, 0, 2));
  V13 = _mm_shuffle_ps(V13, D1, _MM_SHUFFLE(2, 1, 2, 1));

  __m128 C0 = _mm_mul_ps(V00, V10);
  __m128 C2 = _mm_mul_ps(V01, V11);
  __m128 C4 = _mm_mul_ps(V02, V12);
  __m128 C6 = _mm_mul_ps(V03, V13);

  // V11 = D0X,D0Y,D2X,D2X
  V11 = _mm_shuffle_ps(D0, D2, _MM_SHUFFLE(0, 0, 1, 0));
  V00 = PERMUTE_PS(MT.vectors[1], _MM_SHUFFLE(2, 1, 3, 2));
  V10 = _mm_shuffle_ps(D0, V11, _MM_SHUFFLE(2, 1, 0, 3));
  V01 = PERMUTE_PS(MT.vectors[0], _MM_SHUFFLE(1, 3, 2, 3));
  V11 = _mm_shuffle_ps(D0, V11, _MM_SHUFFLE(0, 2, 1, 2));
  // V13 = D1X,D1Y,D2Z,D2Z
  V13 = _mm_shuffle_ps(D1, D2, _MM_SHUFFLE(2, 2, 1, 0));
  V02 = PERMUTE_PS(MT.vectors[3], _MM_SHUFFLE(2, 1, 3, 2));
  V12 = _mm_shuffle_ps(D1, V13, _MM_SHUFFLE(2, 1, 0, 3));
  V03 = PERMUTE_PS(MT.vectors[2], _MM_SHUFFLE(1, 3, 2, 3));
  V13 = _mm_shuffle_ps(D1, V13, _MM_SHUFFLE(0, 2, 1, 2));

  C0 = FNMADD_PS(V00, V10, C0);
  C2 = FNMADD_PS(V01, V11, C2);
  C4 = FNMADD_PS(V02, V12, C4);
  C6 = FNMADD_PS(V03, V13, C6);

  V00 = PERMUTE_PS(MT.vectors[1], _MM_SHUFFLE(0, 3, 0, 3));
  // V10 = D0Z,D0Z,D2X,D2Y
  V10 = _mm_shuffle_ps(D0, D2, _MM_SHUFFLE(1, 0, 2, 2));
  V10 = PERMUTE_PS(V10, _MM_SHUFFLE(0, 2, 3, 0));
  V01 = PERMUTE_PS(MT.vectors[0], _MM_SHUFFLE(2, 0, 3, 1));
  // V11 = D0X,D0W,D2X,D2Y
  V11 = _mm_shuffle_ps(D0, D2, _MM_SHUFFLE(1, 0, 3, 0));
  V11 = PERMUTE_PS(V11, _MM_SHUFFLE(2, 1, 0, 3));
  V02 = PERMUTE_PS(MT.vectors[3], _MM_SHUFFLE(0, 3, 0, 3));
  // V12 = D1Z,D1Z,D2Z,D2W
  V12 = _mm_shuffle_ps(D1, D2, _MM_SHUFFLE(3, 2, 2, 2));
  V12 = PERMUTE_PS(V12, _MM_SHUFFLE(0, 2, 3, 0));
  V03 = PERMUTE_PS(MT.vectors[2], _MM_SHUFFLE(2, 0, 3, 1));
  // V13 = D1X,D1W,D2Z,D2W
  V13 = _mm_shuffle_ps(D1, D2, _MM_SHUFFLE(3, 2, 3, 0));
  V13 = PERMUTE_PS(V13, _MM_SHUFFLE(2, 1, 0, 3));

  V00 = _mm_mul_ps(V00, V10);
  V01 = _mm_mul_ps(V01, V11);
  V02 = _mm_mul_ps(V02, V12);
  V03 = _mm_mul_ps(V03, V13);
  __m128 C1 = _mm_sub_ps(C0, V00);
  C0 = _mm_add_ps(C0, V00);
  __m128 C3 = _mm_add_ps(C2, V01);
  C2 = _mm_sub_ps(C2, V01);
  __m128 C5 = _mm_sub_ps(C4, V02);
  C4 = _mm_add_ps(C4, V02);
  __m128 C7 = _mm_add_ps(C6, V03);
  C6 = _mm_sub_ps(C6, V03);

  C0 = _mm_shuffle_ps(C0, C1, _MM_SHUFFLE(3, 1, 2, 0));
  C2 = _mm_shuffle_ps(C2, C3, _MM_SHUFFLE(3, 1, 2, 0));
  C4 = _mm_shuffle_ps(C4, C5, _MM_SHUFFLE(3, 1, 2, 0));
  C6 = _mm_shuffle_ps(C6, C7, _MM_SHUFFLE(3, 1, 2, 0));
  C0 = PERMUTE_PS(C0, _MM_SHUFFLE(3, 1, 2, 0));
  C2 = PERMUTE_PS(C2, _MM_SHUFFLE(3, 1, 2, 0));
  C4 = PERMUTE_PS(C4, _MM_SHUFFLE(3, 1, 2, 0));
  C6 = PERMUTE_PS(C6, _MM_SHUFFLE(3, 1, 2, 0));
  // Get the determinant
  __m128 vTemp = dotSimd(C0, MT.vectors[0]);
  //if(pDeterminant != nullptr)
  //  *pDeterminant = vTemp;
  vTemp = _mm_div_ps({ 1.0f, 1.0f, 1.0f, 1.0f }, vTemp);
  Mat4f mResult;
  mResult.vectors[0] = _mm_mul_ps(C0, vTemp);
  mResult.vectors[1] = _mm_mul_ps(C2, vTemp);
  mResult.vectors[2] = _mm_mul_ps(C4, vTemp);
  mResult.vectors[3] = _mm_mul_ps(C6, vTemp);
  return mResult;
}

inline float verticalToHorizontalFieldOfView(float verticalRadians, float aspectRatio) noexcept
{
  // from https://www.reddit.com/r/Planetside/comments/1xl1z5/brief_table_for_calculating_fieldofview_vertical/
  return 2.f * atan(tan(verticalRadians / 2.f) * aspectRatio);
}
inline float horizontalToVerticalFieldOfView(float horizontalRadians, float aspectRatio) noexcept
{
  return 2.f * atan(tan(horizontalRadians / 2.f) * (1.f / aspectRatio));
}

struct Quatf
{
  Vec3f v;
  float s;

  constexpr static Quatf identity() noexcept { return {{0.f, 0.f, 0.f}, 1.f}; }

  // For conversion from a matrix, check Game Engine Architecture page 399

  explicit constexpr operator Mat4f() const noexcept
  {
    float xx = v.x * v.x;
    float xy = v.x * v.y;
    float xz = v.x * v.z;
    float xs = v.x * s;
    float yy = v.y * v.y;
    float yz = v.y * v.z;
    float ys = v.y * s;
    float zz = v.z * v.z;
    float zs = v.z * s;
    return
    {
      1 - 2*yy - 2*zz, 2*xy + 2*zs    , 2*xz - 2*ys    , 0.f,
      2*xy - 2*zs    , 1 - 2*xx - 2*zz, 2*yz + 2*xs    , 0.f,
      2*xz + 2*ys    , 2*yz - 2*xs    , 1 - 2*xx - 2*yy, 0.f,
      0.f            , 0.f            , 0.f            , 1.f
    };
  }
};
/**
 * @brief Grassman product. 
 * There are multiple kinds of quaternion multiplication, but this one is used for 3D rotation.
 * @return Quaternion representing rotation right followed by rotation left
 */
inline Quatf operator*(const Quatf& left, const Quatf& right) noexcept
{
  return
  {
    left.s*right.v + right.s*left.v + cross(left.v, right.v),
    left.s*right.s - dot(left.v, right.v)
  };
}
inline Quatf operator*(float left, const Quatf& right) noexcept
{
  return{left*right.v, left*right.s};
}
inline Quatf operator+(const Quatf& left, const Quatf& right) noexcept
{
  return{left.v + right.v, left.s + right.s};
}
inline float length(const Quatf& q) noexcept
{
  return sqrt(q.v.x*q.v.x + q.v.y*q.v.y + q.v.z*q.v.z + q.s*q.s);
}
/**
 * @note To speed this up for renormalization, check http://allenchou.net/2014/02/game-math-fast-re-normalization-of-unit-vectors/ 
*/
inline Quatf normalized(const Quatf& q, float length) noexcept
{
  return
  {
    {q.v.x / length, q.v.y / length, q.v.z / length},
    q.s / length
  };
}
inline Quatf normalized(const Quatf& q) noexcept
{
  return normalized(q, length(q));
}
/**
 * @note Equals to inverse if q is normalized
 */
inline Quatf conjugate(const Quatf& q) noexcept
{
  return { -q.v, q.s };
}
inline Vec3f rotated(const Vec3f& v, const Quatf& q, const Quatf& qConjugate) noexcept
{
  return (q * Quatf{ v, 0.f } * qConjugate).v;
}
inline Vec3f rotated(const Vec3f& v, const Quatf& q) noexcept
{
  return rotated(v, q, conjugate(q));
}
constexpr inline float dot(const Quatf& q1, const Quatf& q2) noexcept
{
  return (q1.v.x * q2.v.x) + (q1.v.y * q2.v.y) + (q1.v.z * q2.v.z) + (q1.s * q2.s);
}
/**
 * @brief Rotational linear interpolation. Not accurate as slerp, but faster.
 */
inline Quatf rlerp(const Quatf& q1, const Quatf& q2, float t) noexcept
{
  float oneMinusT = 1.f - t;
  return normalized({lerp(q1.v, q2.v, t, oneMinusT), oneMinusT*q1.s + t*q2.s});
}
/**
 * @brief Spherical linear interpolation. More accurate than rlerp, but slower.
 */
inline Quatf slerp(const Quatf& q1, const Quatf& q2, float t) noexcept
{
  float theta = acos(dot(q1, q2));
  float wq1 = sin(1.f - t)*theta / sin(theta);
  float wq2 = sin(t)*theta / sin(theta);
  return wq1*q1 + wq2*q2;
}

/**
 * @brief Object that tracks it's target and moves on a sphere around it.
 */
class TrackSphere
{
public:
  /**
   * @param theta Radians to rotate in theta direction, which moves camera side to side.
   * @param phi Radians to rotate in phi direction, which tilts the camera forward and backward.
   * @param radius Distance from target.
   * @param radiusMin Minimum radius.
   * @param radiusMax Maximum radius.
  */
  TrackSphere(float theta, float phi, float radius, float radiusMin, float radiusMax) noexcept
    : theta(theta - Pi)
    , phi(std::clamp(phi, phiMin, phiMax))
    , radius(std::clamp(radius, radiusMin, radiusMax))
    , radiusMin(radiusMin)
    , radiusMax(radiusMax)
  {}

  Vec3f toCartesian(const Vec3f& target) const noexcept
  {
    return target + toCartesianLocal();
  }
  Vec3f toCartesianLocal() const noexcept
  {
    float x = radius * sinf(phi) * sinf(theta);
    float y = radius * cosf(phi);
    float z = radius * sinf(phi) * cosf(theta);

    return { x, y, z };
  }
  Mat4x3f calculateView(const Vec3f& target) const noexcept
  {
    return Mat4x3f::lookAt(toCartesian(target), target, { 0.f, 1.f, 0.f });
  }

  float getTheta() const noexcept { return theta; }
  /**
   * @param dTheta Radians to rotate in theta direction, which moves camera side to side.
   */
  void rotateTheta(float dTheta) noexcept
  {
    theta = clampAngle(theta + dTheta);
  }
  /**
   * @param dPhi Radians to rotate in phi direction, which tilts the camera forward and backward.
   */
  void rotatePhi(float dPhi) noexcept
  {
    phi = std::clamp(phi + dPhi, phiMin, phiMax);
  }
  /**
   * @param distance Distance to zoom in if positive, zoom out if negative.
   */
  void zoom(float distance) noexcept
  {
    radius -= distance;
    radius = std::clamp(radius, radiusMin, radiusMax);
  }

private:
  static constexpr float phiMin = degreesToRadians(0.001f); // Can't be exactly zero, otherwise shit happens
  static constexpr float phiMax = Pi - phiMin; // Can't be exactly Pi, otherwise shit happens

  float theta;
  float phi;
  float radius;
  float radiusMin;
  float radiusMax;
};

// View frustum without near and far planes as they are not always necessary.
class SimpleViewFrustum
{
public:
  SimpleViewFrustum() = default;
  explicit SimpleViewFrustum(const Mat4f& viewProjection);

  void set(const Mat4f& viewProjection);

  bool isPointInside(const Vec3f& point) const;
  bool isAabbInside(float xMin, float yMin, float zMin, float xMax, float yMax, float zMax) const;

private:

  // x,y,z is the plane normal vector towards inside the frustum, w is the normalized plane constant.
  Vec4f planes[4];
};