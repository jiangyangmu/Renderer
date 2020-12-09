#include "Renderer.h"

#include <cassert>
#include <fstream>

constexpr auto PI = 3.141592653f;

struct RGB
{
	float r, g, b;
};
struct Vec3
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
	Vec3 a, b, c;
	RGB rgbA, rgbB, rgbC;
};

struct AABB
{
	Vec3 min, max;
};

struct Camera
{
	Vec3 pos;
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

Vec3 Multiply(const Vec3 & v, const Matrix4x4 & m)
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

inline float EdgeFunction(const Vec3 & a, const Vec3 & b, const Vec3 & c)
{
	return ( c.x - a.x ) * ( b.y - a.y ) - ( c.y - a.y ) * ( b.x - a.x );
}
bool IsIntersect(const Triangle & t, const Vec3 & r)
{
	float side1 = ( r.x - t.a.x ) * ( t.b.y - t.a.y ) - ( r.y - t.a.y ) * ( t.b.x - t.a.x );
	float side2 = ( r.x - t.b.x ) * ( t.c.y - t.b.y ) - ( r.y - t.b.y ) * ( t.c.x - t.b.x );
	float side3 = ( r.x - t.c.x ) * ( t.a.y - t.c.y ) - ( r.y - t.c.y ) * ( t.a.x - t.c.x );
	return (side1 >= 0.0f && side2 >= 0.0f && side3 >= 0.0f) ||
		(side1 < 0.0f && side2 < 0.0f && side3 < 0.0f);
}

namespace GraphicsPipeline
{
	class RenderTarget
	{
	public:
		RenderTarget(unsigned int width, unsigned int height, void * backBuffer)
			: m_width(width)
			, m_height(height)
			, m_backBuffer(backBuffer)
		{
		}

		unsigned int	Width()
		{
			return m_width;
		}
		unsigned int	Height()
		{
			return m_height;
		}
		void		SetPixel(unsigned int x, unsigned int y, int r, int g, int b)
		{
			unsigned char * pixel = (unsigned char *)m_backBuffer + (y * m_width + x) * 3;
			assert((pixel + 3) <= ((unsigned char *)m_backBuffer + m_width * m_height * 3));

			pixel[0] = b;
			pixel[1] = g;
			pixel[2] = r;
		}

	private:
		unsigned int	m_width;
		unsigned int	m_height;
		void *		m_backBuffer;
	};

	namespace Shader
	{
		struct Vertex
		{
			Vec3 pos;
			Vec3 color;
		};

		struct Pixel
		{
			Vec3 pos; // x: [0, screen width) y: [0, screen height) z: depth: [0.0f, 1.0f]
			Vec3 color;
		};

		// Compute lighting
		class VertexShader
		{
		public:
			struct Input
			{
				Vertex v;
			};
			struct Output
			{
				Vertex v;
			};

			static Output Shade(Input in)
			{
				return {in.v};
			}
		};

		class PixelShader
		{
		public:
			struct Input
			{
				Pixel p;
			};
			struct Output
			{
				Vec3 color;
			};

			static Output Shade(Input in)
			{
				return {in.p.color};
			}
		};
	}

	void Rasterize(const Triangle & triNDC,
		       RenderTarget & output)
	{

		const unsigned int width = output.Width();
		const unsigned int height = output.Height();

		// NDC to Screen
		AABB aabb = GetAABB(triNDC);
		int xMin = static_cast< int >( Bound(0.0f, 0.5f * ( aabb.min.x + 1.0f ), 1.0f) * width );
		int xMax = static_cast< int >( Bound(0.0f, 0.5f * ( aabb.max.x + 1.0f ), 1.0f) * width );
		int yMin = static_cast< int >( Bound(0.0f, 0.5f * ( 1.0f - aabb.max.y ), 1.0f) * height );
		int yMax = static_cast< int >( Bound(0.0f, 0.5f * ( 1.0f - aabb.min.y ), 1.0f) * height );
		for (int y = yMin; y <= yMax; ++y)
		{
			for (int x = xMin; x <= xMax; ++x)
			{
				Vec3 r = { static_cast<float>(x) * 2.0f / width - 1.0f, 1.0f - static_cast<float>(y) * 2.0f / height, 1000.0f };

				if (IsIntersect(triNDC, r))
				{
					// Triangle interpolation
					float area = EdgeFunction(triNDC.a, triNDC.b, triNDC.c);
					float w0 = EdgeFunction(triNDC.b, triNDC.c, r) / area;
					float w1 = EdgeFunction(triNDC.c, triNDC.a, r) / area;
					float w2 = EdgeFunction(triNDC.a, triNDC.b, r) / area;

					Vec3 pos =
					{
						(float)x,
						(float)y,
						0.0f,
					};
					Vec3 color =
					{
						w0 * triNDC.rgbA.b + w1 * triNDC.rgbB.b + w2 * triNDC.rgbC.b,
						w0 * triNDC.rgbA.g + w1 * triNDC.rgbB.g + w2 * triNDC.rgbC.g,
						w0 * triNDC.rgbA.r + w1 * triNDC.rgbB.r + w2 * triNDC.rgbC.r
					};

					Shader::PixelShader::Input in =
					{
						{
							pos,
							color
						}
					};
					Shader::PixelShader::Output out = Shader::PixelShader::Shade(in);

					output.SetPixel(x,
							y,
							static_cast< unsigned char >( out.color.x * 255.0f),
							static_cast< unsigned char >( out.color.y * 255.0f),
							static_cast< unsigned char >( out.color.z * 255.0f));
				}
			}
		}
	}
}



Renderer::RenderResult Renderer::RenderResult::Create()
{
	// Output
	Renderer::RenderResult output(800, 600);

	unsigned char * pFrameBuffer = (unsigned char *)(output.FrameBuffer());

	memset(pFrameBuffer, 100, output.GetFrameBufferSize());

	// Input
	Camera cam;
	cam.pos = { 0.0f, 0.0f, 0.0f };
	cam.near = 0.1f;
	cam.far = 1000.0f;
	cam.fov = DegreeToRadian(100.0f);
	cam.aspectRatio = 4.0f / 3.0f;

	Triangle triangles[] =
	{
		{
			{0.0f, 0.0f, 1.0f},
			{0.5f, 0.866f, 1.0f},
			{1.0f, 0.0f, 1.0f},

			{1.0f, 0.0f, 0.0f},
			{0.0f, 1.0f, 0.0f},
			{0.0f, 0.0f, 1.0f},
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
			triangles[ i ].rgbA,
			triangles[ i ].rgbB,
			triangles[ i ].rgbC,
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

		GraphicsPipeline::RenderTarget renderTarget(output.Width(),
							    output.Height(),
							    output.FrameBuffer());

		for ( auto & t : trianglesNDC )
		{
			GraphicsPipeline::Rasterize(t,
						    renderTarget);
		}
	}

	return output;
}
