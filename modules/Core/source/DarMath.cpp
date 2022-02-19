#include "DarMath.hpp"

Vec3f operator*(const Vec3f& left, const Mat3f& right) noexcept
{
  return
  {
    left.x*right[0][0] + left.y*right[1][0] + left.z*right[2][0],
    left.x*right[0][1] + left.y*right[1][1] + left.z*right[2][1],
    left.x*right[0][2] + left.y*right[1][2] + left.z*right[2][2]
  };
}
Vec4f operator*(const Vec4f& left, const Mat4f& right) noexcept
{
  return
  {
    left.x*right[0][0] + left.y*right[1][0] + left.z*right[2][0] + left.w*right[3][0],
    left.x*right[0][1] + left.y*right[1][1] + left.z*right[2][1] + left.w*right[3][1],
    left.x*right[0][2] + left.y*right[1][2] + left.z*right[2][2] + left.w*right[3][2],
    left.x*right[0][3] + left.y*right[1][3] + left.z*right[2][3] + left.w*right[3][3],
  };
}
Vec4f operator*(const Vec4f& left, const Mat4x3f& right) noexcept
{
  return
  {
    left.x*right[0][0] + left.y*right[1][0] + left.z*right[2][0] + left.w*right[3][0],
    left.x*right[0][1] + left.y*right[1][1] + left.z*right[2][1] + left.w*right[3][1],
    left.x*right[0][2] + left.y*right[1][2] + left.z*right[2][2] + left.w*right[3][2],
    left.w,
  };
}
Mat3f operator*(const Mat3f& left, const Mat3f& right) noexcept
{
  return
  {
    left[0][0] * right[0][0] + left[0][1] * right[1][0] + left[0][2] * right[2][0],
    left[0][0] * right[0][1] + left[0][1] * right[1][1] + left[0][2] * right[2][1],
    left[0][0] * right[0][2] + left[0][1] * right[1][2] + left[0][2] * right[2][2],

    left[1][0] * right[0][0] + left[1][1] * right[1][0] + left[1][2] * right[2][0],
    left[1][0] * right[0][1] + left[1][1] * right[1][1] + left[1][2] * right[2][1],
    left[1][0] * right[0][2] + left[1][1] * right[1][2] + left[1][2] * right[2][2],

    left[2][0] * right[0][0] + left[2][1] * right[1][0] + left[2][2] * right[2][0],
    left[2][0] * right[0][1] + left[2][1] * right[1][1] + left[2][2] * right[2][1],
    left[2][0] * right[0][2] + left[2][1] * right[1][2] + left[2][2] * right[2][2]
  };
}
Mat4x3f operator*(const Mat3f& left, const Mat4x3f& right) noexcept
{
  return
  {
    left[0][0]*right[0][0] + left[0][1]*right[1][0] + left[0][2]*right[2][0],
    left[0][0]*right[0][1] + left[0][1]*right[1][1] + left[0][2]*right[2][1],
    left[0][0]*right[0][2] + left[0][1]*right[1][2] + left[0][2]*right[2][2],

    left[1][0]*right[0][0] + left[1][1]*right[1][0] + left[1][2]*right[2][0],
    left[1][0]*right[0][1] + left[1][1]*right[1][1] + left[1][2]*right[2][1],
    left[1][0]*right[0][2] + left[1][1]*right[1][2] + left[1][2]*right[2][2],

    left[2][0]*right[0][0] + left[2][1]*right[1][0] + left[2][2]*right[2][0],
    left[2][0]*right[0][1] + left[2][1]*right[1][1] + left[2][2]*right[2][1],
    left[2][0]*right[0][2] + left[2][1]*right[1][2] + left[2][2]*right[2][2],

    right[3][0],
    right[3][1],
    right[3][2],
  };
}
Mat4x3f operator*(const Mat4x3f& left, const Mat3f& right) noexcept
{
  return
  {
    left[0][0]*right[0][0] + left[0][1]*right[1][0] + left[0][2]*right[2][0],
    left[0][0]*right[0][1] + left[0][1]*right[1][1] + left[0][2]*right[2][1],
    left[0][0]*right[0][2] + left[0][1]*right[1][2] + left[0][2]*right[2][2],

    left[1][0]*right[0][0] + left[1][1]*right[1][0] + left[1][2]*right[2][0],
    left[1][0]*right[0][1] + left[1][1]*right[1][1] + left[1][2]*right[2][1],
    left[1][0]*right[0][2] + left[1][1]*right[1][2] + left[1][2]*right[2][2],

    left[2][0]*right[0][0] + left[2][1]*right[1][0] + left[2][2]*right[2][0],
    left[2][0]*right[0][1] + left[2][1]*right[1][1] + left[2][2]*right[2][1],
    left[2][0]*right[0][2] + left[2][1]*right[1][2] + left[2][2]*right[2][2],

    left[3][0]*right[0][0] + left[3][1]*right[1][0] + left[3][2]*right[2][0],
    left[3][0]*right[0][1] + left[3][1]*right[1][1] + left[3][2]*right[2][1],
    left[3][0]*right[0][2] + left[3][1]*right[1][2] + left[3][2]*right[2][2]
  };
}
Mat4x3f operator*(const Mat4x3f& left, const Mat4x3f& right) noexcept
{
  return
  {
    left[0][0]*right[0][0] + left[0][1]*right[1][0] + left[0][2]*right[2][0],
    left[0][0]*right[0][1] + left[0][1]*right[1][1] + left[0][2]*right[2][1],
    left[0][0]*right[0][2] + left[0][1]*right[1][2] + left[0][2]*right[2][2],

    left[1][0]*right[0][0] + left[1][1]*right[1][0] + left[1][2]*right[2][0],
    left[1][0]*right[0][1] + left[1][1]*right[1][1] + left[1][2]*right[2][1],
    left[1][0]*right[0][2] + left[1][1]*right[1][2] + left[1][2]*right[2][2],

    left[2][0]*right[0][0] + left[2][1]*right[1][0] + left[2][2]*right[2][0],
    left[2][0]*right[0][1] + left[2][1]*right[1][1] + left[2][2]*right[2][1],
    left[2][0]*right[0][2] + left[2][1]*right[1][2] + left[2][2]*right[2][2],

    left[3][0]*right[0][0] + left[3][1]*right[1][0] + left[3][2]*right[2][0] + right[3][0],
    left[3][0]*right[0][1] + left[3][1]*right[1][1] + left[3][2]*right[2][1] + right[3][1],
    left[3][0]*right[0][2] + left[3][1]*right[1][2] + left[3][2]*right[2][2] + right[3][2]
  };
}
Mat4f operator*(const Mat4x3f& left, const Mat4f& right) noexcept
{
  return
  {
    left[0][0]*right[0][0] + left[0][1]*right[1][0] + left[0][2]*right[2][0],
    left[0][0]*right[0][1] + left[0][1]*right[1][1] + left[0][2]*right[2][1],
    left[0][0]*right[0][2] + left[0][1]*right[1][2] + left[0][2]*right[2][2],
    left[0][0]*right[0][3] + left[0][1]*right[1][3] + left[0][2]*right[2][3],

    left[1][0]*right[0][0] + left[1][1]*right[1][0] + left[1][2]*right[2][0],
    left[1][0]*right[0][1] + left[1][1]*right[1][1] + left[1][2]*right[2][1],
    left[1][0]*right[0][2] + left[1][1]*right[1][2] + left[1][2]*right[2][2],
    left[1][0]*right[0][3] + left[1][1]*right[1][3] + left[1][2]*right[2][3],

    left[2][0]*right[0][0] + left[2][1]*right[1][0] + left[2][2]*right[2][0],
    left[2][0]*right[0][1] + left[2][1]*right[1][1] + left[2][2]*right[2][1],
    left[2][0]*right[0][2] + left[2][1]*right[1][2] + left[2][2]*right[2][2],
    left[2][0]*right[0][3] + left[2][1]*right[1][3] + left[2][2]*right[2][3],

    left[3][0]*right[0][0] + left[3][1]*right[1][0] + left[3][2]*right[2][0] + right[3][0],
    left[3][0]*right[0][1] + left[3][1]*right[1][1] + left[3][2]*right[2][1] + right[3][1],
    left[3][0]*right[0][2] + left[3][1]*right[1][2] + left[3][2]*right[2][2] + right[3][2],
    left[3][0]*right[0][3] + left[3][1]*right[1][3] + left[3][2]*right[2][3] + right[3][3]
  };
}
Mat4f operator*(const Mat4f& left, const Mat4x3f& right) noexcept
{
  return
  {
    left[0][0]*right[0][0] + left[0][1]*right[1][0] + left[0][2]*right[2][0] + left[0][3]*right[3][0],
    left[0][0]*right[0][1] + left[0][1]*right[1][1] + left[0][2]*right[2][1] + left[0][3]*right[3][1],
    left[0][0]*right[0][2] + left[0][1]*right[1][2] + left[0][2]*right[2][2] + left[0][3]*right[3][2],
    left[0][3],

    left[1][0]*right[0][0] + left[1][1]*right[1][0] + left[1][2]*right[2][0] + left[1][3]*right[3][0],
    left[1][0]*right[0][1] + left[1][1]*right[1][1] + left[1][2]*right[2][1] + left[1][3]*right[3][1],
    left[1][0]*right[0][2] + left[1][1]*right[1][2] + left[1][2]*right[2][2] + left[1][3]*right[3][2],
    left[1][3],

    left[2][0]*right[0][0] + left[2][1]*right[1][0] + left[2][2]*right[2][0] + left[2][3]*right[3][0],
    left[2][0]*right[0][1] + left[2][1]*right[1][1] + left[2][2]*right[2][1] + left[2][3]*right[3][1],
    left[2][0]*right[0][2] + left[2][1]*right[1][2] + left[2][2]*right[2][2] + left[2][3]*right[3][2],
    left[2][3],

    left[3][0]*right[0][0] + left[3][1]*right[1][0] + left[3][2]*right[2][0] + left[3][3]*right[3][0],
    left[3][0]*right[0][1] + left[3][1]*right[1][1] + left[3][2]*right[2][1] + left[3][3]*right[3][1],
    left[3][0]*right[0][2] + left[3][1]*right[1][2] + left[3][2]*right[2][2] + left[3][3]*right[3][2],
    left[3][3]
  };
}
Mat4f operator*(const Mat4f& left, const Mat4f& right) noexcept
{
  return
  {
    left[0][0]*right[0][0] + left[0][1]*right[1][0] + left[0][2]*right[2][0] + left[0][3]*right[3][0],
    left[0][0]*right[0][1] + left[0][1]*right[1][1] + left[0][2]*right[2][1] + left[0][3]*right[3][1],
    left[0][0]*right[0][2] + left[0][1]*right[1][2] + left[0][2]*right[2][2] + left[0][3]*right[3][2],
    left[0][0]*right[0][3] + left[0][1]*right[1][3] + left[0][2]*right[2][3] + left[0][3]*right[3][3],

    left[1][0]*right[0][0] + left[1][1]*right[1][0] + left[1][2]*right[2][0] + left[1][3]*right[3][0],
    left[1][0]*right[0][1] + left[1][1]*right[1][1] + left[1][2]*right[2][1] + left[1][3]*right[3][1],
    left[1][0]*right[0][2] + left[1][1]*right[1][2] + left[1][2]*right[2][2] + left[1][3]*right[3][2],
    left[1][0]*right[0][3] + left[1][1]*right[1][3] + left[1][2]*right[2][3] + left[1][3]*right[3][3],

    left[2][0]*right[0][0] + left[2][1]*right[1][0] + left[2][2]*right[2][0] + left[2][3]*right[3][0],
    left[2][0]*right[0][1] + left[2][1]*right[1][1] + left[2][2]*right[2][1] + left[2][3]*right[3][1],
    left[2][0]*right[0][2] + left[2][1]*right[1][2] + left[2][2]*right[2][2] + left[2][3]*right[3][2],
    left[2][0]*right[0][3] + left[2][1]*right[1][3] + left[2][2]*right[2][3] + left[2][3]*right[3][3],

    left[3][0]*right[0][0] + left[3][1]*right[1][0] + left[3][2]*right[2][0] + left[3][3]*right[3][0],
    left[3][0]*right[0][1] + left[3][1]*right[1][1] + left[3][2]*right[2][1] + left[3][3]*right[3][1],
    left[3][0]*right[0][2] + left[3][1]*right[1][2] + left[3][2]*right[2][2] + left[3][3]*right[3][2],
    left[3][0]*right[0][3] + left[3][1]*right[1][3] + left[3][2]*right[2][3] + left[3][3]*right[3][3]
  };
}