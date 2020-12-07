
#include <fstream>

constexpr auto PI = 3.141592653f;

struct RGB
{
	float r, g, b;
};
struct Vec3f
{
	float x, y, z;
};
struct Matrix4x4
{
	float f11, f12, f13, f14;
	float f21, f22, f23, f24;
	float f31, f32, f33, f34;
	float f41, f42, f43, f44;
};

struct Triangle
{
	Vec3f a, b, c;
	RGB color;
};

struct AABB
{
	Vec3f min, max;
};

struct Camera
{
	Vec3f pos;
	float near, far;
	float fov; // in radian
	float aspectRatio;
};

float DegreeToRadian(float d)
{
	// 0 < d < 180
	return d * PI / 180.0f;
}

Matrix4x4 PerspectiveFovLH(float fov, float aspectRatio, float near, float far)
{
	float height = 1.0f / tanf(0.5f * fov);
	float width = height / aspectRatio;
	float fRange = far / ( far - near );

	Matrix4x4 m =
	{
		width, 0.0f, 0.0f, 0.0f,
		0.0f, height, 0.0f, 0.0f,
		0.0f, 0.0f, fRange, 1.0f,
		0.0f, 0.0f, -fRange * near, 0.0f
	};

	return m;
}

Vec3f Multiply(const Vec3f & v, const Matrix4x4 & m)
{
	float w = m.f14 * v.x + m.f24 * v.y + m.f34 * v.z + m.f44;
	float wInv = (w == 0.0f ? 0.0f : 1.0f / w);
	return
	{
		( m.f11 * v.x + m.f21 * v.y + m.f31 * v.z + m.f41 ) * wInv,
		( m.f12 * v.x + m.f22 * v.y + m.f32 * v.z + m.f42 ) * wInv,
		( m.f13 * v.x + m.f23 * v.y + m.f33 * v.z + m.f43 ) * wInv,
	};
}

inline float Min(float a, float b, float c)
{
	return	a < b
		? ( a < c ? a : c )
		: ( b < c ? b : c );
}
inline float Max(float a, float b, float c)
{
	return	a > b
		? ( a > c ? a : c )
		: ( b > c ? b : c );
}
inline float Bound(float min, float value, float max)
{
	return value < min ? min : (value > max ? max : value);
}

AABB GetAABB(const Triangle & t)
{
	return
	{
		{
			Min(t.a.x, t.b.x, t.c.x),
			Min(t.a.y, t.b.y, t.c.y),
			Min(t.a.z, t.b.z, t.c.z),
		},
		{
			Max(t.a.x, t.b.x, t.c.x),
			Max(t.a.y, t.b.y, t.c.y),
			Max(t.a.z, t.b.z, t.c.z),
		},
	};
}

bool IsIntersect(const Triangle & t, const Vec3f & r)
{
	float side1 = ( r.x - t.a.x ) * ( t.b.y - t.a.y ) - ( r.y - t.a.y ) * ( t.b.x - t.a.x );
	float side2 = ( r.x - t.b.x ) * ( t.c.y - t.b.y ) - ( r.y - t.b.y ) * ( t.c.x - t.b.x );
	float side3 = ( r.x - t.c.x ) * ( t.a.y - t.c.y ) - ( r.y - t.c.y ) * ( t.a.x - t.c.x );
	return (side1 > 0.0f && side2 > 0.0f && side3 > 0.0f) ||
		(side1 < 0.0f && side2 < 0.0f && side3 < 0.0f);
}

