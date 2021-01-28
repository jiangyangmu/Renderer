#pragma once

#include <cmath>
#include <cfloat> // FLT_EPSILON
#include <cinttypes>
#include <cstring> // memcmp

namespace Graphics
{
	// --------------------------------------------------------------------------
	// Types
	// --------------------------------------------------------------------------

	typedef uint8_t		byte;
	typedef int32_t		i32;
	typedef uint32_t	u32;
	typedef float		f32;

	// --------------------------------------------------------------------------
	// Constants & Routines
	// --------------------------------------------------------------------------

	constexpr f32		C_PI		= 3.14159265358979323846;
	constexpr f32		C_2PI		= 3.14159265358979323846 * 2.0;
	constexpr f32		C_1DIVPI	= 0.318309886183790671538;
	constexpr f32		C_1DIV2PI	= 0.318309886183790671538 * 0.5;
	constexpr f32		C_F32_EPSILON	= FLT_EPSILON;

	inline bool		IsNearZero(f32 value)
	{
		return -C_F32_EPSILON <= value && value <= C_F32_EPSILON;
	}

	// --------------------------------------------------------------------------
	// Structures
	// --------------------------------------------------------------------------

	struct Point3
	{
		f32 x, y, z;
	};
	struct Vector2
	{
		f32 x, y;
	};
	struct Vector3
	{
		f32 x, y, z;

		inline f32	Length() const
		{
			return sqrtf(x * x + y * y + z * z);
		}
		inline void	Normalize()
		{
			float lenReciprocal = 1.0f / Length();
			x *= lenReciprocal;
			y *= lenReciprocal;
			z *= lenReciprocal;
		}
	};
	struct Vector4
	{
		union
		{
			Vector3 v3;
			Vector3 xyz;
			struct
			{
				f32 x, y, z;
			};
		};
		f32 w;
	};
	struct Matrix44
	{
		union
		{
			Vector4 r[4];
			struct
			{
				f32 _11;
				f32 _12;
				f32 _13;
				f32 _14;
				f32 _21;
				f32 _22;
				f32 _23;
				f32 _24;
				f32 _31;
				f32 _32;
				f32 _33;
				f32 _34;
				f32 _41;
				f32 _42;
				f32 _43;
				f32 _44;
			};
			f32 m[4][4];
		};

	};

	// --------------------------------------------------------------------------
	// Operators
	// --------------------------------------------------------------------------

	inline Vector3		operator - (const Vector3 & v)
	{
		return { -v.x, -v.y, -v.z };
	}
	inline Vector3		operator + (const Vector3 & v0, const Vector3 & v1)
	{
		return { v0.x + v1.x, v0.y + v1.y, v0.z + v1.z };
	}
	inline Vector3		operator - (const Vector3 & v0, const Vector3 & v1)
	{
		return { v0.x - v1.x, v0.y - v1.y, v0.z - v1.z };
	}

