#include "Graphics.h"
#include "Common.h"
#include "SharedTypes.h"
#include "win32/Win32App.h"

#include <vector>
#include <memory>

constexpr auto PI = 3.141592653f;

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
inline float Bound(float min, float value, float max)
{
	return value < min ? min : ( value > max ? max : value );
}
inline float DegreeToRadian(float d)
{
	// 0 < d < 180
	return d * PI / 180.0f;
}
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
float EdgeFunction(const Vec2 & a, const Vec2 & b, const Vec2 & c)
{
	return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
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

	Buffer::Buffer(Integer width, Integer height, Integer elementSize, Integer alignment)
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
			m_data = nullptr;
		}
	}

	Buffer::Buffer(Buffer && other)
		: m_width(other.m_width)
		, m_height(other.m_height)
		, m_elementSize(other.m_elementSize)
		, m_sizeInBytes(other.m_sizeInBytes)
		, m_data(other.m_data)
	{
		new ( &other ) Buffer();
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

	void Buffer::Reshape(Integer width, Integer height)
	{
		ASSERT(m_width * m_height <= width * height);
		m_width = width;
		m_height = height;
		m_sizeInBytes = m_width * m_height * m_elementSize;
	}

	Integer Buffer::Width() const
	{
		return m_width;
	}

	Integer Buffer::Height() const
	{
		return m_height;
	}

	Integer Buffer::SizeInBytes() const
	{
		return m_sizeInBytes;
	}

	Integer Buffer::ElementCount() const
	{
		return m_width * m_height;
	}

	Integer Buffer::ElementSize() const
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

	void * Buffer::At(Integer row, Integer col)
	{
		ASSERT(m_data);
		return m_data + ( row * m_width + col ) * m_elementSize;
	}

	const void * Buffer::At(Integer row, Integer col) const
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
		LONG row = static_cast< LONG >( height * v );

		const Byte * bgra = (Byte * )m_bitmap.At(row, col);
		rgb[ 0 ] = static_cast< float >( bgra[ 2 ] ) / 255.f;
		rgb[ 1 ] = static_cast< float >( bgra[ 1 ] ) / 255.f;
		rgb[ 2 ] = static_cast< float >( bgra[ 0 ] ) / 255.f;
	}

	RenderTarget::RenderTarget(Integer width, Integer height, void * backBuffer) : m_width(width)
		, m_height(height)
		, m_backBuffer(backBuffer)
	{

	}

	Integer RenderTarget::Width()
	{
		return m_width;
	}

	Integer RenderTarget::Height()
	{
		return m_height;
	}

	void RenderTarget::SetPixel(Integer x, Integer y, Byte r, Byte g, Byte b)
	{
		unsigned char * pixel = ( unsigned char * ) m_backBuffer + ( y * m_width + x ) * 3;
		ASSERT(( pixel + 3 ) <= ( ( unsigned char * ) m_backBuffer + m_width * m_height * 3 ));

		pixel[ 0 ] = b;
		pixel[ 1 ] = g;
		pixel[ 2 ] = r;
	}

	void Rasterize(Pipeline::Context & context,
		       Pipeline::Vertex * pVertexBuffer,
		       Integer nVertex,
		       Pipeline::VertexFormat vertexFormat)
	{
		using Vertex = Graphics::Pipeline::Vertex;

		for ( Pipeline::Vertex * pVertex = pVertexBuffer;
		     nVertex >= 3;
		     nVertex -= 3, pVertex += 3 )
		{
			// World(Wld) -> Camera(Cam) -> NDC -> Screen(Scn)+Depth -> Raster(Ras)+Depth

			RenderTarget renderTarget = context.GetRenderTarget();
			Buffer & depthBuffer = context.GetDepthBuffer();

			Integer width = renderTarget.Width();
			Integer height = renderTarget.Height();

			const Matrix4x4 & wldToCam = context.GetConstants().WorldToCamera;
			const Matrix4x4 & camToNDC = context.GetConstants().CameraToNDC;

			const Vertex & v0 = pVertex[ 0 ];
			const Vertex & v1 = pVertex[ 1 ];
			const Vertex & v2 = pVertex[ 2 ];

			const Vec3 & p0Wld = v0.pos;
			const Vec3 & p1Wld = v1.pos;
			const Vec3 & p2Wld = v2.pos;

			Vec3 p0Cam = Multiply(p0Wld, wldToCam);
			Vec3 p1Cam = Multiply(p1Wld, wldToCam);
			Vec3 p2Cam = Multiply(p2Wld, wldToCam);

			Vec3 p0NDC = Multiply(p0Cam, camToNDC);
			Vec3 p1NDC = Multiply(p1Cam, camToNDC);
			Vec3 p2NDC = Multiply(p2Cam, camToNDC);

			float z0NDCInv = 1.0f / p0NDC.z;
			float z1NDCInv = 1.0f / p1NDC.z;
			float z2NDCInv = 1.0f / p2NDC.z;

			Vec2 p0Scn = { ( p0NDC.x + 1.0f ) * 0.5f, ( 1.0f - p0NDC.y ) * 0.5f };
			Vec2 p1Scn = { ( p1NDC.x + 1.0f ) * 0.5f, ( 1.0f - p1NDC.y ) * 0.5f };
			Vec2 p2Scn = { ( p2NDC.x + 1.0f ) * 0.5f, ( 1.0f - p2NDC.y ) * 0.5f };

			Vec2 p0Ras = { p0Scn.x * width, p0Scn.y * height };
			Vec2 p1Ras = { p1Scn.x * width, p1Scn.y * height };
			Vec2 p2Ras = { p2Scn.x * width, p2Scn.y * height };

			Integer xRasMin = static_cast< Integer >( Min3(p0Ras.x, p1Ras.x, p2Ras.x) );
			Integer xRasMax = static_cast< Integer >( Max3(p0Ras.x, p1Ras.x, p2Ras.x) );
			Integer yRasMin = static_cast< Integer >( Min3(p0Ras.y, p1Ras.y, p2Ras.y) );
			Integer yRasMax = static_cast< Integer >( Max3(p0Ras.y, p1Ras.y, p2Ras.y) );

			float areaInv = EdgeFunction(p0Ras, p1Ras, p2Ras);
			areaInv = ( areaInv < 0.0001f ) ? 1000.0f : 1.0f / areaInv;
			ASSERT(areaInv >= 0.0f);

			for ( Integer y = yRasMin; y <= yRasMax; ++y )
			{
				int lastIntersectX = INT_MAX;
				for ( Integer x = xRasMin; x <= xRasMax; ++x )
				{
					Vec2 pixel = { x, y };

					// Barycentric coordinate
					float e0 = EdgeFunction(p1Ras, p2Ras, pixel);
					float e1 = EdgeFunction(p2Ras, p0Ras, pixel);
					float e2 = EdgeFunction(p0Ras, p1Ras, pixel);
					if ( e0 < 0 || e1 < 0 || e2 < 0 || ( e0 == 0 && e1 == 0 && e2 == 0 ) )
					{
						continue;
					}
					float bary0 = e0 * areaInv;
					float bary1 = e1 * areaInv;
					float bary2 = e2 * areaInv;
					ASSERT(0.0f <= bary0 && bary0 <= 1.0f);
					ASSERT(0.0f <= bary1 && bary1 <= 1.0f);
					ASSERT(0.0f <= bary2 && bary2 <= 1.0f);

					// Depth
					float zNDC = 1.0f / ( z0NDCInv * bary0 + z1NDCInv * bary1 + z2NDCInv * bary2 );
					float * depth = static_cast< float * >( depthBuffer.At(y, x) );
					if ( *depth <= zNDC )
					{
						continue;
					}
					*depth = zNDC;

					// Vertex properties
					RGB color;
					if ( vertexFormat == Pipeline::VertexFormat::POSITION_RGB )
					{
						const RGB & cA = v0.color;
						const RGB & cB = v1.color;
						const RGB & cC = v2.color;

						color =
						{
							bary0 * cA.r + bary1 * cB.r + bary2 * cC.r,
							bary0 * cA.g + bary1 * cB.g + bary2 * cC.g,
							bary0 * cA.b + bary1 * cB.b + bary2 * cC.b
						};
					}
					else
					{
						const Vec2 & tA = v0.uv;
						const Vec2 & tB = v1.uv;
						const Vec2 & tC = v2.uv;

						float u = bary0 * tA.x + bary1 * tB.x + bary2 * tC.x;
						float v = bary0 * tA.y + bary1 * tB.y + bary2 * tC.y;

						float rgb[ 3 ];
						DB::Textures::Duang().Sample(u, v, rgb);
						color = { rgb[ 0 ], rgb[ 1 ], rgb[ 2 ] };
					}
					Vec3 pos =
					{
						static_cast< float >( x ),
						static_cast< float >( y ),
						zNDC
					};

					// Draw pixel
					renderTarget.SetPixel(x,
							      y,
							      static_cast< unsigned char >( color.r * 255.0f ),
							      static_cast< unsigned char >( color.g * 255.0f ),
							      static_cast< unsigned char >( color.b * 255.0f ));
				}
			}
		}
	}

	namespace DB
	{
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

		std::vector<std::vector<Vertex>> Triangles::One()
		{
			std::vector<std::vector<Vertex>> vertices =
			{
				// RGB
				{
					Pipeline::MakeVertex({0.0f,   0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}),
					Pipeline::MakeVertex({1.0f,   0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}),
					Pipeline::MakeVertex({0.5f, 0.866f, 1.0f}, {0.0f, 0.0f, 1.0f}),
				},
				// Texture
				{}
			};
			return vertices;
		}

		std::vector<std::vector<Vertex>> Triangles::Two()
		{
			std::vector<std::vector<Vertex>> vertices =
			{
				// RGB
				{
					Pipeline::MakeVertex({0.0f,   0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}),
					Pipeline::MakeVertex({1.0f,   0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}),
					Pipeline::MakeVertex({0.5f, 0.866f, 1.0f}, {0.0f, 0.0f, 1.0f}),
					Pipeline::MakeVertex({-1.0f,   0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}),
					Pipeline::MakeVertex({ 0.0f,   0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}),
					Pipeline::MakeVertex({-0.5f, 0.866f, 1.0f}, {0.0f, 0.0f, 1.0f}),
				},
				// Texture
				{}
			};
			return vertices;
		}

		std::vector<std::vector<Vertex>> Triangles::TwoIntersect()
		{
			std::vector<std::vector<Vertex>> vertices =
			{
				// RGB
				{
					Pipeline::MakeVertex({-1.0f, -0.5f, 1.0f}, {0.0f, 0.0f, 1.0f}),
					Pipeline::MakeVertex({ 0.5f,  0.0f, 0.5f}, {0.0f, 1.0f, 0.0f}),
					Pipeline::MakeVertex({-1.0f,  0.5f, 1.0f}, {1.0f, 0.0f, 0.0f}),
					Pipeline::MakeVertex({ 1.0f, -0.5f, 1.0f}, {0.0f, 0.0f, 1.0f}),
					Pipeline::MakeVertex({ 1.0f,  0.5f, 1.0f}, {0.0f, 1.0f, 0.0f}),
					Pipeline::MakeVertex({-0.5f,  0.0f, 0.5f}, {1.0f, 0.0f, 0.0f}),
				},
				// Texture
				{}
			};
			return vertices;
		}

		std::vector<std::vector<Vertex>> Triangles::TextureTest()
		{
			std::vector<std::vector<Vertex>> vertices =
			{
				// RGB
				{},
				// Texture
				{
					Pipeline::MakeVertex({0.0f, 0.0f, 1.0f}, Vec2{0.0f, 0.0f}),
					Pipeline::MakeVertex({1.0f, 0.0f, 1.0f}, Vec2{1.0f, 0.0f}),
					Pipeline::MakeVertex({0.0f, 1.0f, 1.0f}, Vec2{0.0f, 1.0f}),
					Pipeline::MakeVertex({1.0f, 1.0f, 1.0f}, Vec2{1.0f, 1.0f}),
					Pipeline::MakeVertex({0.0f, 1.0f, 1.0f}, Vec2{0.0f, 1.0f}),
					Pipeline::MakeVertex({1.0f, 0.0f, 1.0f}, Vec2{1.0f, 0.0f}),
				}
			};
			return vertices;
		}

	}

	Pipeline::Vertex Pipeline::MakeVertex(Vec3 pos, RGB color)
	{
		Vertex v;
		v.pos = pos;
		v.color = color;
		return v;
	}

	Pipeline::Vertex Pipeline::MakeVertex(Vec3 pos, Vec2 uv)
	{
		Vertex v;
		v.pos = pos;
		v.uv = uv;
		return v;
	}

}