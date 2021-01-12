#pragma once

#include "Camera.h"
#include "Common.h"
#include "Event.h"

#include <functional>
#include <memory>
#include <vector>

namespace Graphics
{
	class RenderWindow;

	class Buffer
	{
	public:
		Buffer();
		Buffer(Integer width, Integer height, Integer elementSize, Integer alignment = 1, Integer rowPadding = 0);
		~Buffer();

		Buffer(const Buffer &) = delete;
		Buffer(Buffer && other);
		Buffer & operator = (const Buffer &) = delete;
		Buffer & operator = (Buffer && other);

		// Operations

		void		SetAll(Byte value);
		template <typename T>
		void		SetAllAs(T value)
		{
			ASSERT(sizeof(T) == m_elementSize);
			Byte * pRow = m_data;
			T * pData;
			for (Integer r = 0; r < m_height; ++r)
			{
				pData = reinterpret_cast<T *>(pRow);
				for (Integer c = 0; c < m_width; ++c)
				{
					*pData++ = value;
				}
				pRow += m_rowSizeInBytes;
			}
		}
		// TODO: resize (keep properties)

		// Properties

		Integer		Width() const;
		Integer		Height() const;
		Integer		Alignment() const;
		Integer		SizeInBytes() const;
		Integer		RowSizeInBytes() const;
		Integer		ElementCount() const;
		Integer		ElementSize() const;
		const void *	Data() const;
		void *		Data();
		const void *	At(Integer row, Integer col) const
		{
			ASSERT(m_data);
			return m_data + row * m_rowSizeInBytes + col * m_elementSize;
		}
		void *		At(Integer row, Integer col)
		{
			ASSERT(m_data);
			return m_data + row * m_rowSizeInBytes + col * m_elementSize;
		}


	private:
		Integer		m_width;
		Integer		m_height;
		Integer		m_alignment;
		Integer		m_elementSize;
		Integer		m_sizeInBytes;
		Integer		m_rowSizeInBytes;
		Byte *		m_data;
	};

	class Texture2D1
	{
	public:
		// Constructors
		
		Texture2D1();
		Texture2D1(const Buffer & bitmap);

		Texture2D1(const Texture2D1 &) = default;
		Texture2D1(Texture2D1 &&) = default;
		Texture2D1 & operator = (const Texture2D1 &) = default;
		Texture2D1 & operator = (Texture2D1 &&) = default;

		// Operations

		inline void Sample(float u, float v, float * rgb) const
		{
			ASSERT(0.0f <= u && u <= 1.0001f);
			ASSERT(0.0f <= v && v <= 1.0001f);
			ASSERT(u + v <= 2.0f);

			LONG width = m_bitmap->Width();
			LONG height = m_bitmap->Height();

			LONG col = static_cast< LONG >( width * u );
			LONG row = static_cast< LONG >( height * v );

			const Byte * bgra = ( Byte * ) m_bitmap->At(row, col);
			rgb[ 0 ] = static_cast< float >( bgra[ 2 ] ) / 255.f;
			rgb[ 1 ] = static_cast< float >( bgra[ 1 ] ) / 255.f;
			rgb[ 2 ] = static_cast< float >( bgra[ 0 ] ) / 255.f;
		}

	private:
		const Buffer *	m_bitmap;
	};

	// TODO: remove pixel format assumption
	class RenderTarget1
	{
	public:
		// Constructors

		static Ptr<RenderTarget1>	FromRenderWindow(RenderWindow & renderWindow);
		
		RenderTarget1(const RenderTarget1 &) = delete;
		RenderTarget1(RenderTarget1 && other) = default;
		RenderTarget1 &			operator = (const RenderTarget1 &) = delete;
		RenderTarget1 &			operator = (RenderTarget1 && other) = default;

		// Operations

		void				CopyPixelData(Byte * pBytes, Integer nBytes);

		// Properties

		Integer				Width()
		{
			return m_width;
		}
		Integer				Height()
		{
			return m_height;
		}

		// Events

	public: _RECV_EVENT_DECL1(RenderTarget1, OnWndResize);

	private:
		RenderTarget1() : m_refRenderWindow(nullptr) {}

		Integer				m_width;
		Integer				m_height;
		RenderWindow *			m_refRenderWindow;
	};

	namespace Materials
	{
		struct Ambient
		{
			RGB color;
		};
		struct Diffuse
		{
			RGB color;
		};
		struct Specular
		{
			RGB color;
		};
		struct BlinnPhong
		{
			Ambient ambient;
			Diffuse diffuse;
			Specular specular;
			float ambientRatio;
			float diffuseRatio;
			float specularRatio;
		};
	}
	namespace Lights
	{
		struct Light
		{
			RGB color;
			Vec3 pos;
			Vec3 attenuation;
		};
		struct Sphere
		{
			Light light;
			float radius;
		};
		struct Plane
		{
			Light light;
		};
	}

	namespace Pipeline
	{
		typedef int64_t TextureId;

		enum class VertexFormat
		{
			POSITION_RGB,
			POSITION_TEXCOORD,
			POSITION_NORM_MATERIAL,
		};

