#pragma once

// --------------------------------------------------------------------------
// Common type aliases, constants
// --------------------------------------------------------------------------

#include <cinttypes>

typedef unsigned char Byte;
typedef int64_t Integer;

constexpr auto PI = 3.141592653f;

#include <memory>

template <typename T>
using Ptr = std::unique_ptr<T>;
template <typename T>
using Ref = std::shared_ptr<T>; // TODO: implement a non thread-safe one

// --------------------------------------------------------------------------
// Linear algebra utils
// --------------------------------------------------------------------------

#include <cmath>

struct Matrix4x4;

struct Vec2
{
	float x, y;
};

struct Vec3
{
	float x, y, z;

	// Constructors

	static inline Vec3 Zero()
	{
		return { 0.0f, 0.0f, 0.0f };
	}

	// Properties

	inline float Length() const
	{
		return sqrtf(x * x + y * y + z * z);
	}

	// Operations

	inline void Scale(float s)
	{
		x *= s;
		y *= s;
		z *= s;
	}
	
	inline Vec3 operator - () const
	{
		return { -x, -y, -z };
	}
	inline Vec3 operator + (const Vec3 & other) const
	{
		return { x + other.x, y + other.y, z + other.z };
	}
	inline Vec3 operator - (const Vec3 & other) const
	{
		return { x - other.x, y - other.y, z - other.z };
	}

	static inline Vec3 Scale(const Vec3 & v, float scale)
	{
		return { v.x * scale, v.y * scale, v.z * scale };
	}
	static inline Vec3 Normalize(const Vec3 & v)
	{
		return Scale(v, 1.0f / v.Length());
	}
	static inline float Dot(const Vec3 & v1, const Vec3 & v2)
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}
	static inline Vec3 ElementwiseProduct(const Vec3 & v1, const Vec3 & v2)
	{
		return { v1.x * v2.x, v1.y * v2.y, v1.z * v2.z };
	}
	static inline Vec3 CrossLH(const Vec3 & v1, const Vec3 & v2)
	{
		// Left handed
		return
		{
			v1.y * v2.z - v1.z * v2.y,
			v1.z * v2.x - v1.x * v2.z,
			v1.x * v2.y - v1.y * v2.x
		};
	}
	static inline Vec3 Transform(const Vec3 & v, const Matrix4x4 & m);
};

struct Matrix4x4
{
	float f11, f12, f13, f14;
	float f21, f22, f23, f24;
	float f31, f32, f33, f34;
	float f41, f42, f43, f44;

	// Constructors

	static inline Matrix4x4 Identity()
	{
		Matrix4x4 m =
		{
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f,
		};

		return m;
	}
	static inline Matrix4x4 Translation(float x, float y, float z)
	{
		Matrix4x4 m =
		{
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			x, y, z, 1.0f,
		};

		return m;
	}
	static inline Matrix4x4 PerspectiveFovLH(float fov, float aspectRatio, float zNear, float zFar)
	{
		float height = 1.0f / tanf(0.5f * fov);
		float width = height / aspectRatio;
		float fRange = zFar / ( zFar - zNear );

		Matrix4x4 m =
		{
			width, 0.0f, 0.0f, 0.0f,
			0.0f, height, 0.0f, 0.0f,
			0.0f, 0.0f, fRange, 1.0f,
			0.0f, 0.0f, -fRange * zNear, 0.0f
		};

		return m;
	}
	static inline Matrix4x4 RotationAxisLH(const Vec3 & axis, float angle)
	{
		// assert: axis is normalized.
		// assert: angle is in radian.

		const float & x = axis.x;
		const float & y = axis.y;
		const float & z = axis.z;

		float sinAngle = sinf(angle);
		float cosAngle = cosf(angle);
		float one_cosAngle = 1.0f - cosAngle;

		float a0 = one_cosAngle * x * x;
		float a1 = one_cosAngle * y * y;
		float a2 = one_cosAngle * z * z;

		float a3 = one_cosAngle * x * y;
		float a4 = one_cosAngle * y * z;
		float a5 = one_cosAngle * z * x;

		float a6 = sinAngle * x;
		float a7 = sinAngle * y;
		float a8 = sinAngle * z;

		Matrix4x4 m =
		{
			a0 + cosAngle, a3 + a8, a5 - a7, 0.0f,
			a3 - a8, a1 + cosAngle, a4 + a6, 0.0f,
			a5 + a7, a4 - a6, a2 + cosAngle, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f /*sinAngle, sinAngle, sinAngle, 1.0f*/
		};

		return m;
	}
	static inline Matrix4x4 LookToLH(const Vec3 & eyePosition, const Vec3 & eyeDirection, const Vec3 & up)
	{
		// assert: eyeDirection not zero or infinite
		// assert: up not zero or infinite

		Vec3 r2 = Vec3::Normalize(eyeDirection);
		Vec3 r0 = Vec3::Normalize(Vec3::CrossLH(up, r2));
		Vec3 r1 = Vec3::CrossLH(r2, r0);

		Vec3 negEyePosition = -eyePosition;

		float d0 = Vec3::Dot(r0, negEyePosition);
		float d1 = Vec3::Dot(r1, negEyePosition);
		float d2 = Vec3::Dot(r2, negEyePosition);

		Matrix4x4 m =
		{
			r0.x, r1.x, r2.x, 0.0f,
			r0.y, r1.y, r2.y, 0.0f,
			r0.z, r1.z, r2.z, 0.0f,
			d0, d1, d2, 1.0f
		};

		return m;
	}
	static inline Matrix4x4 Transpose(const Matrix4x4 & m)
	{
		Matrix4x4 mt =
		{
			m.f11, m.f21, m.f31, m.f41,
			m.f12, m.f22, m.f32, m.f42,
			m.f13, m.f23, m.f33, m.f43,
			m.f14, m.f24, m.f34, m.f44,
		};

		return mt;
	}

