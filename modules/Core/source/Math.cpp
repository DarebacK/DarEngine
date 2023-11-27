#include "Core/Math.hpp"

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

SimpleViewFrustum::SimpleViewFrustum(const Mat4f & viewProjection)
{
  set(viewProjection);
}
void SimpleViewFrustum::set(const Mat4f& viewProjection)
{
  // For the math explanation see https://www.braynzarsoft.net/viewtutorial/q16390-34-aabb-cpu-side-frustum-culling, 
  // https://www.rastertek.com/dx11win10tut23.html and https://www.flipcode.com/archives/Frustum_Culling.shtml
  // This is also interesting https://gamedev.net/forums/topic/676255-determine-whenever-box-is-visible-in-frustum/5277726/?page=1

  // Left Frustum Plane
  planes[0].x = viewProjection[0][3] + viewProjection[0][0];
  planes[0].y = viewProjection[1][3] + viewProjection[1][0];
  planes[0].z = viewProjection[2][3] + viewProjection[2][0];
  planes[0].w = viewProjection[3][3] + viewProjection[3][0];
  float planeNormalLength = length(Vec3f(planes[0].x, planes[0].y, planes[0].z));
  planes[0] /= planeNormalLength;

  // Right Frustum Plane
  planes[1].x = viewProjection[0][3] - viewProjection[0][0];
  planes[1].y = viewProjection[1][3] - viewProjection[1][0];
  planes[1].z = viewProjection[2][3] - viewProjection[2][0];
  planes[1].w = viewProjection[3][3] - viewProjection[3][0];
  planeNormalLength = length(Vec3f(planes[1].x, planes[1].y, planes[1].z));
  planes[1] /= planeNormalLength;

  // Top Frustum Plane
  planes[2].x = viewProjection[0][3] - viewProjection[0][1];
  planes[2].y = viewProjection[1][3] - viewProjection[1][1];
  planes[2].z = viewProjection[2][3] - viewProjection[2][1];
  planes[2].w = viewProjection[3][3] - viewProjection[3][1];
  planeNormalLength = length(Vec3f(planes[2].x, planes[2].y, planes[2].z));
  planes[2] /= planeNormalLength;

  // Bottom Frustum Plane
  planes[3].x = viewProjection[0][3] + viewProjection[0][1];
  planes[3].y = viewProjection[1][3] + viewProjection[1][1];
  planes[3].z = viewProjection[2][3] + viewProjection[2][1];
  planes[3].w = viewProjection[3][3] + viewProjection[3][1];
  planeNormalLength = length(Vec3f(planes[3].x, planes[3].y, planes[3].z));
  planes[3] /= planeNormalLength;

  // Near Frustum Plane
  // We could add the third column to the fourth column to get the near plane,
  // but we don't have to do this because the third column IS the near plane
  //planes[4].x = viewProjection[0][2];
  //planes[4].y = viewProjection[1][2];
  //planes[4].z = viewProjection[2][2];
  //planes[4].w = viewProjection[3][2];
  //planeNormalLength = length(Vec3f(planes[4].x, planes[4].y, planes[4].z));
  //planes[4] /= planeNormalLength;

  // Far Frustum Plane
  //planes[5].x = viewProjection[0][3] - viewProjection[0][2];
  //planes[5].y = viewProjection[1][3] - viewProjection[1][2];
  //planes[5].z = viewProjection[2][3] - viewProjection[2][2];
  //planes[5].w = viewProjection[3][3] - viewProjection[3][2];
  //planeNormalLength = length(Vec3f(planes[5].x, planes[5].y, planes[5].z));
  //planes[5] /= planeNormalLength;
}
bool SimpleViewFrustum::isPointInside(const Vec3f& point) const
{
  // TODO: vectorize

  for(const Vec4f& plane : planes)
  {
    const float distance = (plane.x * point.x) + (plane.y * point.y) + (plane.z * point.z) + plane.w;
    if(distance < 0.f)
    {
      return false;
    }
  }

  return true;
}
bool SimpleViewFrustum::isAabbInside(float xMin, float yMin, float zMin, float xMax, float yMax, float zMax) const
{
  // TODO: vectorize

  for(const Vec4f& plane : planes)
  {
    // AABB vertex that is furthest in the direction of the plane normal vector.
    Vec3f furthestVertex;
    furthestVertex.x = plane.x < 0.f ? xMin : xMax;
    furthestVertex.y = plane.y < 0.f ? yMin : yMax;
    furthestVertex.z = plane.z < 0.f ? zMin : zMax;

    const float distance = dot(toVec3f(plane), furthestVertex) + plane.w;
    if(distance < 0.f)
    {
      return false;
    }
  }

  return true;
}