	inline Matrix44		operator + (const Matrix44 & m0, const Matrix44 & m1)
	{
		Matrix44 m =
		{
			m0._11 + m1._11, m0._12 + m1._12, m0._13 + m1._13, m0._14 + m1._14,
			m0._21 + m1._21, m0._22 + m1._22, m0._23 + m1._23, m0._24 + m1._24,
			m0._31 + m1._31, m0._32 + m1._32, m0._33 + m1._33, m0._34 + m1._34,
			m0._41 + m1._41, m0._42 + m1._42, m0._43 + m1._43, m0._44 + m1._44,
		};
		return m;
	}
	inline Matrix44		operator - (const Matrix44 & m0, const Matrix44 & m1)
	{
		Matrix44 m =
		{
			m0._11 - m1._11, m0._12 - m1._12, m0._13 - m1._13, m0._14 - m1._14,
			m0._21 - m1._21, m0._22 - m1._22, m0._23 - m1._23, m0._24 - m1._24,
			m0._31 - m1._31, m0._32 - m1._32, m0._33 - m1._33, m0._34 - m1._34,
			m0._41 - m1._41, m0._42 - m1._42, m0._43 - m1._43, m0._44 - m1._44,
		};
		return m;
	}
	inline Matrix44		operator * (const Matrix44 & m0, const Matrix44 & m1)
	{
		Matrix44 m;

		for ( int row = 0; row < 4; ++row )
		{
			for ( int col = 0; col < 4; ++col )
			{
				m.m[ row ][ col ] =
					m0.m[ row ][ 0 ] * m1.m[ 0 ][ col ] +
					m0.m[ row ][ 1 ] * m1.m[ 1 ][ col ] +
					m0.m[ row ][ 2 ] * m1.m[ 2 ][ col ] +
					m0.m[ row ][ 3 ] * m1.m[ 3 ][ col ];
			}
		}

		return m;
	}
	inline Matrix44		operator * (const Matrix44 & m, f32 f)
	{
		Matrix44 ms =
		{
			f * m._11, f * m._12, f * m._13, f * m._14,
			f * m._21, f * m._22, f * m._23, f * m._24,
			f * m._31, f * m._32, f * m._33, f * m._34,
			f * m._41, f * m._42, f * m._43, f * m._44,
		};
		return ms;
	}

	// --------------------------------------------------------------------------
	// Functions
	// --------------------------------------------------------------------------

	// Helpers

	template <typename T>
	inline T		Bound(T min, T value, T max)
	{
		return value < min ? min : ( value > max ? max : value );
	}
	template <typename T>
	inline T		Max(T a, T b)
	{
		return a >= b ? a : b;
	}
	template <typename T>
	inline T		Min(T a, T b)
	{
		return a <= b ? a : b;
	}
	template <typename T>
	inline T		AlignFloor(T value, T alignment)
	{
		// value > 0, alignment > 0
		return value - ( value % alignment );
	}
	template <typename T>
	inline T		AlignCeiling(T value, T alignment)
	{
		// value > 0, alignment > 0
		T m = value % alignment;
		return m > 0 ? ( value - m + alignment ) : value;
	}

	inline f32		Min3(f32 a, f32 b, f32 c)
	{
		return	a < b
			? ( a < c ? a : c )
			: ( b < c ? b : c );
	}
	inline f32		Max3(f32 a, f32 b, f32 c)
	{
		return	a > b
			? ( a > c ? a : c )
			: ( b > c ? b : c );
	}
	inline f32		EdgeFunction(const Vector2 & a, const Vector2 & b, const Vector2 & c)
	{
		return ( c.x - a.x ) * ( b.y - a.y ) - ( c.y - a.y ) * ( b.x - a.x );
	}
	inline Vector2		WeightedAdd(const Vector2 & v0, const Vector2 & v1, const Vector2 & v2, f32 w0, f32 w1, f32 w2)
	{
		return
		{
			v0.x * w0 + v1.x * w1 + v2.x * w2,
			v0.y * w0 + v1.y * w1 + v2.y * w2,
		};
	}
	inline Vector3		WeightedAdd(const Vector3 & v0, const Vector3 & v1, const Vector3 & v2, f32 w0, f32 w1, f32 w2)
	{
		return
		{
			v0.x * w0 + v1.x * w1 + v2.x * w2,
			v0.y * w0 + v1.y * w1 + v2.y * w2,
			v0.z * w0 + v1.z * w1 + v2.z * w2,
		};
	}

	// Point3
	inline f32		P3Distance(Point3 p0, Point3 p1)
	{
		return sqrtf(( p1.x - p0.x ) * ( p1.x - p0.x ) + ( p1.y - p0.y ) * ( p1.y - p0.y ) + ( p1.z - p0.z ) * ( p1.z - p0.z ));
	}

