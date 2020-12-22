#pragma once

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

	inline Vec3 & Scale(float s)
	{
		x *= s;
		y *= s;
		z *= s;
		return *this;
	}
	inline Vec3 operator + (const Vec3 & other) const
	{
		return { x + other.x, y + other.y, z + other.z };
	}
	inline Vec3 operator - (const Vec3 & other) const
	{
		return { x - other.x, y - other.y, z - other.z };
	}

	static inline Vec3 Normalize(const Vec3 & v)
	{
		Vec3 nv = v;
		return nv.Scale(1.0 / nv.Length());
	}
	static inline float Dot(const Vec3 & v1, const Vec3 & v2)
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}
	static inline Vec3 Cross(const Vec3 & v1, const Vec3 & v2)
	{
		return
		{
			v1.y * v2.z - v1.z * v2.y,
			v1.z * v2.x - v1.x * v2.z,
			v1.x * v2.y - v1.y * v2.x
		};
	}

};

struct Matrix4x4
{
	float f11, f12, f13, f14;
	float f21, f22, f23, f24;
	float f31, f32, f33, f34;
	float f41, f42, f43, f44;

	// Constructors

	static Matrix4x4 Identity()
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
	static Matrix4x4 PerspectiveFovLH(float fov, float aspectRatio, float zNear, float zFar)
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
};
