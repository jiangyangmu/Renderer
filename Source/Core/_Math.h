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
	typedef int8_t		i8;
	typedef uint8_t		u8;
	typedef int32_t		i32;
	typedef uint32_t	u32;
	typedef int64_t		i64;
	typedef uint64_t	u64;
	typedef float		f32;

	// --------------------------------------------------------------------------
	// Constants & Routines
	// --------------------------------------------------------------------------

	constexpr f32		C_PI		= ( f32 ) 3.14159265358979323846;
	constexpr f32		C_2PI		= ( f32 ) 3.14159265358979323846 * 2.0;
	constexpr f32		C_1DIVPI	= ( f32 ) 0.318309886183790671538;
	constexpr f32		C_1DIV2PI	= ( f32 ) 0.318309886183790671538 * 0.5;
	constexpr f32		C_F32_EPSILON	= FLT_EPSILON;
	constexpr u32		C_F32_INFINITY	= 0x7F800000;
	constexpr u32		C_F32_QNAN	= 0x7CF00000;
	constexpr u32		C_F32_SIGN_BIT	= 0x80000000;

	inline bool		IsNear(f32 f0, f32 f1)
	{
		f32 fd = fabsf(f0 - f1);
		return fd <= C_F32_EPSILON;
	}
	inline bool		IsNearZero(f32 value)
	{
		return -C_F32_EPSILON <= value && value <= C_F32_EPSILON;
	}
	inline bool		F32IsNearZero(f32 value)
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

		inline Vector3 &	operator += (const Vector3 & other)
		{
			x += other.x;
			y += other.y;
			z += other.z;
			return *this;
		}
		inline Vector3 &	operator -= (const Vector3 & other)
		{
			x -= other.x;
			y -= other.y;
			z -= other.z;
			return *this;
		}

		inline f32		Length() const
		{
			return sqrtf(x * x + y * y + z * z);
		}
		inline void		Normalize()
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
			Vector3 xyz;
			struct
			{
				f32 x, y, z;
			};
		};
		f32 w;
	};
	typedef Vector4 Quaternion;
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
	struct BBox2
	{
		f32 xl, yl;
		f32 xh, yh;
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

	inline Vector4		operator - (const Vector4 & v)
	{
		return { -v.x, -v.y, -v.z, -v.w };
	}
	inline Vector4		operator + (const Vector4 & v0, const Vector4 & v1)
	{
		return { v0.x + v1.x, v0.y + v1.y, v0.z + v1.z, v0.w + v1.w };
	}
	inline Vector4		operator - (const Vector4 & v0, const Vector4 & v1)
	{
		return { v0.x - v1.x, v0.y - v1.y, v0.z - v1.z, v0.w - v1.w };
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

	// Scalar
	inline void		ScalarSinCos(f32 * pfSin, f32 * pfCos, f32 fAngle)
	{
		*pfSin = sinf(fAngle);
		*pfCos = cosf(fAngle);
	}

	// F32
	inline void		F32Lerp(f32 * pDst, const f32 * pSrc1, const f32 * pSrc2, f32 t, int nCount)
	{
		f32 u = 1.0f - t;
		for ( int i = 0; i < nCount; ++i )
		{
			*pDst = u * ( *pSrc1 ) + t * ( *pSrc2 );
			++pDst;
			++pSrc1;
			++pSrc2;
		}
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

	inline Vector3		V3Replicate(f32 fValue)
	{
		return { fValue, fValue, fValue };
	}
	inline Vector3		V3ReplicatePtr(const f32 * pfValue)
	{
		return { *pfValue, *pfValue, *pfValue };
	}
	inline Vector3		V3Set(f32 x, f32 y, f32 z)
	{
		return { x, y, z };
	}
	inline Vector3		V3SetBinaryConstant(u32 c0, u32 c1, u32 c2, u32 c3);
	inline Vector3		V3SplatConstant(i32 iConstant, u32 iDivExp);
	inline Vector3		V3SplatConstantInt(i32 iConstant);
	inline Vector3		V3SplatEpsilon()
	{ 
		return { C_F32_EPSILON, C_F32_EPSILON, C_F32_EPSILON };
	}
	inline Vector3		V3SplatInfinity()
	{
		union __fu32
		{
			f32 f; u32 u;
		} value;
		value.u = C_F32_INFINITY;
		return { value.f, value.f, value.f};
	}
	inline Vector3		V3SplatQNaN()
	{
		union __fu32
		{
			f32 f; u32 u;
		} value;
		value.u = C_F32_QNAN;
		return { value.f, value.f, value.f };
	}
	inline Vector3		V3SplatSignMask()
	{
		union __fu32
		{
			f32 f; u32 u;
		} value;
		value.u = C_F32_SIGN_BIT;
		return { value.f, value.f, value.f };
	}
	inline Vector3		V3TrueInt()
	{
		union __fu32
		{
			f32 f; u32 u;
		} value;
		value.u = 0xFFFFFFFF;
		return { value.f, value.f, value.f };
	}

	// Vector3 - Bitwise
	inline Vector3		V3AndCInt(Vector3 v0, Vector3 v1);
	inline Vector3		V3AndInt(Vector3 v0, Vector3 v1);
	inline Vector3		V3NorInt(Vector3 v0, Vector3 v1);
	inline Vector3		V3OrInt(Vector3 v0, Vector3 v1);
	inline Vector3		V3XorInt(Vector3 v0, Vector3 v1);

	// Vector3 - Componentwise
	inline Vector3		V3SplatX(Vector3 v);
	inline Vector3		V3SplatY(Vector3 v);
	inline Vector3		V3SplatZ(Vector3 v);
	inline Vector3		V3Swizzle(Vector3 v, u32 iComponent0, u32 iComponent1, u32 iComponent2);
	inline Vector3		V3Permute(Vector3 v0, Vector3 v1, u32 iPermuteX, u32 iPermuteY, u32 iPermuteZ);
	inline Vector3		V3RotateLeft(Vector3 v, u32 iCount);
	inline Vector3		V3RotateRight(Vector3 v, u32 iCount);
	inline Vector3		V3ShiftLeft(Vector3 v0, Vector3 v1, u32 iCount);
	inline Vector3		V3Insert(Vector3 vD, Vector3 vS, u32 iVSLeftRotateElements, u32 iVector0, u32 iVector1, u32 iVector2);
	inline Vector3		V3Select(Vector3 v0, Vector3 v1, Vector3 vControl);
	inline Vector3		V3SelectControl(u32 iVector0, u32 iVector1, u32 iVector2);

	// Vector3 - Arithmetic
	inline Vector3		V3Abs(Vector3 v);
	inline Vector3		V3Add(Vector3 v0, Vector3 v1);
	inline Vector3		V3AddAngles(Vector3 v0, Vector3 v1);
	inline Vector3		V3Ceiling(Vector3 v);
	inline Vector3		V3Clamp(Vector3 v, Vector3 vMin, Vector3 vMax);
	inline Vector3		V3Divide(Vector3 v0, Vector3 v1);
	inline Vector3		V3Floor(Vector3 v);
	inline Vector3		V3IsInfinite(Vector3 v);
	inline Vector3		V3IsNaN(Vector3 v);
	inline Vector3		V3Max(Vector3 v0, Vector3 v1);
	inline Vector3		V3Min(Vector3 v0, Vector3 v1);
	inline Vector3		V3Mod(Vector3 v0, Vector3 v1);
	inline Vector3		V3ModAngles(Vector3 vAngles);
	inline Vector3		V3Multiply(Vector3 v0, Vector3 v1)
	{
		return
		{
			v0.x * v1.x,
			v0.y * v1.y,
			v0.z * v1.z
		};
	}
	inline Vector3		V3MultiplyAdd(Vector3 v0, Vector3 v1, Vector3 v2);
	inline Vector3		V3Negate(Vector3 v);
	inline Vector3		V3NegativeMultiplySubtract(Vector3 v0, Vector3 v1, Vector3 v2);
	inline Vector3		V3Pow(Vector3 v0, Vector3 v1);
	inline Vector3		V3Reciprocal(Vector3 v);
	inline Vector3		V3ReciprocalEst(Vector3 v);
	inline Vector3		V3ReciprocalSqrt(Vector3 v);
	inline Vector3		V3ReciprocalSqrtEst(Vector3 v);
	inline Vector3		V3Round(Vector3 v);
	inline Vector3		V3Saturate(Vector3 v);
	inline Vector3		V3Scale(Vector3 v, f32 fScale)
	{
		return { v.x * fScale, v.y * fScale, v.z * fScale };
	}
	inline Vector3		V3Sqrt(Vector3 v);
	inline Vector3		V3SqrtEst(Vector3 v);
	inline Vector3		V3Subtract(Vector3 v0, Vector3 v1);
	inline Vector3		V3SubtractAngles(Vector3 v0, Vector3 v1);
	inline Vector3		V3Sum(Vector3 v);
	inline Vector3		V3Truncate(Vector3 v);

	// Vector3 - Transcendental
	inline Vector3		V3Sin(Vector3 v)
	{
		return { sinf(v.x), sinf(v.y), sinf(v.z) };
	}
	inline Vector3		V3SinEst(Vector3 v);
	inline Vector3		V3SinH(Vector3 v)
	{
		return { sinhf(v.x), sinhf(v.y), sinhf(v.z) };
	}
	inline Vector3		V3Cos(Vector3 v)
	{
		return { cosf(v.x), cosf(v.y), cosf(v.z) };
	}
	inline Vector3		V3CosEst(Vector3 v);
	inline Vector3		V3CosH(Vector3 v)
	{
		return { coshf(v.x), coshf(v.y), coshf(v.z) };
	}
	inline void		V3SinCos(Vector3 * pSin, Vector3 * pCos, Vector3 v);
	inline void		V3SinCosEst(Vector3 * pSin, Vector3 * pCos, Vector3 v);
	inline Vector3		V3Tan(Vector3 v)
	{
		return { tanf(v.x), tanf(v.y), tanf(v.z) };
	}
	inline Vector3		V3TanEst(Vector3 v);
	inline Vector3		V3TanH(Vector3 v)
	{
		return { tanhf(v.x), tanhf(v.y), tanhf(v.z) };
	}
	inline Vector3		V3ACos(Vector3 v)
	{
		return { acosf(v.x), acosf(v.y), acosf(v.z) };
	}
	inline Vector3		V3ACosEst(Vector3 v);
	inline Vector3		V3ASin(Vector3 v)
	{
		return { asinf(v.x), asinf(v.y), asinf(v.z) };
	}
	inline Vector3		V3ASinEst(Vector3 v);
	inline Vector3		V3ATan(Vector3 v)
	{
		return { atanf(v.x), atanf(v.y), atanf(v.z) };
	}
	inline Vector3		V3ATanEst(Vector3 v);
	inline Vector3		V3ATan2(Vector3 vY, Vector3 vX)
	{
		return { atan2f(vY.x, vX.x), atan2f(vY.y, vX.y), atan2f(vY.z, vX.z) };
	}
	inline Vector3		V3ATan2Est(Vector3 vY, Vector3 vX);
	inline Vector3		V3Exp2(Vector3 v)
	{
		return { exp2f(v.x), exp2f(v.y), exp2f(v.z) };
	}
	inline Vector3		V3ExpE(Vector3 v)
	{
		return { expf(v.x), expf(v.y), expf(v.z) };
	}
	inline Vector3		V3Log2(Vector3 v)
	{
		return { log2f(v.x), log2f(v.y), log2f(v.z) };
	}
	inline Vector3		V3LogE(Vector3 v)
	{
		return { logf(v.x), logf(v.y), logf(v.z) };
	}

	// Vector3 - Comparison
	inline bool		V3HasInfinite(Vector3 v)
	{
		return isinf(v.x) || isinf(v.y) || isinf(v.z);
	}
	inline bool		V3HasNaN(Vector3 v)
	{
		return isnan(v.x) || isnan(v.y) || isnan(v.z);
	}
	inline bool		V3Equal(Vector3 v0, Vector3 v1)
	{
		return v0.x == v1.x && v0.y == v1.y && v0.z == v1.z;
	}
	inline bool		V3NearEqual(Vector3 v0, Vector3 v1)
	{
		return IsNear(v0.x, v1.x) && IsNear(v0.y, v1.y) && IsNear(v0.z, v1.z);
	}
	inline bool		V3NotEqual(Vector3 v0, Vector3 v1)
	{
		return v0.x != v1.x || v0.y != v1.y || v0.z != v1.z;
	}
	inline bool		V3Greater(Vector3 v0, Vector3 v1)
	{
		return v0.x > v1.x && v0.y > v1.y && v0.z > v1.z;
	}
	inline bool		V3GreaterOrEqual(Vector3 v0, Vector3 v1)
	{
		return v0.x >= v1.x && v0.y >= v1.y && v0.z >= v1.z;
	}
	inline bool		V3Less(Vector3 v0, Vector3 v1)
	{
		return v0.x < v1.x && v0.y < v1.y && v0.z < v1.z;
	}
	inline bool		V3LessOrEqual(Vector3 v0, Vector3 v1)
	{
		return v0.x <= v1.x && v0.y <= v1.y && v0.z <= v1.z;
	}

	// Vector3 - Geometric
	inline f32		V3AngleBetweenNormals(Vector3 v0, Vector3 v1);
	inline f32		V3AngleBetweenNormalsEst(Vector3 v0, Vector3 v1);
	inline f32		V3AngleBetweenVectors(Vector3 v0, Vector3 v1);
	inline Vector3		V3BaryCentric(Vector3 vP0, Vector3 vP1, Vector3 vP2, f32 f, f32 g);
	inline Vector3		V3BaryCentricV(Vector3 vP0, Vector3 vP1, Vector3 vP2, Vector3 vF, Vector3 vG);
	inline Vector3		V3CatmullRom(Vector3 vP0, Vector3 vP1, Vector3 vP2, Vector3 vP3, f32 t);
	inline Vector3		V3CatmullRomV(Vector3 vP0, Vector3 vP1, Vector3 vP2, Vector3 vP3, Vector3 vT);
	inline Vector3		V3Hermite(Vector3 vP0, Vector3 vTangent0, Vector3 vP1, Vector3 vTangent1, f32 t);
	inline Vector3		V3HermiteV(Vector3 vP0, Vector3 vTangent0, Vector3 vP1, Vector3 vTangent1, Vector3 vT);
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

	inline Vector3		V3ClampLength(Vector3 v, f32 fLengthMin, f32 fLengthMax);
	inline Vector3		V3ClampLengthV(Vector3 v, Vector3 vLengthMin, Vector3 vLengthMax);
	inline bool		V3InBounds(Vector3 v, Vector3 vBounds);
	inline f32		V3Length(Vector3 v)
	{
		return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	}
	inline f32		V3LengthEst(Vector3 v);
	inline f32		V3LengthSq(Vector3 v)
	{
		return v.x * v.x + v.y * v.y + v.z * v.z;
	}
	inline f32		V3ReciprocalLength(Vector3 v)
	{
		return 1.0f / sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
	}
	inline f32		V3ReciprocalLengthEst(Vector3 v);
	inline Vector3		V3Normalize(Vector3 v)
	{
		return V3Scale(v, V3ReciprocalLength(v));
	}
	inline Vector3		V3NormalizeEst(Vector3 v);

	inline void		V3ComponentsFromNormal(Vector3 * pParallel, Vector3 * pPerpendicular, Vector3 v, Vector3 normal);
	inline Vector3		V3CrossLH(Vector3 v0, Vector3 v1)
	{
		return
		{
			v0.y * v1.z - v0.z * v1.y,
			v0.z * v1.x - v0.x * v1.z,
			v0.x * v1.y - v0.y * v1.x
		};
	}
	inline f32		V3Dot(Vector3 v0, Vector3 v1)
	{
		return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
	}
	inline Vector3		V3Orthogonal(Vector3 v);
	inline Vector3		V3Reflect(Vector3 vIncident, Vector3 vNormal);
	inline Vector3		V3Refract(Vector3 vIncident, Vector3 vNormal, f32 fRefractionIndex);
	inline Vector3		V3RefractV(Vector3 vIncident, Vector3 vNormal, Vector3 vRefractionIndex);

	inline Vector3		V3Rotate(Vector3 v, Vector4 vRotationQuaternion);
	inline Vector3		V3InverseRotate(Vector3 v, Vector4 vRotationQuaternion);
	inline Vector3		V3Project(Vector3 v, f32 fViewportX, f32 fViewportY, f32 fViewportWidth, f32 fViewportHeight, f32 fViewportMinZ, f32 fViewportMaxZ, const Matrix44 & mProjection, const Matrix44 & mView, const Matrix44 & mWorld);
	inline Vector3		V3Unproject(Vector3 v, f32 fViewportX, f32 fViewportY, f32 fViewportWidth, f32 fViewportHeight, f32 fViewportMinZ, f32 fViewportMaxZ, const Matrix44 & mProjection, const Matrix44 & mView, const Matrix44 & mWorld);
	inline Vector3 *	V3ProjectStream(Vector3 * pOutputStream, u32 iOutputStride, const Vector3 *pInputStream, u32 iInputStride, u32 iVectorCount, f32 fViewportX, f32 fViewportY, f32 fViewportWidth, f32 fViewportHeight, f32 fViewportMinZ, f32 fViewportMaxZ, const Matrix44 & mProjection, const Matrix44 & mView, const Matrix44 & mWorld);
	inline Vector3 *	V3UnprojectStream(Vector3 * pOutputStream, u32 iOutputStride, const Vector3 *pInputStream, u32 iInputStride, u32 iVectorCount, f32 fViewportX, f32 fViewportY, f32 fViewportWidth, f32 fViewportHeight, f32 fViewportMinZ, f32 fViewportMaxZ, const Matrix44 & mProjection, const Matrix44 & mView, const Matrix44 & mWorld);
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
	inline Vector3		V3TransformNormal(Vector3 v, const Matrix44 & m);
	inline Vector3		V3TransformCoord(Vector3 v, const Matrix44 & m);
	inline Vector3 *	V3TransformStream(Vector3 * pOutputStream, u32 iOutputStride, const Vector3 *pInputStream, u32 iInputStride, u32 iVectorCount, const Matrix44 & m);
	inline Vector3 *	V3TransformNormalStream(Vector3 * pOutputStream, u32 iOutputStride, const Vector3 *pInputStream, u32 iInputStride, u32 iVectorCount, const Matrix44 & m);
	inline Vector3 *	V3TransformCoordStream(Vector3 * pOutputStream, u32 iOutputStride, const Vector3 *pInputStream, u32 iInputStride, u32 iVectorCount, const Matrix44 & m);

	// Vector4 - Componentwise
	inline f32		V4GetByIndex(const Vector4 & v, int i)
	{
		return ((const f32 *)&v)[i];
	}
	inline void		V4SetByIndex(Vector4 & v, int i, f32 value)
	{
		( ( f32 * ) &v )[ i ] = value;
	}
	// Vector4 - Arithmetic
	inline Vector4		V4Scale(Vector4 v, f32 fScale)
	{
		return { v.x * fScale, v.y * fScale, v.z * fScale, v.w * fScale };
	}
	inline Vector4		V4Saturate(Vector4 v)
	{
		return
		{
			Bound(0.0f, v.x, 1.0f),
			Bound(0.0f, v.y, 1.0f),
			Bound(0.0f, v.z, 1.0f),
			Bound(0.0f, v.w, 1.0f),
		};
	}

	// Vector4 - Geometric
	inline Vector4		V4Transform(Vector4 v, const Matrix44 & m)
	{
		return
		{
			m._11 * v.x + m._21 * v.y + m._31 * v.z + m._41 * v.w,
			m._12 * v.x + m._22 * v.y + m._32 * v.z + m._42 * v.w,
			m._13 * v.x + m._23 * v.y + m._33 * v.z + m._43 * v.w,
			m._14 * v.x + m._24 * v.y + m._34 * v.z + m._44 * v.w,
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

	// Matrix44 - Comparison
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
	inline Matrix44		M44LookToLH(Vector3 vEyePosition, Vector3 vEyeDirection, Vector3 vUpDirection)
	{
		// assert: eyeDirection not zero or infinite
		// assert: up not zero or infinite
		Vector3 r2 = V3Normalize(vEyeDirection);
		Vector3 r0 = V3Normalize(V3CrossLH(vUpDirection, r2));
		Vector3 r1 = V3CrossLH(r2, r0);
		Vector3 vNegEyePosition = -vEyePosition;
		f32 d0 = V3Dot(r0, vNegEyePosition);
		f32 d1 = V3Dot(r1, vNegEyePosition);
		f32 d2 = V3Dot(r2, vNegEyePosition);
		Matrix44 m =
		{
			r0.x, r1.x, r2.x, 0.0f,
			r0.y, r1.y, r2.y, 0.0f,
			r0.z, r1.z, r2.z, 0.0f,
			d0, d1, d2, 1.0f
		};
		return m;
	}
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
	inline Matrix44		M44RotationQuaternion(Quaternion q);
	inline Matrix44		M44RotationRollPitchYaw(f32 fPitch, f32 fYaw, f32 fRoll);
	inline Matrix44		M44RotationRollPitchYawFromVector(Vector3 vAngles);
	inline Matrix44		M44RotationX(f32 fAngle);
	inline Matrix44		M44RotationY(f32 fAngle);
	inline Matrix44		M44RotationZ(f32 fAngle);

	// Matrix44 - Lighting
	inline Matrix44		M44Reflect(Vector4 vReflectionPlane);
	inline Matrix44		M44Shadow(Vector4 vShadowPlane, Vector3 vLightPosition);

	// Quaternion
	inline Quaternion	Q4Identity()
	{
		return { 0.0f, 0.0f, 0.0f, 1.0f };
	}

	// Quaternion - Arithmetic
	inline Quaternion	Q4Exp(Quaternion q);
	inline Quaternion	Q4Ln(Quaternion q);

	// Quaternion - Comparison
	inline bool		Q4Equal(Quaternion q0, Quaternion q1)
	{
		return q0.x == q1.x && q0.y == q1.y && q0.z == q1.z && q0.w == q1.w;
	}
	inline bool		Q4NotEqual(Quaternion q0, Quaternion q1)
	{
		return q0.x != q1.x || q0.y != q1.y || q0.z != q1.z || q0.w != q1.w;
	}
	inline bool		Q4IsIdentity(Quaternion q)
	{
		return q.x == 0.0f && q.y == 0.0f && q.z == 0.0f && q.w == 1.0f;
	}
	inline bool		Q4HasInfinite(Quaternion q)
	{
		return isinf(q.x) || isinf(q.y) || isinf(q.z);
	}
	inline bool		Q4HasNaN(Quaternion q)
	{
		return isnan(q.x) || isnan(q.y) || isnan(q.z);
	}

	// Quaternion - Geometric
	inline f32		Q4Length(Quaternion q)
	{
		return sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
	}
	inline f32		Q4LengthSq(Quaternion q)
	{
		return q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
	}
	inline f32		Q4ReciprocalLength(Quaternion q)
	{
		return 1.0f / Q4Length(q);
	}
	inline Quaternion	Q4Normalize(Quaternion q)
	{
		return V4Scale(q, Q4ReciprocalLength(q));
	}
	inline Quaternion	Q4NormalizeEst(Quaternion q);
	inline Quaternion	Q4Slerp(Quaternion qNorm0, Quaternion qNorm1, f32 t);
	inline Quaternion	Q4Squad(Quaternion qNorm0, Quaternion qNorm1, Quaternion qNorm2, Quaternion qNorm3, f32 t);
	inline void		Q4SquadSetup(Quaternion * pqA, Quaternion * pqB, Quaternion * pqC, Quaternion q0, Quaternion q1, Quaternion q2, Quaternion q3);
	inline void		Q4ToAxisAngle(Vector3 * pvAxis, f32 * pfAngle, Quaternion q);

	inline Quaternion	Q4BaryCentric(Quaternion q0, Quaternion q1, Quaternion q2, f32 f, f32 g);
	inline Quaternion	Q4Conjugate(Quaternion q)
	{
		Quaternion qc;
		qc.xyz	= -q.xyz;
		qc.w	= q.w;
		return qc;
	}
	inline f32		Q4Dot(Quaternion q0, Quaternion q1)
	{
		return q0.x * q1.x + q0.y * q1.y + q0.z * q1.z + q0.w * q1.w; 
	}
	inline Quaternion	Q4HamiltonProductLH(Quaternion q0, Quaternion q1)
	{
		Quaternion q;
		q.xyz	= V3CrossLH(q0.xyz, q1.xyz) + V3Scale(q0.xyz, q1.w) + V3Scale(q1.xyz, q0.w);
		q.w	= q0.w * q1.w - V3Dot(q0.xyz, q1.xyz);
		return q;
	}
	inline Quaternion	Q4Inverse(Quaternion q)
	{
		return V4Scale(Q4Conjugate(q), 1.0f / Q4LengthSq(q));
	}

	// Quaternion - Transform
	inline Quaternion	Q4RotationNormalLH(Vector3 vNormalAxis, f32 fAngle)
	{
		f32 fSin;
		f32 fCos;

		ScalarSinCos(&fSin, &fCos, 0.5f * fAngle);

		Quaternion q;

		q.xyz	= V3Scale(vNormalAxis, fSin);
		q.w	= fCos;
		
		return q;
	}
	inline Quaternion	Q4RotationAxisLH(Vector3 vAxis, f32 fAngle)
	{
		return Q4RotationNormalLH(V3Normalize(vAxis), fAngle);
	}
	inline Quaternion	Q4RotationMatrix(Matrix44 m);
	inline Quaternion	Q4RotationRollPitchYaw(f32 fPitch, f32 fYaw, f32 fRoll);
	inline Quaternion	Q4RotationRollPitchYawFromVector(Vector3 vAngles);

	// Vector3 x Quaternion
	inline Vector3		V3Transform(Vector3 v, const Quaternion & q, const Quaternion & qInv)
	{
		Quaternion qv = { v, 1.0f };
		return Q4HamiltonProductLH(Q4HamiltonProductLH(q, qv), qInv).xyz;
	}

	// Bounding Box 2D
	inline BBox2		BB2Create(const Vector2 & p0, const Vector2 & p1, const Vector2 & p2)
	{
		BBox2 bb2;
		bb2.xl = Min3(p0.x, p1.x, p2.x);
		bb2.yl = Min3(p0.y, p1.y, p2.y);
		bb2.xh = Max3(p0.x, p1.x, p2.x);
		bb2.yh = Max3(p0.y, p1.y, p2.y);
		return bb2;
	}

	// Conversion
	inline f32		ConvertToDegrees(f32 fRadians)
	{
		return fRadians * 180.0f * C_1DIVPI;
	}
	inline f32		ConvertToRadians(f32 fDegrees)
	{
		return fDegrees * C_PI / 180.0f;
	}

	// Collision Detection
	inline Vector3		ProjectPointPlane(const Vector3 & posPlane, const Vector3 & normPlane, const Vector3 & p)
	{
		Vector3 v = p - posPlane;
		f32 dist = V3Dot(v, normPlane);
		return p - V3Scale(normPlane, dist);
	}
	inline bool		IntersectLinePlane(const Vector3 & posPlane, const Vector3 & normPlane, const Vector3 & posLine, const Vector3 & dirLine, f32 * pT)
	{
		// assert: all norm/dir normalized

		f32 vn = V3Dot(dirLine, normPlane);

		if ( fabsf(vn) <= 1e-6 )
		{
			return false;
		}

		Vector3 v = posPlane - posLine;

		*pT = V3Dot(v, normPlane) / vn;

		return true;
	}
	inline f32		DistancePointLine(Vector3 vL0, Vector3 vL1, Vector3 vPoint);
	inline void		MirrorRayPlane(const Vector3 & posPlane, const Vector3 & normPlane, const Vector3 & posRay, const Vector3 & dirRay, Vector3 * pPosRayMirr, Vector3 * pDirRayMirr)
	{
		const Vector3 & posOrig = posRay;
		const Vector3 & dirOrig = dirRay;
		Vector3 posProject;
		Vector3 posIntersect;
		f32 t;

		posProject = ProjectPointPlane(posPlane, normPlane, posOrig);

		*pPosRayMirr = posOrig + V3Scale(posProject - posOrig, 2.0f);

		if ( IntersectLinePlane(posPlane, normPlane, posOrig, dirOrig, &t) )
		{
			posIntersect = posOrig + V3Scale(dirOrig, t);
			*pDirRayMirr = V3Normalize(posIntersect - *pPosRayMirr);
			if (t < 0.0f) *pDirRayMirr = -*pDirRayMirr;
		}
		else
		{
			*pDirRayMirr = dirOrig;
		}
	}

	//

	// --------------------------------------------------------------------------
	// Classes
	// --------------------------------------------------------------------------

	// Transform
	// Connection
	// BoundingBox
}