	// Vector3
	inline Vector3		V3Zero()
	{
		Vector3 v = { 0.0f, 0.0f, 0.0f };
		return v;
	}
	inline Vector3		V3One()
	{
		Vector3 v = { 1.0f, 1.0f, 1.0f };
		return v;
	}
	inline Vector3		V3UnitX()
	{
		Vector3 v = { 1.0f, 0.0f, 0.0f };
		return v;
	}
	inline Vector3		V3UnitY()
	{
		Vector3 v = { 0.0f, 1.0f, 0.0f };
		return v;
	}
	inline Vector3		V3UnitZ()
	{
		Vector3 v = { 0.0f, 0.0f, 1.0f };
		return v;
	}
	inline f32		V3Length(Vector3 v)
	{
		return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	}
	inline f32		V3ReciprocalLength(Vector3 v)
	{
		return 1.0f / sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	}
	inline Vector3		V3Scale(Vector3 v, f32 fScale)
	{
		return { v.x * fScale, v.y * fScale, v.z * fScale };
	}
	inline Vector3		V3Normalize(Vector3 v)
	{
		return V3Scale(v, V3ReciprocalLength(v));
	}
	inline f32		V3Dot(Vector3 v0, Vector3 v1)
	{
		return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
	}
	inline Vector3		V3CrossLH(Vector3 v0, Vector3 v1)
	{
		return
		{
			v0.y * v1.z - v0.z * v1.y,
			v0.z * v1.x - v0.x * v1.z,
			v0.x * v1.y - v0.y * v1.x
		};
	}
	inline Vector3		V3ElementwiseProduct(Vector3 v0, Vector3 v1)
	{
		return
		{
			v0.x * v1.x,
			v0.y * v1.y,
			v0.z * v1.z
		};
	}
	inline Vector3		V3Lerp(Vector3 v0, Vector3 v1, f32 t)
	{
		return
		{
			( 1.0f - t ) * v0.x + t * v1.x,
			( 1.0f - t ) * v0.y + t * v1.y,
			( 1.0f - t ) * v0.z + t * v1.z,
		};
	}
	inline Vector3		V3LerpV(Vector3 v0, Vector3 v1, Vector3 vT)
	{
		return
		{
			( 1.0f - vT.x ) * v0.x + vT.x * v1.x,
			( 1.0f - vT.y ) * v0.y + vT.y * v1.y,
			( 1.0f - vT.z ) * v0.z + vT.z * v1.z,
		};
	}
	inline Vector3		V3Reflect(Vector3 incident, Vector3 normal);
	inline Vector3		V3Refract(Vector3 incident, Vector3 normal, f32 refractionIndex);
	inline f32		V3AngleBetweenVectors(Vector3 v0, Vector3 v1);
	inline Vector3		V3Transform(Vector3 v, const Matrix44 & m)
	{
		f32 w = m._14 * v.x + m._24 * v.y + m._34 * v.z + m._44;
		f32 wReciprocal = ( fabsf(w) < 1e-6f ? 1e6f : 1.0f / w );
		return
		{
			( m._11 * v.x + m._21 * v.y + m._31 * v.z + m._41 ) * wReciprocal,
			( m._12 * v.x + m._22 * v.y + m._32 * v.z + m._42 ) * wReciprocal,
			( m._13 * v.x + m._23 * v.y + m._33 * v.z + m._43 ) * wReciprocal,
		};
	}

	// Matrix44
	inline Matrix44		M44Zero()
	{
		Matrix44 m =
		{
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 0.0f,
		};
		return m;
	}
	inline Matrix44		M44Identity()
	{
		Matrix44 m =
		{
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f,
		};
		return m;
	}

	// Matrix44 - Test
	inline bool		M44IsIdentity(const Matrix44 & m)
	{
		Matrix44 m1 = M44Identity();
		return memcmp(&m1, &m, sizeof(Matrix44)) == 0;
	}
	inline bool		M44IsInfinite(const Matrix44 & m)
	{
		const f32 * p = &m._11;
		const f32 * pEnd = &m._11;

		do
		{
			if ( isinf(*p) )
			{
				return true;
			}
		}
		while ( ++p != pEnd );

		return false;
	}
	inline bool		M44IsNaN(const Matrix44 & m)
	{
		const f32 * p = &m._11;
		const f32 * pEnd = &m._11;

		do
		{
			if ( isnan(*p) )
			{
				return true;
			}
		}
		while ( ++p != pEnd );

		return false;
	}

