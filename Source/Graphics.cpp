#include "Graphics.h"
#include "Common.h"
#include "SharedTypes.h"
#include "win32/Win32App.h"

#include <vector>
#include <memory>

using Graphics::Camera;
using Graphics::Triangle;

constexpr auto PI = 3.141592653f;

struct BarycentricCoordinate
{
	float a, b, c;
};

struct AABB
{
	Vec3 min, max;
};

// Utilities
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
	return value < min ? min : ( value > max ? max : value );
}
inline float DegreeToRadian(float d)
{
	// 0 < d < 180
	return d * PI / 180.0f;
}

// Vec3 & Matrix
Vec3 Multiply(const Vec3 & v, const Matrix4x4 & m)
{
	float w = m.f14 * v.x + m.f24 * v.y + m.f34 * v.z + m.f44;
	float wInv = ( w == 0.0f ? 0.0f : 1.0f / w );
	return
	{
		( m.f11 * v.x + m.f21 * v.y + m.f31 * v.z + m.f41 ) * wInv,
		( m.f12 * v.x + m.f22 * v.y + m.f32 * v.z + m.f42 ) * wInv,
		( m.f13 * v.x + m.f23 * v.y + m.f33 * v.z + m.f43 ) * wInv,
	};
}

// Helper
AABB GetAABB(const Triangle & triNDC)
{
	return
	{
		{
			Min(triNDC.a.x, triNDC.b.x, triNDC.c.x),
			Min(triNDC.a.y, triNDC.b.y, triNDC.c.y),
			Min(triNDC.a.z, triNDC.b.z, triNDC.c.z),
		},
		{
			Max(triNDC.a.x, triNDC.b.x, triNDC.c.x),
			Max(triNDC.a.y, triNDC.b.y, triNDC.c.y),
			Max(triNDC.a.z, triNDC.b.z, triNDC.c.z),
		},
	};
}
Vec3 GetPixelRay(const Camera & camera, int width, int height, int pixelX, int pixelY)
{
	float nearPlaneYMax = camera.zNear * tanf(0.5f * camera.fov);
	float nearPlaneXMax = camera.aspectRatio * nearPlaneYMax;

	Vec3 ray =
	{
		nearPlaneXMax * ( pixelX * 2.0f / width - 1.0f ),
		nearPlaneYMax * ( 1.0f - pixelY * 2.0f / height ),
		camera.zNear
	};

	return ray;
}
bool RayTriangleIntersection(const Vec3 & ray, const Triangle & tri, float * pDistance, BarycentricCoordinate * pBarycentric)
{
	const Vec3 & p0 = tri.a;
	const Vec3 & p1 = tri.b;
	const Vec3 & p2 = tri.c;

	Vec3 o = Vec3::Zero();
	Vec3 d = Vec3::Normalize(ray);

	Vec3 e1 = p1 - p0;
	Vec3 e2 = p2 - p0;
	Vec3 q = Vec3::Cross(d, e2);
	float a = Vec3::Dot(e1, q);

	if ( -0.00001 < a && a < 0.00001 )
	{
		return false;
	}

	float f = 1.0 / a;

	Vec3 s = o - p0;
	float u = f * Vec3::Dot(s, q);

	if ( u < 0.0 )
	{
		return false;
	}

	Vec3 r = Vec3::Cross(s, e1);
	float v = f * Vec3::Dot(d, r);

	if ( v < 0.0 || ( ( double ) u + v ) > 1.0 )
	{
		return false;
	}

	float t = f * Vec3::Dot(e2, r);

	*pDistance = t;
	pBarycentric->a = 1.0f - u - v;
	pBarycentric->b = u;
	pBarycentric->c = v;

	return true;
}

namespace Graphics
{
	Buffer::Buffer()
		: m_width(0)
		, m_height(0)
		, m_elementSize(0)
		, m_sizeInBytes(0)
		, m_data(nullptr)
	{
	}

	Buffer::Buffer(unsigned int width, unsigned int height, unsigned int elementSize, unsigned int alignment)
		: m_width(width)
		, m_height(height)
		, m_elementSize(elementSize)
		, m_sizeInBytes(0)
		, m_data(nullptr)
	{
		m_data = ( Byte * ) _aligned_malloc(width * height * elementSize, alignment);
		if ( m_data )
		{
			m_sizeInBytes = width * height * elementSize;
		}
	}

	Buffer::~Buffer()
	{
		if ( m_data )
		{
			_aligned_free(m_data);
		}
		m_data = nullptr;
	}

	Buffer::Buffer(Buffer && other)
		: m_width(other.m_width)
		, m_height(other.m_height)
		, m_elementSize(other.m_elementSize)
		, m_sizeInBytes(other.m_sizeInBytes)
		, m_data(other.m_data)
	{
		other.m_width = 0;
		other.m_height = 0;
		other.m_sizeInBytes = 0;
		other.m_data = nullptr;
	}

