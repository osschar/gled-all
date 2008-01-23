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