	// Matrix44 - Matrix Operation
	inline Matrix44		M44Multiply(const Matrix44 & m0, const Matrix44 & m1)
	{
		Matrix44 m;

		for ( int row = 0; row < 4; ++row )
		{
			for ( int col = 0; col < 4; ++col )
			{
				m.m[ row ][ col ] =
					m0.m[ row ][ 0 ] * m1.m[ 0 ][ col ] +
					m0.m[ row ][ 1 ] * m1.m[ 1 ][ col ] +
					m0.m[ row ][ 2 ] * m1.m[ 2 ][ col ] +
					m0.m[ row ][ 3 ] * m1.m[ 3 ][ col ];
			}
		}

		return m;
	}
	inline Matrix44		M44MultiplyScalar(const Matrix44 & m, f32 f)
	{
		Matrix44 ms =
		{
			f * m._11, f * m._12, f * m._13, f * m._14,
			f * m._21, f * m._22, f * m._23, f * m._24,
			f * m._31, f * m._32, f * m._33, f * m._34,
			f * m._41, f * m._42, f * m._43, f * m._44,
		};
		return ms;
	}
	inline Matrix44		M44Transpose(const Matrix44 & m)
	{
		Matrix44 mt =
		{
			m._11, m._21, m._31, m._41,
			m._12, m._22, m._32, m._42,
			m._13, m._23, m._33, m._43,
			m._14, m._24, m._34, m._44,
		};
		return mt;
	}
	inline f32		M44Determinant(const Matrix44 & m)
	{
		f32 det1
			= m._22 * m._33 * m._44
			+ m._23 * m._34 * m._42
			+ m._24 * m._32 * m._43
			- m._22 * m._34 * m._43
			- m._23 * m._32 * m._44
			- m._24 * m._33 * m._42;

		f32 det2
			= m._21 * m._33 * m._44
			+ m._23 * m._34 * m._41
			+ m._24 * m._31 * m._43
			- m._21 * m._34 * m._43
			- m._23 * m._31 * m._44
			- m._24 * m._33 * m._41;

		f32 det3
			= m._21 * m._32 * m._44
			+ m._22 * m._34 * m._41
			+ m._24 * m._31 * m._42
			- m._21 * m._34 * m._42
			- m._22 * m._31 * m._44
			- m._24 * m._32 * m._41;

		f32 det4
			= m._21 * m._32 * m._43
			+ m._22 * m._33 * m._41
			+ m._23 * m._31 * m._42
			- m._21 * m._33 * m._42
			- m._22 * m._31 * m._43
			- m._23 * m._32 * m._41;

		return m._11 * det1 - m._12 * det2 + m._13 * det3 - m._14 * det4;
	}
	inline f32		M44Trace(const Matrix44 & m)
	{
		return m._11 + m._22 + m._33 + m._44;
	}
	inline bool		__M44Inverse_CayleyHamilton(Matrix44 * pmInv, f32 * pDeterminant, const Matrix44 & m)
	{
		f32 det;
		f32 tr;
		f32 tr2;
		f32 tr3;
		Matrix44 m2;
		Matrix44 m3;
		
		det = M44Determinant(m);

		if (IsNearZero(det))
		{
			return false;
		}
		else
		{
			m2 = M44Multiply(m, m);
			m3 = M44Multiply(m2, m);
			tr = M44Trace(m);
			tr2 = M44Trace(m2);
			tr3 = M44Trace(m3);

			*pmInv =
				( M44Identity() * ((tr * tr * tr - 3 * tr * tr2 + 2 * tr3) / 6.0f)
				 - m * ((tr * tr - tr2) * 0.5f)
				 + m2 * tr
				 - m3
				 )
				* (1.0f / det);

			if (pDeterminant)
			{
				*pDeterminant = det;
			}

			return true;
		}
	}
	inline Matrix44		M44Inverse(f32 * pDeterminant, const Matrix44 & m)
	{
		Matrix44 mInv;

		if (!__M44Inverse_CayleyHamilton(&mInv, pDeterminant, m))
		{
			mInv = M44Zero();
		}

		return mInv;
	}

