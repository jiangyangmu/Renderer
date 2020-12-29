#pragma once

#include "Camera.h"
#include "Common.h"

#include <functional>
#include <memory>
#include <vector>

namespace Graphics
{
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

		// Properties

		Integer		Width() const;
		Integer		Height() const;
		Integer		SizeInBytes() const;
		Integer		RowSizeInBytes() const;
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
		Integer		m_rowSizeInBytes;
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

		inline void Sample(float u, float v, float * rgb) const
		{
			ASSERT(0.0f <= u && u <= 1.0001f);
			ASSERT(0.0f <= v && v <= 1.0001f);
			ASSERT(u + v <= 2.0f);

			LONG width = m_bitmap.Width();
			LONG height = m_bitmap.Height();

			LONG col = static_cast< LONG >( width * u );
			LONG row = static_cast< LONG >( height * v );

			const Byte * bgra = ( Byte * ) m_bitmap.At(row, col);
			rgb[ 0 ] = static_cast< float >( bgra[ 2 ] ) / 255.f;
			rgb[ 1 ] = static_cast< float >( bgra[ 1 ] ) / 255.f;
			rgb[ 2 ] = static_cast< float >( bgra[ 0 ] ) / 255.f;
		}

	private:
		const Buffer &	m_bitmap;
	};

	class RenderTarget
	{
	public:
		RenderTarget(Integer width, Integer height, Integer rowSizeInBytes, void * backBuffer);

		Integer		Width();
		Integer		Height();
		void		SetPixel(Integer x, Integer y, Byte r, Byte g, Byte b);

	private:
		Integer		m_width;
		Integer		m_height;
		Integer		m_rowSizeInBytes;
		void *		m_backBuffer;
	};

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

		struct DiffuseLight
		{
			Vec3 posWld;
			RGB color;
		};

		struct SpecularLight
		{
			Vec3 posWld;
			RGB color;
		};

		class Context
		{
		public:
			struct Constants
			{
				Matrix4x4	WorldToCamera;
				Matrix4x4	CameraToNDC;

				int		DebugPixel[2];
				Texture2D	Texture;
				DiffuseLight	Diffuse;
				SpecularLight	Specular;
			};

			Context(Integer width, Integer height);

			// Operations

			void			Resize(Integer width, Integer height)
			{
				m_width = width;
				m_height = height;

				int rowPadding = (4 - ((width * 3) & 0x3)) & 0x3;
				m_depthBuffer = Buffer(width, height, 4, 4);
				m_swapBuffer[ 0 ] = Buffer(width, height, 3, 4, rowPadding);
				m_swapBuffer[ 1 ] = Buffer(width, height, 3, 4, rowPadding);
			}
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
				return RenderTarget(m_width, m_height, GetBackBuffer().RowSizeInBytes(), GetBackBuffer().Data());
			}
			Buffer &		GetDepthBuffer()
			{
				return m_depthBuffer;
			}
			Constants &		GetConstants()
			{
				return m_constants;
			}

			void			SetVertexShader(void * func)
			{
				m_vertexShaderFunc = func;
			}
			void			SetPixelShader(void * func)
			{
				m_pixelShaderFunc = func;
			}
			void *			GetVertexShader() const
			{
				return m_vertexShaderFunc;
			}
			void *			GetPixelShader() const
			{
				return m_pixelShaderFunc;
			}

		private:
			Integer			m_width;
			Integer			m_height;

			Buffer			m_swapBuffer[ 2 ];
			int			m_frontId;
			int			m_backId;

			Buffer			m_depthBuffer;

			Constants		m_constants;

			void *			m_vertexShaderFunc;
			void *			m_pixelShaderFunc;
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
				using Func = Output(*)( Input in, const Context::Constants & constants );

				static inline Output VS_RGB(Input in, const Context::Constants & constants)
				{
					Output out;
					out.posNDC = Vec3::Transform(Vec3::Transform(in.pos, constants.WorldToCamera), constants.CameraToNDC);
					out.color = in.color;
					return out;
				}
				static inline Output VS_TEX(Input in, const Context::Constants & constants)
				{
					Output out;
					out.posNDC = Vec3::Transform(Vec3::Transform(in.pos, constants.WorldToCamera), constants.CameraToNDC);
					out.uv = in.uv;
					return out;
				}
				static inline Output VS_LIGHT(Input in, const Context::Constants & constants)
				{
					Output out;
					out.posNDC = Vec3::Transform(Vec3::Transform(in.pos, constants.WorldToCamera), constants.CameraToNDC);
					out.norm = in.norm;
					out.material = in.material;
					out.posWld = in.pos;
					return out;
				}
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
				using Func = Output (*)(Input in, const Context::Constants & constants);

				static inline Output PS_RGB(Input in, const Context::Constants & constants)
				{
					return { in.color };
				}
				static inline Output PS_TEX(Input in, const Context::Constants & constants)
				{
					float color[3];
					constants.Texture.Sample(in.uv.x, in.uv.y, color);
					return { color[0], color[1], color[2] };
				}
				static inline Output PS_LIGHT(Input in, const Context::Constants & constants)
				{
					constexpr float diffuseRatio = 0.5f;
					constexpr float specularRatio = 0.5f;

					// Diffuse Color = max( cos(-L, norm), 0) * ElementwiseProduce(C_light, C_material)
					Vec3 diffuse;
					{
						const DiffuseLight & diffuseLight = constants.Diffuse;

						Vec3 invLightDir = Vec3::Normalize(diffuseLight.posWld - in.posWld);

						float decayFactor = Max(0.0f, Vec3::Dot(invLightDir, in.norm));

						diffuse =
							Vec3::Scale(
								Vec3::ElementwiseProduct(RGBToVec3(diffuseLight.color), in.material),
								decayFactor);
					}

					// Specular Color = max( cos(L', to-eye), 0) * ElementwiseProduce(C_light, C_material)
					Vec3 specular;
					{
						const SpecularLight & specularLight = constants.Specular;

						Vec3 lightDir = Vec3::Normalize(in.posWld - specularLight.posWld);
						Vec3 reflectLightDir = Vec3::Normalize(lightDir - Vec3::Scale(in.norm, 2 * Vec3::Dot(in.norm, lightDir)));
						Vec3 toEyeDir = Vec3::Normalize(-in.posWld);

						float decayFactor = Max(0.0f, Vec3::Dot(reflectLightDir, toEyeDir));
						decayFactor = decayFactor * decayFactor;
						decayFactor = decayFactor * decayFactor;
						decayFactor = decayFactor * decayFactor;

						specular =
							Vec3::Scale(
								Vec3::ElementwiseProduct(RGBToVec3(specularLight.color), in.material),
								decayFactor);
					}

					return { WeightedAdd(diffuse, specular, Vec3::Zero(), diffuseRatio, specularRatio, 0.0f) };
				}
			};
		}

		class Input
		{
		public:
			enum class Topology
			{
				TRIANGLE_LIST,
			};

			std::vector<Vertex>	m_vertices[2];
		};
	}


	void Rasterize(Pipeline::Context & context, const Pipeline::Vertex * pVertexBuffer, Integer nVertex, Pipeline::VertexFormat vertexFormat);

	namespace DB
	{
		using Vertex = Pipeline::Vertex;
		using DiffuseLight = Pipeline::DiffuseLight;
		using SpecularLight = Pipeline::SpecularLight;

		struct Triangles
		{
			static const std::vector<std::vector<Vertex>> & One();
			static const std::vector<std::vector<Vertex>> & Two();
			static const std::vector<std::vector<Vertex>> & IntersectionTest();
			static const std::vector<std::vector<Vertex>> & TextureTest();
			static const std::vector<std::vector<Vertex>> & PerspectiveProjectionTest();
			static const std::vector<std::vector<Vertex>> & CameraTest();
			static const std::vector<std::vector<Vertex>> & LightingTest();
		};

		struct Textures
		{
			static const Graphics::Texture2D & Duang();
		};

		struct Lights
		{
			static const DiffuseLight & Diffuse();
			static const SpecularLight & Specular();
		};
	}
}