	Buffer & Buffer::operator=(Buffer && other)
	{
		this->~Buffer();
		new ( this ) Buffer(std::move(other));
		return *this;
	}

	void Buffer::SetAll(Byte value)
	{
		if ( m_data )
		{
			memset(m_data, value, m_sizeInBytes);
		}
	}

	void Buffer::Reshape(unsigned int width, unsigned int height)
	{
		ASSERT(m_width * m_height <= width * height);
		m_width = width;
		m_height = height;
		m_sizeInBytes = m_width * m_height * m_elementSize;
	}

	unsigned int Buffer::Width() const
	{
		return m_width;
	}

	unsigned int Buffer::Height() const
	{
		return m_height;
	}

	unsigned int Buffer::SizeInBytes() const
	{
		return m_sizeInBytes;
	}

	unsigned int Buffer::ElementCount() const
	{
		return m_width * m_height;
	}

	unsigned int Buffer::ElementSize() const
	{
		return m_elementSize;
	}

	void * Buffer::Data()
	{
		return m_data;
	}

	const void * Buffer::Data() const
	{
		return m_data;
	}

	void * Buffer::At(unsigned int row, unsigned int col)
	{
		ASSERT(m_data);
		return m_data + ( row * m_width + col ) * m_elementSize;
	}

	const void * Buffer::At(unsigned int row, unsigned int col) const
	{
		ASSERT(m_data);
		return m_data + ( row * m_width + col ) * m_elementSize;
	}

	Texture2D::Texture2D(const Buffer & bitmap)
		: m_bitmap(bitmap)
	{
	}

	void Texture2D::Sample(float u, float v, float * rgb) const
	{
		ASSERT(0.0f <= u && u <= 1.0f);
		ASSERT(0.0f <= v && v <= 1.0f);
		ASSERT(u + v <= 2.0f);

		LONG width = m_bitmap.Width();
		LONG height = m_bitmap.Height();
		
		LONG col = static_cast< LONG >( width * u );
		LONG row = static_cast< LONG >( height * ( 1.0f - v ) );

		const Byte * bgra = (Byte * )m_bitmap.At(row, col);
		rgb[ 0 ] = static_cast< float >( bgra[ 2 ] ) / 255.f;
		rgb[ 1 ] = static_cast< float >( bgra[ 1 ] ) / 255.f;
		rgb[ 2 ] = static_cast< float >( bgra[ 0 ] ) / 255.f;
	}

	Transform::Transform(const Camera & camera)
	{
		m_worldToCamera = Matrix4x4::Identity();
		m_cameraToNDC = Matrix4x4::PerspectiveFovLH(camera.fov,
							    camera.aspectRatio,
							    camera.zNear,
							    camera.zFar);
	}

	Vec3 Transform::WorldToCamera(const Vec3 & v) const
	{
		return Multiply(v, m_worldToCamera);
	}

	Vec3 Transform::CameraToNDC(const Vec3 & v) const
	{
		return Multiply(v, m_cameraToNDC);
	}

	TransformTriangle::TransformTriangle(const Transform & transform, const Triangle & triangle)
	{
		m_triangle[ 0 ] = triangle;
		m_triangle[ 1 ] =
		{
			transform.WorldToCamera(triangle.a),
			transform.WorldToCamera(triangle.b),
			transform.WorldToCamera(triangle.c),
			triangle.rgbA,
			triangle.rgbB,
			triangle.rgbC,
			triangle.uvA,
			triangle.uvB,
			triangle.uvC,
		};
		m_triangle[ 2 ] =
		{
			transform.CameraToNDC(m_triangle[ 1 ].a),
			transform.CameraToNDC(m_triangle[ 1 ].b),
			transform.CameraToNDC(m_triangle[ 1 ].c),
			triangle.rgbA,
			triangle.rgbB,
			triangle.rgbC,
			triangle.uvA,
			triangle.uvB,
			triangle.uvC,
		};
	}

	const Triangle & TransformTriangle::GetWorldSpace()
	{
		return m_triangle[ 0 ];
	}

	const Triangle & TransformTriangle::GetCameraSpace()
	{
		return m_triangle[ 1 ];
	}

	const Triangle & TransformTriangle::GetNDCSpace()
	{
		return m_triangle[ 2 ];
	}

	RenderTarget::RenderTarget(unsigned int width, unsigned int height, void * backBuffer) : m_width(width)
		, m_height(height)
		, m_backBuffer(backBuffer)
	{

	}

	unsigned int RenderTarget::Width()
	{
		return m_width;
	}

	unsigned int RenderTarget::Height()
	{
		return m_height;
	}

	void RenderTarget::SetPixel(unsigned int x, unsigned int y, int r, int g, int b)
	{
		unsigned char * pixel = ( unsigned char * ) m_backBuffer + ( y * m_width + x ) * 3;
		ASSERT(( pixel + 3 ) <= ( ( unsigned char * ) m_backBuffer + m_width * m_height * 3 ));

		pixel[ 0 ] = b;
		pixel[ 1 ] = g;
		pixel[ 2 ] = r;
	}

