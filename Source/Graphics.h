#pragma once

#include "SharedTypes.h"

#include <functional>
#include <memory>
#include <vector>

namespace Graphics
{
	struct RGB
	{
		float r, g, b;
	};

	struct Camera
	{
		Vec3 pos;
		float zNear, zFar;
		float fov; // in radian
		float aspectRatio;
	};

	class Buffer
	{
	public:
		Buffer();
		Buffer(Integer width, Integer height, Integer elementSize, Integer alignment = 1);
		~Buffer();

		Buffer(const Buffer &) = delete;
		Buffer(Buffer && other);
		Buffer & operator = (const Buffer &) = delete;
		Buffer & operator = (Buffer && other);

		// Operations

		void		SetAll(Byte value);
		void		Reshape(Integer width, Integer height);

		// Properties

		Integer		Width() const;
		Integer		Height() const;
		Integer		SizeInBytes() const;
		Integer		ElementCount() const;
		Integer		ElementSize() const;
		const void *	Data() const;
		void *		Data();
		const void *	At(Integer row, Integer col) const;
		void *		At(Integer row, Integer col);

	private:
		Integer		m_width;
		Integer		m_height;
		Integer		m_elementSize;
		Integer		m_sizeInBytes;
		Byte *		m_data;
	};

	class Texture2D
	{
	public:
		// Constructors
		
		Texture2D(const Buffer & bitmap);

		Texture2D(const Texture2D &) = default;
		Texture2D(Texture2D &&) = default;
		Texture2D & operator = (const Texture2D &) = default;
		Texture2D & operator = (Texture2D &&) = default;

		// Operations

		void Sample(float u, float v, float * rgb) const;

	private:
		const Buffer &	m_bitmap;
	};

	class RenderTarget
	{
	public:
		RenderTarget(Integer width, Integer height, void * backBuffer);

		Integer		Width();
		Integer		Height();
		void		SetPixel(Integer x, Integer y, Byte r, Byte g, Byte b);

	private:
		Integer		m_width;
		Integer		m_height;
		void *		m_backBuffer;
	};

	namespace Pipeline
	{
		typedef int64_t TextureId;

		class Context
		{
		public:
			struct Constants
			{
				Matrix4x4	WorldToCamera;
				Matrix4x4	CameraToNDC;
				int		DebugPixel[2];
			};

			Context(Integer width, Integer height)
				: m_width(width)
				, m_height(height)
				, m_frontId(0)
				, m_backId(1)
				, m_depthBuffer(width, height, 4, 4)
			{
				m_swapBuffer[ 0 ] = Buffer(width, height, 3);
				m_swapBuffer[ 1 ] = Buffer(width, height, 3);
			}

			// Operations

			void			SwapBuffer()
			{
				std::swap(m_frontId, m_backId);
			}

			// Properties

			Integer			GetWidth()
			{
				return m_width;
			}
			Integer			GetHeight()
			{
				return m_height;
			}
			Buffer &		GetFrontBuffer()
			{
				return m_swapBuffer[ m_frontId ];
			}
			Buffer &		GetBackBuffer()
			{
				return m_swapBuffer[ m_backId ];
			}
			RenderTarget		GetRenderTarget()
			{
				return RenderTarget(m_width, m_height, GetBackBuffer().Data());
			}
			Buffer &		GetDepthBuffer()
			{
				return m_depthBuffer;
			}
			Constants &		GetConstants()
			{
				return m_constants;
			}

		private:
			Integer			m_width;
			Integer			m_height;
			Buffer			m_swapBuffer[ 2 ];
			int			m_frontId;
			int			m_backId;

			Buffer			m_depthBuffer;

			std::vector<Buffer>	m_texBuffers;

			Constants		m_constants;
		};

		enum class VertexFormat
		{
			POSITION_RGB = 0,
			POSITION_TEXCOORD = 1,
		};
		struct Vertex
		{
			Vec3 pos;
			union
			{
				RGB color;
				Vec2 uv;
			};
		};

		class Input
		{
		public:
			enum class Topology
			{
				TRIANGLE_LIST,
			};

			std::vector<Vertex>	m_vertices[2];
		};

		Vertex MakeVertex(Vec3 pos, RGB color);
		Vertex MakeVertex(Vec3 pos, Vec2 uv);

		namespace Shader
		{
			struct Pixel
			{
				Vec3 pos; // x: [0, screen width) y: [0, screen height) z: depth: [0.0f, 1.0f]
				Vec3 color;
				Vec2 tex;
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
					return { in.v };
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
					return { in.p.color };
				}
			};
		}
	}


	void Rasterize(Pipeline::Context & context, Pipeline::Vertex * pVertexBuffer, Integer nVertex, Pipeline::VertexFormat vertexFormat);

	namespace DB
	{
		using Vertex = Pipeline::Vertex;

		struct Triangles
		{
			static std::vector<std::vector<Vertex>> One();
			static std::vector<std::vector<Vertex>> Two();
			static std::vector<std::vector<Vertex>> TwoIntersect();
			static std::vector<std::vector<Vertex>> TextureTest();
			static std::vector<std::vector<Vertex>> Perspective();
		};

		struct Textures
		{
			static const Graphics::Texture2D & Duang();
		};

		const Camera & DefaultCamera();
	}
}