	// Operations

	inline Matrix4x4 operator * (const Matrix4x4 & other) const
	{
		return Multiply(*this, other);
	}
	static inline Matrix4x4 Multiply(const Matrix4x4 & m1, const Matrix4x4 & m2)
	{
		Matrix4x4 m;

		const float * pm1 = reinterpret_cast< const float * >( &m1 );
		const float * pm2 = reinterpret_cast< const float * >( &m2 );
		float * pm = reinterpret_cast< float * >( &m );
		for ( int row = 0; row < 4; ++row )
		{
			for ( int col = 0; col < 4; ++col )
			{
				pm[ row * 4 + col ] =
					pm1[ row * 4 + 0 ] * pm2[ 0 * 4 + col ] +
					pm1[ row * 4 + 1 ] * pm2[ 1 * 4 + col ] +
					pm1[ row * 4 + 2 ] * pm2[ 2 * 4 + col ] +
					pm1[ row * 4 + 3 ] * pm2[ 3 * 4 + col ];
			}
		}

		return m;
	}

	inline Vec3 GetTranslationVector() const
	{
		return { f41, f42, f43 };
	}
};

Vec3 Vec3::Transform(const Vec3 & v, const Matrix4x4 & m)
{
	float w = m.f14 * v.x + m.f24 * v.y + m.f34 * v.z + m.f44;
	float wInv = ( fabsf(w) < 0.0001f ? 10000.0f : 1.0f / w );
	return
	{
		( m.f11 * v.x + m.f21 * v.y + m.f31 * v.z + m.f41 ) * wInv,
		( m.f12 * v.x + m.f22 * v.y + m.f32 * v.z + m.f42 ) * wInv,
		( m.f13 * v.x + m.f23 * v.y + m.f33 * v.z + m.f43 ) * wInv,
	};
}

inline Vec2 WeightedAdd(const Vec2 & v0, const Vec2 & v1, const Vec2 & v2, float w0, float w1, float w2)
{
	return
	{
		v0.x * w0 + v1.x * w1 + v2.x * w2,
		v0.y * w0 + v1.y * w1 + v2.y * w2,
	};
}
inline Vec3 WeightedAdd(const Vec3 & v0, const Vec3 & v1, const Vec3 & v2, float w0, float w1, float w2)
{
	return
	{
		v0.x * w0 + v1.x * w1 + v2.x * w2,
		v0.y * w0 + v1.y * w1 + v2.y * w2,
		v0.z * w0 + v1.z * w1 + v2.z * w2,
	};
}

struct RGB
{
	float r, g, b;
};

inline Vec3 RGBToVec3(const RGB & rgb)
{
	return { rgb.r, rgb.g, rgb.b };
}

inline RGB Vec3ToRGB(const Vec3 & v)
{
	return { v.x, v.y, v.z };
}

inline float DegreeToRadian(float d)
{
	return d * PI / 180.0f;
}

inline float RadianToDegree(float r)
{
	return r * 180.0f / PI;
}

template <typename T>
inline T Bound(T min, T value, T max)
{
	return value < min ? min : ( value > max ? max : value );
}

template <typename T>
inline T Max(T a, T b)
{
	return a >= b ? a : b;
}
template <typename T>
inline T Min(T a, T b)
{
	return a <= b ? a : b;
}
inline float Min3(float a, float b, float c)
{
	return	a < b
		? ( a < c ? a : c )
		: ( b < c ? b : c );
}
inline float Max3(float a, float b, float c)
{
	return	a > b
		? ( a > c ? a : c )
		: ( b > c ? b : c );
}
inline float EdgeFunction(const Vec2 & a, const Vec2 & b, const Vec2 & c)
{
	return ( c.x - a.x ) * ( b.y - a.y ) - ( c.y - a.y ) * ( b.x - a.x );
}
template <typename T>
inline T AlignFloor(T value, T alignment)
{
	// value > 0, alignment > 0
	return value - (value % alignment);
}
template <typename T>
inline T AlignCeiling(T value, T alignment)
{
	// value > 0, alignment > 0
	T m = value % alignment;
	return m > 0 ? (value - m + alignment) : value;
}

// --------------------------------------------------------------------------
// Macros for debugging
// --------------------------------------------------------------------------

#include <cassert>

#define _ALERT_IF_FALSE(e) if (!(e)) { MessageBox(NULL, TEXT(#e), TEXT("Exception"), MB_OK); ExitProcess(0); }

#define ENSURE_NOT_NULL(e) _ALERT_IF_FALSE((e) != NULL)
#define ENSURE_TRUE(e) _ALERT_IF_FALSE((e))
#define ENSURE_GDIPLUS_OK(e) _ALERT_IF_FALSE((e) == Gdiplus::Ok)

#define _RELEASE_ASSERT
#ifdef _RELEASE_ASSERT

#ifdef NDEBUG
#define ASSERT(e) _ALERT_IF_FALSE(e)
#else
#define ASSERT(e) assert(e)
#endif

#else

#define ASSERT(e) assert(e)

#endif
