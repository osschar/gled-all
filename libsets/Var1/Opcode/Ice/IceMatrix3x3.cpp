//----------------------------------------------------------------------
/**
 *	Contains code for 3x3 matrices.
 *	\file		IceMatrix3x3.cpp
 *	\author		Pierre Terdiman
 *	\date		April, 4, 2000
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
/**
 *	3x3 matrix.
 *	DirectX-compliant, ie row-column order, ie m[Row][Col].
 *	Same as:
 *	m11  m12  m13  first row.
 *	m21  m22  m23  second row.
 *	m31  m32  m33  third row.
 *	Stored in memory as m11 m12 m13 m21...
 *
 *	Multiplication rules:
 *
 *	[x'y'z'] = [xyz][M]
 *
 *	x' = x*m11 + y*m21 + z*m31
 *	y' = x*m12 + y*m22 + z*m32
 *	z' = x*m13 + y*m23 + z*m33
 *
 *	\class		Matrix3x3
 *	\author		Pierre Terdiman
 *	\version	1.0
 *
 *      MT comment: This is what I'd call column major. The
 *      multiplication rules are strange, but that's what one gets
 *      with row-vectors.
 *      All left/right multiplications need to be substituted for
 *      right/left.
 *      See Point::operator*(Matrix3x3) and Matrix3x3::operator*(Point).
 */
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// Precompiled Header
#include "Opcode.h"

using namespace Opcode;

// Cast operator
Matrix3x3::operator Matrix4x4() const
{
  return Matrix4x4(m[0][0],  m[0][1],  m[0][2],  0.0f,
                   m[1][0],  m[1][1],  m[1][2],  0.0f,
                   m[2][0],  m[2][1],  m[2][2],  0.0f,
                   0.0f,     0.0f,     0.0f,     1.0f);
}

/*
 * A function for creating a rotation matrix that rotates a vector called
 * "from" into another vector called "to".
 * Input : from[3], to[3] which both must be *normalized* non-zero vectors
 * Output: mtx[3][3] -- a 3x3 matrix in colum-major form
 * Authors: Tomas Möller, John Hughes
 *          "Efficiently Building a Matrix to Rotate One Vector to Another"
 *          Journal of Graphics Tools, 4(4):1-4, 1999
 */

Matrix3x3& Matrix3x3::FromTo(const Point& from, const Point& to)
{
  static const float FROM_TO_EPSILON = 0.000001f;

  float e, f;

  e = from | to;
  f = (e < 0.0f) ? -e : e;
  if (f > 1.0f - FROM_TO_EPSILON) /* "from" and "to"-vector almost parallel */
  {
    Point u, v;       /* temporary storage vectors */
    Point x;          /* vector most nearly orthogonal to "from" */
    float c1, c2, c3; /* coefficients for later use */

    x.x = (from.x > 0.0f) ? from.x : -from.x;
    x.y = (from.y > 0.0f) ? from.y : -from.y;
    x.z = (from.z > 0.0f) ? from.z : -from.z;

    if (x.x < x.y)
    {
      if (x.x < x.z)
      {
        x.x = 1.0f; x.y = x.z = 0.0f;
      }
      else
      {
        x.z = 1.0f; x.x = x.y = 0.0f;
      }
    }
    else
    {
      if (x.y < x.z)
      {
        x.y = 1.0f; x.x = x.z = 0.0f;
      }
      else
      {
        x.z = 1.0f; x.x = x.y = 0.0f;
      }
    }

    u.Sub(x, from);
    v.Sub(x, to);

    c1 = 2.0f / u.SquareMagnitude();
    c2 = 2.0f / v.SquareMagnitude();
    c3 = c1 * c2  * u.Dot(v);

    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        m[j][i] =  - c1 * u[i] * u[j]
                   - c2 * v[i] * v[j]
                   + c3 * v[i] * u[j];
      }
      m[i][i] += 1.0;
    }
  }
  else  /* the most common case, unless "from"="to", or "from"=-"to" */
  {
    Float_t h;
    Point   v;
    v.Cross(from, to);

#if 0
    /* unoptimized version - a good compiler will optimize this. */
    /* h = (1.0 - e)/DOT(v, v); old code */
    h = 1.0/(1.0 + e);      /* optimization by Gottfried Chen */
    m[0][0] = e + h * v.x * v.x;
    m[1][0] = h * v.x * v.y - v.z;
    m[2][0] = h * v.x * v.z + v.y;

    m[0][1] = h * v.x * v.y + v.z;
    m[1][1] = e + h * v.y * v.y;
    m[2][1] = h * v.y * v.z - v.x;

    m[0][2] = h * v.x * v.z - v.y;
    m[1][2] = h * v.y * v.z + v.x;
    m[2][2] = e + h * v.z * v.z;
#else
    /* ...otherwise use this hand optimized version (9 mults less) */
    float hvx, hvz, hvxy, hvxz, hvyz;
    /* h = (1.0 - e)/DOT(v, v); old code */
    h = 1.0f/(1.0f + e);      /* optimization by Gottfried Chen */
    hvx = h * v.x;
    hvz = h * v.z;
    hvxy = hvx * v.y;
    hvxz = hvx * v.z;
    hvyz = hvz * v.y;
    m[0][0] = e + hvx * v.x;
    m[1][0] = hvxy - v.z;
    m[2][0] = hvxz + v.y;

    m[0][1] = hvxy + v.z;
    m[1][1] = e + h * v.y * v.y;
    m[2][1] = hvyz - v.x;

    m[0][2] = hvxz - v.y;
    m[1][2] = hvyz + v.x;
    m[2][2] = e + hvz * v.z;
#endif
  }

  return *this;
}