	void Rasterizer::Rasterize(const unsigned int width, const unsigned int height, const Camera & camera, const Triangle & triangle, const Transform & transform, RasterizerCallback rasterizerCB)
	{
		TransformTriangle transTriangle(transform, triangle);

		// Pick pixels with AABB and ray-triangle intersection test.

		AABB aabb = GetAABB(transTriangle.GetNDCSpace());


		int pixelXRange[ 2 ];
		int pixelYRange[ 2 ];
		pixelXRange[ 0 ] = static_cast< int >( Bound(0.0f, 0.5f * ( aabb.min.x + 1.0f ), 1.0f) * width );
		pixelXRange[ 1 ] = static_cast< int >( Bound(0.0f, 0.5f * ( aabb.max.x + 1.0f ), 1.0f) * width );
		pixelYRange[ 0 ] = static_cast< int >( Bound(0.0f, 0.5f * ( 1.0f - aabb.max.y ), 1.0f) * height );
		pixelYRange[ 1 ] = static_cast< int >( Bound(0.0f, 0.5f * ( 1.0f - aabb.min.y ), 1.0f) * height );

		for ( int y = pixelYRange[ 0 ]; y <= pixelYRange[ 1 ]; ++y )
		{
			int lastIntersectX = INT_MAX;
			for ( int x = pixelXRange[ 0 ]; x <= pixelXRange[ 1 ]; ++x )
			{
				using DB::GlbDebugPixel;
				if ( GlbDebugPixel[ 0 ] >= 0 )
				{
					if ( x != GlbDebugPixel[ 0 ] || y != GlbDebugPixel[ 1 ] )
					{
						continue;
					}
					else
					{
						// Set break point here.
						GlbDebugPixel[ 0 ] = GlbDebugPixel[ 1 ] = -1;
					}
				}

				// Compute pixel properties

				float distance;
				BarycentricCoordinate barycentric;
				if ( !RayTriangleIntersection(GetPixelRay(camera, width, height, x, y),
							      transTriangle.GetCameraSpace(),
							      &distance,
							      &barycentric) )
				{
					if ( lastIntersectX < x ) break;
					else continue;
				}
				lastIntersectX = x;

				RGB color;
				{
					//const RGB & cA = transTriangle.GetWorldSpace().rgbA;
					//const RGB & cB = transTriangle.GetWorldSpace().rgbB;
					//const RGB & cC = transTriangle.GetWorldSpace().rgbC;
					//
					//color =
					//{
					//	barycentric.a * cA.r + barycentric.b * cB.r + barycentric.c * cC.r,
					//	barycentric.a * cA.g + barycentric.b * cB.g + barycentric.c * cC.g,
					//	barycentric.a * cA.b + barycentric.b * cB.b + barycentric.c * cC.b
					//};

					const Vec2 & tA = transTriangle.GetWorldSpace().uvA;
					const Vec2 & tB = transTriangle.GetWorldSpace().uvB;
					const Vec2 & tC = transTriangle.GetWorldSpace().uvC;

					float u = barycentric.a * tA.x + barycentric.b * tB.x + barycentric.c * tC.x;
					float v = barycentric.a * tA.y + barycentric.b * tB.y + barycentric.c * tC.y;

					float rgb[ 3 ];
					DB::Textures::Duang().Sample(u, v, rgb);
					color = { rgb[ 0 ], rgb[ 1 ], rgb[ 2 ] };
				}
				Vec3 pos;
				{
					float depthNDC = ( distance - camera.zNear ) / ( camera.zFar - camera.zNear ); // FIXIT: this is not perspective correct

					pos =
					{
						static_cast< float >( x ),
						static_cast< float >( y ),
						depthNDC
					};
				}

				// Draw pixel
				rasterizerCB(x, y, pos, color);
			}
		}
	}

	namespace DB
	{
		int GlbDebugPixel[ 2 ] = { -1, -1 };

		const Texture2D & Textures::Duang()
		{
			static Buffer bitmapData;
			static Texture2D texture(bitmapData);

			if ( !bitmapData.Data() )
			{
				LONG width, height;
				LPVOID data;

				win32::LoadBMP(L"Resources/duang.bmp", &width, &height, &data);
				bitmapData = Buffer(width, height, 4, 4);
				
				memcpy(bitmapData.Data(), data, width * height * 4);

				delete[] data;
			}

			return texture;
		}

		const Camera & DefaultCamera()
		{
			static Camera camera;
			static bool init = false;

			if ( !init )
			{
				init = true;
				camera.pos = { 0.0f, 0.0f, 0.0f };
				camera.zNear = 0.1f;
				camera.zFar = 1000.0f; // use smaller value for better depth test.
				camera.fov = DegreeToRadian(90.0f);
				camera.aspectRatio = 4.0f / 3.0f;
			}

			return camera;
		}
	}
}