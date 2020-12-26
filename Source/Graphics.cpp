#include "Graphics.h"
#include "Common.h"
#include "win32/Win32App.h"

#include <vector>
#include <memory>

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
template <typename T>
inline T Bound(T min, T value, T max)
{
	return value < min ? min : ( value > max ? max : value );
}
inline float EdgeFunction(const Vec2 & a, const Vec2 & b, const Vec2 & c)
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
		, m_rowSizeInBytes(0)
		, m_data(nullptr)
	{
	}

	Buffer::Buffer(Integer width, Integer height, Integer elementSize, Integer alignment, Integer rowPadding)
		: m_width(width)
		, m_height(height)
		, m_elementSize(elementSize)
		, m_sizeInBytes(0)
		, m_rowSizeInBytes(0)
		, m_data(nullptr)
	{
		m_data = ( Byte * ) _aligned_malloc(height * (width * elementSize + rowPadding), alignment);
		if ( m_data )
		{
			m_rowSizeInBytes = width * elementSize + rowPadding;
			m_sizeInBytes = height * m_rowSizeInBytes;
			ZeroMemory(m_data, m_sizeInBytes);
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
		, m_rowSizeInBytes(other.m_rowSizeInBytes)
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

	Integer Buffer::RowSizeInBytes() const
	{
		return m_rowSizeInBytes;
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
		return m_data + row * m_rowSizeInBytes + col * m_elementSize;
	}

	const void * Buffer::At(Integer row, Integer col) const
	{
		ASSERT(m_data);
		return m_data + row * m_rowSizeInBytes + col * m_elementSize;
	}

	Texture2D::Texture2D(const Buffer & bitmap)
		: m_bitmap(bitmap)
	{
	}

	RenderTarget::RenderTarget(Integer width, Integer height, Integer rowSizeInBytes, void * backBuffer)
		: m_width(width)
		, m_height(height)
		, m_rowSizeInBytes(rowSizeInBytes)
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
		unsigned char * pixel = ( unsigned char * ) m_backBuffer + y * m_rowSizeInBytes + x * 3;
		ASSERT(( pixel + 3 ) <= ( ( unsigned char * ) m_backBuffer + m_height * m_rowSizeInBytes ));

		pixel[ 0 ] = b;
		pixel[ 1 ] = g;
		pixel[ 2 ] = r;
	}

	void Rasterize(Pipeline::Context & context,
		       const Pipeline::Vertex * pVertexBuffer,
		       Integer nVertex,
		       Pipeline::VertexFormat vertexFormat)
	{
		using Vertex = Graphics::Pipeline::Vertex;

		RenderTarget renderTarget = context.GetRenderTarget();
		Buffer & depthBuffer = context.GetDepthBuffer();

		Integer width = renderTarget.Width();
		Integer height = renderTarget.Height();

		const Matrix4x4 & wldToCam = context.GetConstants().WorldToCamera;
		const Matrix4x4 & camToNDC = context.GetConstants().CameraToNDC;

		Integer dbgX = context.GetConstants().DebugPixel[ 0 ];
		Integer dbgY = context.GetConstants().DebugPixel[ 1 ];

		for (const Pipeline::Vertex * pVertex = pVertexBuffer;
		     nVertex >= 3;
		     nVertex -= 3, pVertex += 3 )
		{
			// World(Wld) -> Camera(Cam) -> NDC -> Screen(Scn)+Depth -> Raster(Ras)+Depth
			const Vertex & v0 = pVertex[ 0 ];
			const Vertex & v1 = pVertex[ 1 ];
			const Vertex & v2 = pVertex[ 2 ];

			const Vec3 & p0Wld = v0.pos;
			const Vec3 & p1Wld = v1.pos;
			const Vec3 & p2Wld = v2.pos;

			Vec3 p0Cam = Vec3::Transform(p0Wld, wldToCam);
			Vec3 p1Cam = Vec3::Transform(p1Wld, wldToCam);
			Vec3 p2Cam = Vec3::Transform(p2Wld, wldToCam);

			//if (p0Cam.z < 0.0f && p1Cam.z < 0.0f && p2Cam.z < 0.0f) continue;

			float z0CamInv = 1.0f / p0Cam.z;
			float z1CamInv = 1.0f / p1Cam.z;
			float z2CamInv = 1.0f / p2Cam.z;

			Vec3 p0NDC = Vec3::Transform(p0Cam, camToNDC);
			Vec3 p1NDC = Vec3::Transform(p1Cam, camToNDC);
			Vec3 p2NDC = Vec3::Transform(p2Cam, camToNDC);

			//float z0NDCInv = 1.0f / p0NDC.z;
			//float z1NDCInv = 1.0f / p1NDC.z;
			//float z2NDCInv = 1.0f / p2NDC.z;

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

			// ASSERT(0 <= xRasMin && xRasMin < width);
			// ASSERT(0 <= yRasMin && yRasMin < height);
			if (xRasMin < 0 || width <= xRasMax || yRasMin < 0 || height <= yRasMax)
			{
				continue;
			}

			float areaInv = EdgeFunction(p0Ras, p1Ras, p2Ras);
			areaInv = ( areaInv < 0.0001f ) ? 1000.0f : 1.0f / areaInv;
			ASSERT(areaInv >= 0.0f);

			for ( Integer y = yRasMin; y <= yRasMax; ++y )
			{
				for ( Integer x = xRasMin; x <= xRasMax; ++x )
				{
					//renderTarget.SetPixel(x, y, 100, 100, 100);

					Vec2 pixel = { x, y };

					if ( dbgX != -1 && dbgY != -1 && ( x != dbgX || y != dbgY ) ) continue;

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
					ASSERT(0.0f <= bary0 && bary0 <= 1.0001f);
					ASSERT(0.0f <= bary1 && bary1 <= 1.0001f);
					ASSERT(0.0f <= bary2 && bary2 <= 1.0001f);
					ASSERT((bary0 + bary1 + bary2) <= 1.0001f);
					
					// Depth
					float zNDC = p0NDC.z * bary0 + p1NDC.z * bary1 + p2NDC.z * bary2;
					// ASSERT(0.0f <= zNDC && zNDC <= 1.0001f);
					if (!(0.0f <= zNDC && zNDC <= 1.0001f))
					{
						continue;
					}

					float * depth = static_cast< float * >( depthBuffer.At(y, x) );
					if ( *depth <= zNDC )
					{
						continue;
					}
					*depth = zNDC;

					// Vertex properties
					float zCam = 1.0f / ( z0CamInv * bary0 + z1CamInv * bary1 + z2CamInv * bary2 );
					float zz0 = zCam * z0CamInv;
					float zz1 = zCam * z1CamInv;
					float zz2 = zCam * z2CamInv;

					RGB color;
					if ( vertexFormat == Pipeline::VertexFormat::POSITION_RGB )
					{
						const RGB & cA = v0.color;
						const RGB & cB = v1.color;
						const RGB & cC = v2.color;

						color =
						{
							zz0 * bary0 * cA.r + zz1 * bary1 * cB.r + zz2 * bary2 * cC.r,
							zz0 * bary0 * cA.g + zz1 * bary1 * cB.g + zz2 * bary2 * cC.g,
							zz0 * bary0 * cA.b + zz1 * bary1 * cB.b + zz2 * bary2 * cC.b
						};
					}
					else
					{
						const Vec2 & tA = v0.uv;
						const Vec2 & tB = v1.uv;
						const Vec2 & tC = v2.uv;

						float u = zz0 * bary0 * tA.x + zz1 * bary1 * tB.x + zz2 * bary2 * tC.x;
						float v = zz0 * bary0 * tA.y + zz1 * bary1 * tB.y + zz2 * bary2 * tC.y;

						float rgb[ 3 ];
						DB::Textures::Duang().Sample(u, v, rgb);
						color = { rgb[ 0 ], rgb[ 1 ], rgb[ 2 ] };
					}
					ASSERT(color.r >= 0.0f && color.g >= 0.0f && color.b >= 0.0f);
					ASSERT(color.r <= 1.0001f && color.g <= 1.0001f && color.b <= 1.0001f);

					// Draw pixel
					renderTarget.SetPixel(x,
							      y,
							      static_cast< Byte >( color.r * 255.0f ),
							      static_cast< Byte >( color.g * 255.0f ),
							      static_cast< Byte >( color.b * 255.0f ));
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

		const std::vector<std::vector<Vertex>> & Triangles::One()
		{
			static std::vector<std::vector<Vertex>> vertices =
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

		const std::vector<std::vector<Vertex>> & Triangles::Two()
		{
			static std::vector<std::vector<Vertex>> vertices =
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

		const std::vector<std::vector<Vertex>> & Triangles::IntersectionTest()
		{
			static std::vector<std::vector<Vertex>> vertices =
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

		const std::vector<std::vector<Vertex>> & Triangles::TextureTest()
		{
			static std::vector<std::vector<Vertex>> vertices =
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

		const std::vector<std::vector<Graphics::Pipeline::Vertex>> & Triangles::PerspectiveProjectionTest()
		{
			static std::vector<std::vector<Vertex>> vertices =
			{
				// RGB
				{
					Pipeline::MakeVertex({-1.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}),
					Pipeline::MakeVertex({ 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}),
					Pipeline::MakeVertex({-0.5f, 3.0f, 5.0f}, {0.0f, 0.0f, 1.0f}),
				},
				// Texture
				{
					Pipeline::MakeVertex({0.0f, 0.0f, 1.0f}, Vec2{0.0f, 0.0f}),
					Pipeline::MakeVertex({1.0f, 0.0f, 1.0f}, Vec2{1.0f, 0.0f}),
					Pipeline::MakeVertex({0.0f, 1.0f, 1.7f}, Vec2{0.0f, 1.0f}),
					Pipeline::MakeVertex({1.0f, 1.0f, 1.7f}, Vec2{1.0f, 1.0f}),
					Pipeline::MakeVertex({0.0f, 1.0f, 1.7f}, Vec2{0.0f, 1.0f}),
					Pipeline::MakeVertex({1.0f, 0.0f, 1.0f}, Vec2{1.0f, 0.0f}),
				}
			};
			return vertices;
		}

		const std::vector<std::vector<Vertex>> & Triangles::CameraTest()
		{
			static std::vector<std::vector<Vertex>> vertices;

			if (vertices.empty())
			{
				std::vector<Vertex> v;

				constexpr int len = 5;
				constexpr float edge = 1.0f;

				float d = 0.5f * edge;
				v.resize(( len + 1 ) * ( len + 1 ) * 6);
				for ( int x = -len; x <= len; ++x )
				{
					for ( int z = -len; z <= len; ++z )
					{
						RGB color = ( ( x + z ) % 2 == 0 ) ? RGB { 0.0f, 0.8f, 0.0f } : RGB { 1.0f, 1.0f, 1.0f };

						std::vector<Vertex> square =
						{
							Pipeline::MakeVertex({ x-d, -1.0f, z-d }, color),
							Pipeline::MakeVertex({ x+d, -1.0f, z-d }, color),
							Pipeline::MakeVertex({ x+d, -1.0f, z+d }, color),
							Pipeline::MakeVertex({ x-d, -1.0f, z-d }, color),
							Pipeline::MakeVertex({ x+d, -1.0f, z+d }, color),
							Pipeline::MakeVertex({ x-d, -1.0f, z+d }, color),
						};
						v.insert(v.end(), square.begin(), square.end());
					}
				}

				vertices.resize(2);
				vertices[ 0 ] = std::move(v);
			}

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