int main()
{
	const int WIDTH = 800;
	const int HEIGHT = 600;
	// Output
	static unsigned char frameBuffer[HEIGHT][WIDTH][3];
	static float depthBuffer[HEIGHT][WIDTH] = { 0 };
	memset(frameBuffer, 100, sizeof(frameBuffer));

	// Input
	Camera cam;
	cam.pos = { 0.0f, 0.0f, 0.0f };
	cam.near = 0.1f;
	cam.far = 1000.0f;
	cam.fov = DegreeToRadian(120.0f);
	cam.aspectRatio = 4.0f / 3.0f;

	Triangle triangles[] =
	{
		{
			{0.0f, 0.0f, 1.0f},
			{1.0f, 1.0f, 1.0f},
			{1.0f, 0.0f, 1.0f},
			{1.0f, 0.0f, 0.0f},
		},
		{
			{0.0f, 0.0f, 1.0f},
			{1.0f, 1.0f, 1.0f},
			{0.0f, 1.0f, 1.0f},
			{0.0f, 1.0f, 0.0f},
		},
	};
	constexpr auto numTriangle = sizeof(triangles) / sizeof(Triangle);

	// Projection & Clipping
	// TODO: matrix utils (create world-to-camera, camera-to-clipping, clipping-to-ndc)
	Matrix4x4 cameraToNDC = PerspectiveFovLH(
		cam.fov,
		cam.aspectRatio,
		cam.near,
		cam.far
	);
	// TODO: 0/1-clipping algorithm (give clipping cube, triangle, decide include/exclude)
	// TODO: 0.5-clipping algorithm (give clipping cube, triangle, cut outside part and redo triangulation)
	{
		// 1. World to Camera
		// 2. Camera to Clipping (do clipping)
		// 3. Clipping to NDC
	}

	Triangle trianglesNDC[ numTriangle ];
	for ( int i = 0; i < numTriangle; ++i )
	{
		trianglesNDC[ i ] =
		{
			Multiply(triangles[ i ].a, cameraToNDC),
			Multiply(triangles[ i ].b, cameraToNDC),
			Multiply(triangles[ i ].c, cameraToNDC),
			triangles[ i ].color,
		};
	}

	// == DATA: triangles in NDC ==

	// Rasterization
	{
		// 1. Select pixel
		{
			// 1.1. Compute bounding pixel rect, shoot ray (z-axis)
			// 1.2. Ray-Triangle intersection point
			// 1.3. Compute triangle inner point properties (color, depth)
			{
				// Shading
				{
					// Barycentric Coordinates
				}
			}
			// 1.4. Output pixel and properties
		}

		// == DATA: pixels in Screen3D ==

		// Occlusion Culling
		{
			// 1. Z-Test
		}

		for ( auto & t : trianglesNDC )
		{
			AABB aabb = GetAABB(t);

			// NDC to Screen
			int xMin = static_cast< int >( Bound(0.0f, 0.5f * ( aabb.min.x + 1.0f ), 1.0f) * WIDTH );
			int xMax = static_cast< int >( Bound(0.0f, 0.5f * ( aabb.max.x + 1.0f ), 1.0f) * WIDTH );
			int yMin = static_cast< int >( Bound(0.0f, 0.5f * ( 1.0f - aabb.max.y ), 1.0f) * HEIGHT );
			int yMax = static_cast< int >( Bound(0.0f, 0.5f * ( 1.0f - aabb.min.y ), 1.0f) * HEIGHT );
			for (int y = yMin; y <= yMax; ++y)
			{
				for (int x = xMin; x <= xMax; ++x)
				{
					Vec3f r = { static_cast<float>(x) * 2.0f / WIDTH - 1.0f, 1.0f - static_cast<float>(y) * 2.0f / HEIGHT, 100.0f };
					if (IsIntersect(t, r))
					{
						frameBuffer[y][x][0] = static_cast<unsigned char>(t.color.g * 255.0f);
						frameBuffer[y][x][1] = static_cast<unsigned char>(t.color.b * 255.0f);
						frameBuffer[y][x][2] = static_cast<unsigned char>(t.color.r * 255.0f);
					}
				}
			}
		}
	}

	std::ofstream ofs;
	ofs.open("triangle.ppm");
	ofs << "P6\n" << WIDTH << " " << HEIGHT << "\n255\n";
	ofs.write(( char* ) frameBuffer, WIDTH * HEIGHT * 3);
	ofs.close();


	return 0;
}
