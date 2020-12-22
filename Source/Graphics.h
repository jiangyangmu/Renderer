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

	struct Triangle
	{
		Vec3 a, b, c;
		RGB rgbA, rgbB, rgbC;
		Vec2 uvA, uvB, uvC;
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
		Buffer(unsigned int width, unsigned int height, unsigned int elementSize, unsigned int alignment = 1);
		~Buffer();

		Buffer(const Buffer &) = delete;
		Buffer(Buffer && other);
		Buffer & operator = (const Buffer &) = delete;
		Buffer & operator = (Buffer && other);

		// Operations

		void		SetAll(Byte value);
		void		Reshape(unsigned int width, unsigned int height);

		// Properties

		unsigned int	Width() const;
		unsigned int	Height() const;
		unsigned int	SizeInBytes() const;
		unsigned int	ElementCount() const;
		unsigned int	ElementSize() const;
		const void *	Data() const;
		void *		Data();
		const void *	At(unsigned int row, unsigned int col) const;
		void *		At(unsigned int row, unsigned int col);

	private:
		unsigned int	m_width;
		unsigned int	m_height;
		unsigned int	m_elementSize;
		unsigned int	m_sizeInBytes;
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

	class Transform
	{
	public:
		Transform(const Camera & camera);

		Vec3 WorldToCamera(const Vec3 & v) const;
		Vec3 CameraToNDC(const Vec3 & v) const;

	private:
		Matrix4x4	m_worldToCamera;
		Matrix4x4	m_cameraToNDC;
	};

	class TransformTriangle
	{
	public:
		TransformTriangle(const Transform & transform, const Triangle & triangle);

		const Triangle &	GetWorldSpace();
		const Triangle &	GetCameraSpace();
		const Triangle &	GetNDCSpace();

	private:
		Triangle	m_triangle[ 3 ]; // origin(world), camera, NDC
	};

	class RenderTarget
	{
	public:
		RenderTarget(unsigned int width, unsigned int height, void * backBuffer);

		unsigned int	Width();
		unsigned int	Height();
		void		SetPixel(unsigned int x, unsigned int y, int r, int g, int b);

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

	class Rasterizer
	{
	public:

		using RasterizerProc = void(int pixelX, int pixelY, const Vec3 & pos, const RGB & color);
		using RasterizerCallback = std::function<RasterizerProc>;

		static void Rasterize(const unsigned int width,
				      const unsigned int height,
				      const Camera & camera,
				      const Triangle & triangle,
				      const Transform & transform,
				      RasterizerCallback rasterizerCB);
	};

	namespace DB
	{
		extern int GlbDebugPixel[ 2 ];

		struct Triangles
		{
			static std::vector<Triangle> One()
			{
				return
				{
					Triangle
					{
						{0.0f, 0.0f, 1.0f},
					{1.0f, 0.0f, 1.0f},
					{0.5f, 0.866f, 1.0f},

					{1.0f, 0.0f, 0.0f},
					{0.0f, 1.0f, 0.0f},
					{0.0f, 0.0f, 1.0f},
				}
				};
			}
			static std::vector<Triangle> Two()
			{
				return
				{
					Triangle
					{
						{0.0f, 0.0f, 1.0f},
					{1.0f, 0.0f, 1.0f},
					{0.5f, 0.866f, 1.0f},

					{1.0f, 0.0f, 0.0f},
					{0.0f, 1.0f, 0.0f},
					{0.0f, 0.0f, 1.0f},
				},
				Triangle
					{
						{-1.0f, 0.0f, 1.0f},
					{0.0f, 0.0f, 1.0f},
					{-0.5f, 0.866f, 1.0f},

					{1.0f, 0.0f, 0.0f},
					{0.0f, 1.0f, 0.0f},
					{0.0f, 0.0f, 1.0f},
				}
				};
			}
			static std::vector<Triangle> TwoIntersect()
			{
				return
				{
					Triangle
					{
						{-1.0f, -0.5f, 1.0f},
					{0.5f, 0.0f, 0.5f},
					{-1.0f, 0.5f, 1.0f},

					{0.0f, 0.0f, 1.0f},
					{0.0f, 1.0f, 0.0f},
					{1.0f, 0.0f, 0.0f},
				},
				Triangle
					{
						{1.0f, -0.5f, 1.0f},
					{1.0f, 0.5f, 1.0f},
					{-0.5f, 0.0f, 0.5f},

					{0.0f, 0.0f, 1.0f},
					{0.0f, 1.0f, 0.0f},
					{1.0f, 0.0f, 0.0f},
				}
				};
			}
			static std::vector<Triangle> TextureTest()
			{
				return
				{
					Triangle
					{
						{0.0f, 0.0f, 1.0f}, // Position
					{1.0f, 0.0f, 1.0f},
					{0.0f, 1.0f, 1.0f},

					{0.0f, 0.0f, 0.0f}, // Color
					{0.0f, 1.0f, 0.0f},
					{0.0f, 0.0f, 1.0f},

					{0.0f, 0.0f}, // Texture Coordinate
					{1.0f, 0.0f},
					{0.0f, 1.0f},
				},
				Triangle
					{
						{1.0f, 1.0f, 1.0f}, // Position
					{0.0f, 1.0f, 1.0f},
					{1.0f, 0.0f, 1.0f},

					{0.0f, 1.0f, 1.0f}, // Color
					{0.0f, 0.0f, 1.0f},
					{0.0f, 1.0f, 0.0f},

					{1.0f, 1.0f}, // Texture Coordinate
					{0.0f, 1.0f},
					{1.0f, 0.0f},
				},
				};
			}
		};

		struct Textures
		{
			static const Graphics::Texture2D & Duang();
		};

		const Camera & DefaultCamera();
	}
}