	// Matrix44 - Projection
	inline Matrix44		M44OrthographicLH(f32 fViewWidth, f32 fViewHeight, f32 fNearZ, f32 fFarZ);
	inline Matrix44		M44OrthographicRH(f32 fViewWidth, f32 fViewHeight, f32 fNearZ, f32 fFarZ);
	inline Matrix44		M44OrthographicOffCenterLH(f32 fViewLeft, f32 fViewRight, f32 fViewBottom, f32 fViewTop, f32 fNearZ, f32 fFarZ);
	inline Matrix44		M44OrthographicOffCenterRH(f32 fViewLeft, f32 fViewRight, f32 fViewBottom, f32 fViewTop, f32 fNearZ, f32 fFarZ);
	inline Matrix44		M44PerspectiveFovLH(f32 fFovAngleY, f32 fAspectRatio, f32 fNearZ, f32 fFarZ)
	{
		f32 height = 1.0f / tanf(0.5f * fFovAngleY);
		f32 width = height / fAspectRatio;
		f32 fRange = fFarZ / ( fFarZ - fNearZ );

		Matrix44 m =
		{
			width, 0.0f, 0.0f, 0.0f,
			0.0f, height, 0.0f, 0.0f,
			0.0f, 0.0f, fRange, 1.0f,
			0.0f, 0.0f, -fRange * fNearZ, 0.0f
		};

		return m;
	}
	inline Matrix44		M44PerspectiveFovRH(f32 fFovAngleY, f32 fAspectRatio, f32 fNearZ, f32 fFarZ);
	inline Matrix44		M44PerspectiveLH(f32 fViewWidth, f32 fViewHeight, f32 fNearZ, f32 fFarZ);
	inline Matrix44		M44PerspectiveRH(f32 fViewWidth, f32 fViewHeight, f32 fNearZ, f32 fFarZ);
	inline Matrix44		M44PerspectiveOffCenterLH(f32 fViewLeft, f32 fViewRight, f32 fViewBottom, f32 fViewTop, f32 fNearZ, f32 fFarZ);
	inline Matrix44		M44PerspectiveOffCenterRH(f32 fViewLeft, f32 fViewRight, f32 fViewBottom, f32 fViewTop, f32 fNearZ, f32 fFarZ);

	// Matrix44 - Transform
	inline Matrix44		M44LookAtLH(Vector3 vEyePosition, Vector3 vFocusPosition, Vector3 vUpDirection);
	inline Matrix44		M44LookAtRH(Vector3 vEyePosition, Vector3 vFocusPosition, Vector3 vUpDirection);
	inline Matrix44		M44LookToLH(Vector3 vEyePosition, Vector3 vEyeDirection, Vector3 vUpDirection);
	inline Matrix44		M44LookToRH(Vector3 vEyePosition, Vector3 vEyeDirection, Vector3 vUpDirection);
	
	inline Matrix44		M44AffineTransformation(Vector3 vScaling, Vector3 vRotationOrigin, Vector4 vRotationQuaternion, Vector3 vTranslation);
	inline Matrix44		M44Transformation(Vector3 vScalingOrigin, Vector4 vScalingOrientationQuaternion, Vector3 vScaling, Vector3 vRotationOrigin, Vector4 vRotationQuaternion, Vector3 vTranslation);
	inline bool		M44Decompose(Vector3 * pvScaling, Vector3  * pvRotationQuaternion, Vector3 * pvTranslation, const Matrix44 & m);
	