		struct Vertex
		{
			Vec3 pos;
			union
			{
				Vec3 color;
				Vec2 uv;
				struct
				{
					Vec3 norm;
					Vec3 material;
				};
			};
		};

		Vertex MakeVertex(Vec3 pos, Vec3 color);
		Vertex MakeVertex(Vec3 pos, Vec2 uv);
		Vertex MakeVertex(Vec3 pos, Vec3 norm, Vec3 material);

		struct ContextConstants
		{
			Matrix4x4		WorldToCamera;
			Matrix4x4		CameraToNDC;

			int			DebugPixel[ 2 ];

			Texture2D1		Texture;

			Vec3			CameraPosition; // world coordinates
			Lights::Light		Light;
			Materials::BlinnPhong	Material;
		};

		namespace Shader
		{
			class VertexShader
			{
			public:
				using Input = Vertex;
				struct Output
				{
					Vec3 posNDC; // x: [0, screen width) y: [0, screen height) z: depth: [0.0f, 1.0f]
					Vec3 color;
					Vec3 norm, material, posWld;
					Vec2 uv;
				};
				using Func = Output(*)( const Input & in, const ContextConstants & constants );

				static Output VS_RGB(const Input & in, const ContextConstants & constants);
				static Output VS_TEX(const Input & in, const ContextConstants & constants);
				static Output VS_BlinnPhong(const Input & in, const ContextConstants & constants);
			};

			class PixelShader
			{
			public:
				struct Input
				{
					Vec3 posNDC; // x: [0, screen width) y: [0, screen height) z: depth: [0.0f, 1.0f]
					Vec3 color;
					Vec3 norm, material, posWld;
					Vec2 uv;
				};
				struct Output
				{
					Vec3 color;
				};
				using Func = Output(*)( const Input & in, const ContextConstants & constants );

				static Output PS_RGB(const Input & in, const ContextConstants & constants);
				static Output PS_TEX(const Input & in, const ContextConstants & constants);
				static Output PS_BlinnPhong(const Input & in, const ContextConstants & constants);
			};
		}
	}

	class RenderInput
	{
	public:
		using Vertex = Pipeline::Vertex;

		enum class Topology
		{
			TRIANGLE_LIST,
		};

		std::vector<std::vector<Vertex>> m_vertices;
	};

	class RenderContext1
	{
	public:
		using VS = Pipeline::Shader::VertexShader::Func;
		using PS = Pipeline::Shader::PixelShader::Func;
		using ContextConstants = Pipeline::ContextConstants;

		RenderContext1(Integer width, Integer height);

		// Operations

		void			LoadTexture(LPCWSTR lpFilePath);
		void			Resize(Integer width, Integer height);
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
		Buffer &		GetDepthBuffer()
		{
			return m_depthBuffer;
		}
		Buffer &		GetStencilBuffer()
		{
			return m_stencilBuffer;
		}
		Texture2D1		GetTexture(Integer id)
		{
			return Texture2D1(m_textureBuffers[ id ]);
		}

		ContextConstants &	GetConstants()
		{
			return m_constants;
		}

		void			SetVertexShader(VS vertexShader)
		{
			m_vertexShaderFunc = vertexShader;
		}
		void			SetPixelShader(PS pixelShader)
		{
			m_pixelShaderFunc = pixelShader;
		}
		VS			GetVertexShader() const
		{
			return m_vertexShaderFunc;
		}
		PS			GetPixelShader() const
		{
			return m_pixelShaderFunc;
		}

		// Events

	public: _RECV_EVENT_DECL1(RenderContext1, OnWndResize);

	private:
		// Dimension

		Integer			m_width;
		Integer			m_height;

		// Buffers

		Buffer			m_swapBuffer[ 2 ];
		int			m_frontId;
		int			m_backId;

		Buffer			m_depthBuffer;
		Buffer			m_stencilBuffer;

		std::vector<Buffer>	m_textureBuffers;

		// Constant data

		ContextConstants	m_constants;

		// Shaders

		VS			m_vertexShaderFunc;
		PS			m_pixelShaderFunc;
	};

	inline void ResetBackBuffer(Buffer & backBuffer)
	{
		backBuffer.SetAll(0);
	}
	inline void ResetDepthBuffer(Buffer & depthBuffer)
	{
		depthBuffer.SetAllAs<float>(1.0f);
	}
	inline void ResetStencilBuffer(Buffer & stencilBuffer)
	{
		Integer xMid = stencilBuffer.Width() / 2;
		Integer yMid = stencilBuffer.Height() / 2;
		for ( Integer y = 0; y < stencilBuffer.Height(); ++y )
		{
			for ( Integer x = 0; x < stencilBuffer.Width(); ++x )
			{
				*static_cast< Byte * >( stencilBuffer.At(y, x) ) = ( ( x - xMid ) * ( x - xMid ) + ( y - yMid ) * ( y - yMid ) ) < 100 * 100 ? 1 : 0;
			}
		}
	}

	void Rasterize(RenderContext1 & context, const Pipeline::Vertex * pVertexBuffer, Integer nVertex, Pipeline::VertexFormat vertexFormat);
}