	inline Matrix44		M44Translation(f32 fOffsetX, f32 fOffsetY, f32 fOffsetZ)
	{
		Matrix44 m =
		{
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			fOffsetX, fOffsetY, fOffsetZ, 1.0f,
		};
		return m;
	}
	inline Matrix44		M44TranslationFromVector(Vector3 vOffset);
	inline Matrix44		M44Scaling(f32 fScaleX, f32 fScaleY, f32 fScaleZ)
	{
		Matrix44 m =
		{
			fScaleX, 0.0f, 0.0f, 0.0f,
			0.0f, fScaleY, 0.0f, 0.0f,
			0.0f, 0.0f, fScaleZ, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f,
		};
		return m;
	}
	inline Matrix44		M44ScalingFromVector(Vector3 vScale);
	inline Matrix44		M44RotationAxisNormalLH(Vector3 vNormalAsix, f32 fAngle)
	{
		// assert: axis is normalized.
		// assert: angle is in radian.

		const f32 & x = vNormalAsix.x;
		const f32 & y = vNormalAsix.y;
		const f32 & z = vNormalAsix.z;

		f32 sinAngle = sinf(fAngle);
		f32 cosAngle = cosf(fAngle);
		f32 one_cosAngle = 1.0f - cosAngle;

		f32 a0 = one_cosAngle * x * x;
		f32 a1 = one_cosAngle * y * y;
		f32 a2 = one_cosAngle * z * z;

		f32 a3 = one_cosAngle * x * y;
		f32 a4 = one_cosAngle * y * z;
		f32 a5 = one_cosAngle * z * x;

		f32 a6 = sinAngle * x;
		f32 a7 = sinAngle * y;
		f32 a8 = sinAngle * z;

		Matrix44 m =
		{
			a0 + cosAngle, a3 + a8, a5 - a7, 0.0f,
			a3 - a8, a1 + cosAngle, a4 + a6, 0.0f,
			a5 + a7, a4 - a6, a2 + cosAngle, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};

		return m;
	}
	inline Matrix44		M44RotationAxisLH(Vector3 vAsix, f32 fAngle)
	{
		// assert: angle is in radian.
		return M44RotationAxisNormalLH(V3Normalize(vAsix), fAngle);
	}
	inline Matrix44		M44RotationQuaternion(Vector4 vQuaternion);
	inline Matrix44		M44RotationRollPitchYaw(f32 fPitch, f32 fYaw, f32 fRoll);
	inline Matrix44		M44RotationRollPitchYawFromVector(Vector3 vAngles);
	inline Matrix44		M44RotationX(f32 fAngle);
	inline Matrix44		M44RotationY(f32 fAngle);
	inline Matrix44		M44RotationZ(f32 fAngle);

	// Matrix44 - Lighting
	inline Matrix44		M44Reflect(Vector4 vReflectionPlane);
	inline Matrix44		M44Shadow(Vector4 vShadowPlane, Vector3 vLightPosition);

	inline f32		ConvertToDegrees(f32 fRadians)
	{
		return fRadians * 180.0f * C_1DIVPI;
	}
	inline f32		ConvertToRadians(f32 fDegrees)
	{
		return fDegrees * C_PI / 180.0f;
	}

	// Collision Detection
	inline bool		IntersectRayPlane(const Vector3 & posPlane, const Vector3 & normPlane, const Vector3 & posRay, const Vector3 & dirRay, Vector3 * pPoint)
	{
		// assert: all norm/dir normalized

		f32 denom = V3Dot(-normPlane, dirRay);

		if ( denom <= 1e-6 )
		{
			return false;
		}

		Vector3 v = posPlane - posRay;

		f32 t = V3Dot(v, -normPlane) / denom;

		if ( t < 0.0f )
		{
			return false;
		}

		*pPoint = posRay + V3Scale(dirRay, t);

		return true;
	}


	// --------------------------------------------------------------------------
	// Classes
	// --------------------------------------------------------------------------

	// Transform
	// Connection
	// BoundingBox
}