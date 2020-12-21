#include "Renderer.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <functional>
#include <vector>

constexpr auto PI = 3.141592653f;
static int GlbDebugPixel[2] = {-1, -1};

struct RGB
{
	float r, g, b;
};
struct Vec3
{
	float x, y, z;

	// Constructors

	static Vec3 Zero()
	{
		return {0.0f, 0.0f, 0.0f};
	}

	// Properties

	float Length() const
	{
		return sqrtf(x * x + y * y + z * z);
	}

	// Operations

	Vec3 & Scale(float s)
	{
		x *= s;
		y *= s;
		z *= s;
		return *this;
	}
	Vec3 operator + (const Vec3 & other) const
	{
		return { x + other.x, y + other.y, z + other.z };
	}
	Vec3 operator - (const Vec3 & other) const
	{
		return { x - other.x, y - other.y, z - other.z };
	}

	static Vec3 Normalize(const Vec3 & v)
	{
		Vec3 nv = v;
		return nv.Scale(1.0 / nv.Length());
	}
	static float Dot(const Vec3 & v1, const Vec3 & v2)
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}
	static Vec3 Cross(const Vec3 & v1, const Vec3 & v2)
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
	static Matrix4x4 PerspectiveFovLH(float fov, float aspectRatio, float near, float far)
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
};

struct Triangle
{
	Vec3 a, b, c;
	RGB rgbA, rgbB, rgbC;
};

struct BarycentricCoordinate
{
	float a, b, c;
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
	return value < min ? min : (value > max ? max : value);
}
float DegreeToRadian(float d)
{
	// 0 < d < 180
	return d * PI / 180.0f;
}

// Vec & Matrix
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
	float nearPlaneYMax = camera.near * tanf(0.5f * camera.fov);
	float nearPlaneXMax = camera.aspectRatio * nearPlaneYMax;

	Vec3 ray =
	{
		nearPlaneXMax * ( pixelX * 2.0f / width - 1.0f ),
		nearPlaneYMax * ( 1.0f - pixelY * 2.0f / height ),
		camera.near
	};

	return ray;
}
bool RayTriangleIntersection(const Vec3 & ray, const Triangle & tri, float * pDistance, BarycentricCoordinate * pBarycentric)
{
	const Vec3 & p0 = tri.a;
	const Vec3 & p1 = tri.b;
	const Vec3 & p2 = tri.c;

	Vec3 o	= Vec3::Zero();
	Vec3 d	= Vec3::Normalize(ray);

	Vec3 e1 = p1 - p0;
	Vec3 e2 = p2 - p0;
	Vec3 q	= Vec3::Cross(d, e2);
	float a	= Vec3::Dot(e1, q);
	
	if (-0.00001 < a && a < 0.00001)
	{
		return false;
	}

	float f	= 1.0 / a;

	Vec3 s	= o - p0;
	float u	= f * Vec3::Dot(s, q);

	if (u < 0.0)
	{
		return false;
	}

	Vec3 r	= Vec3::Cross(s, e1);
	float v	= f * Vec3::Dot(d, r);

	if (v < 0.0 || ((double)u + v) > 1.0)
	{
		return false;
	}

	float t	= f * Vec3::Dot(e2, r);

	*pDistance	= t;
	pBarycentric->a = 1.0f - u - v;
	pBarycentric->b	= u;
	pBarycentric->c = v;

	return true;
}

namespace GraphicsPipeline
{
	class Transform
	{
	public:
		Transform(const Camera & camera)
		{
			m_worldToCamera = Matrix4x4::Identity();
			m_cameraToNDC	= Matrix4x4::PerspectiveFovLH(camera.fov,
								      camera.aspectRatio,
								      camera.near,
								      camera.far);
		}

		Vec3 WorldToCamera(const Vec3 & v) const
		{
			return Multiply(v, m_worldToCamera);
		}
		Vec3 CameraToNDC(const Vec3 & v) const
		{
			return Multiply(v, m_cameraToNDC);
		}

	private:
		Matrix4x4	m_worldToCamera;
		Matrix4x4	m_cameraToNDC;
	};

	class TransformTriangle
	{
	public:
		TransformTriangle(const Transform & transform, const Triangle & triangle)
		{
			m_triangle[0] = triangle;
			m_triangle[1] =
			{
				transform.WorldToCamera(triangle.a),
				transform.WorldToCamera(triangle.b),
				transform.WorldToCamera(triangle.c),
				triangle.rgbA,
				triangle.rgbB,
				triangle.rgbC,
			};
			m_triangle[2] =
			{
				transform.CameraToNDC(m_triangle[1].a),
				transform.CameraToNDC(m_triangle[1].b),
				transform.CameraToNDC(m_triangle[1].c),
				triangle.rgbA,
				triangle.rgbB,
				triangle.rgbC,
			};
		}

		const Triangle &	GetWorldSpace()
		{
			return m_triangle[0];
		}
		const Triangle &	GetCameraSpace()
		{
			return m_triangle[1];
		}
		const Triangle &	GetNDCSpace()
		{
			return m_triangle[2];
		}

	private:
		Triangle	m_triangle[3]; // origin(world), camera, NDC
	};


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

	class RasterizerPixelIterator
	{
	public:
		using PixelProc		= void (int pixelX, int pixelY, float distance, BarycentricCoordinate barycentric);
		using PixelCallback	= std::function<PixelProc>;

		RasterizerPixelIterator(unsigned int width,
					unsigned int height,
					const Camera & camera,
					TransformTriangle & transTriangle)
			: m_width(width)
			, m_height(height)
			, m_camera(camera)
			, m_transTriangle(transTriangle)
			, m_pixelXRange()
			, m_pixelYRange()
		{
		}

		void ForEachPixel(PixelCallback pixelCB)
		{
			// Pick pixels with AABB and ray-triangle intersection test.

			AABB aabb = GetAABB(m_transTriangle.GetNDCSpace());
			m_pixelXRange[ 0 ] = static_cast< int >( Bound(0.0f, 0.5f * ( aabb.min.x + 1.0f ), 1.0f) * m_width );
			m_pixelXRange[ 1 ] = static_cast< int >( Bound(0.0f, 0.5f * ( aabb.max.x + 1.0f ), 1.0f) * m_width );
			m_pixelYRange[ 0 ] = static_cast< int >( Bound(0.0f, 0.5f * ( 1.0f - aabb.max.y ), 1.0f) * m_height );
			m_pixelYRange[ 1 ] = static_cast< int >( Bound(0.0f, 0.5f * ( 1.0f - aabb.min.y ), 1.0f) * m_height );

			for ( int y = m_pixelYRange[ 0 ]; y <= m_pixelYRange[ 1 ]; ++y )
			{
				for ( int x = m_pixelXRange[ 0 ]; x <= m_pixelXRange[ 1 ]; ++x )
				{
					if (GlbDebugPixel[0] >= 0)
					{
						if (x != GlbDebugPixel[0] || y != GlbDebugPixel[1])
						{
							continue;
						}
						else
						{
							// Set break point here.
							GlbDebugPixel[0] = GlbDebugPixel[1] = -1;
						}
					}

					float distance;
					BarycentricCoordinate barycentric;
					if ( RayTriangleIntersection(GetPixelRay(m_camera, m_width, m_height, x, y),
								     m_transTriangle.GetCameraSpace(),
								     &distance,
								     &barycentric) )
					{
						pixelCB(x, y, distance, barycentric);
					}
				}
			}
		}

	private:
		const unsigned int	m_width;
		const unsigned int	m_height;
		const Camera &		m_camera;
		TransformTriangle &	m_transTriangle;
		int			m_pixelXRange[2];
		int			m_pixelYRange[2];
	};

	using RasterizerProc = void (int pixelX, int pixelY, const Vec3 & pos, const RGB & color);
	using RasterizerCallback = std::function<RasterizerProc>;

	void Rasterize(const unsigned int width,
		       const unsigned int height,
		       const Camera & camera,
		       const Triangle & triangle,
		       const Transform & transform,
		       RasterizerCallback rasterizerCB)
	{
		TransformTriangle transTriangle(transform, triangle);

		RasterizerPixelIterator pixelIterator(width,
						      height,
						      camera,
						      transTriangle);

		pixelIterator.ForEachPixel(
			[&] (int pixelX, int pixelY, float distance, BarycentricCoordinate barycentric)
			{
				// Properties
				RGB color;
				{
					const RGB & cA = transTriangle.GetWorldSpace().rgbA;
					const RGB & cB = transTriangle.GetWorldSpace().rgbB;
					const RGB & cC = transTriangle.GetWorldSpace().rgbC;

					color =
					{
						barycentric.a * cA.r + barycentric.b * cB.r + barycentric.c * cC.r,
						barycentric.a * cA.g + barycentric.b * cB.g + barycentric.c * cC.g,
						barycentric.a * cA.b + barycentric.b * cB.b + barycentric.c * cC.b
					};
				}

				// Position
				float depthNDC = ( distance - camera.near ) / ( camera.far - camera.near ); // FIXIT: this is not perspective correct

				Vec3 pos =
				{
					static_cast<float>(pixelX),
					static_cast<float>(pixelY),
					depthNDC
				};

				// Draw pixel
				rasterizerCB(pixelX, pixelY, pos, color);
			});
	}
}

std::unique_ptr<Renderer::RenderResult> Renderer::RenderResult::Create()
{
	std::unique_ptr<Renderer::RenderResult> output(new Renderer::RenderResult(800, 600));
	
	output->BackBuffer().SetAll(100);

	std::fill_n(reinterpret_cast<float *>(output->DepthBuffer().Data()),
		    output->DepthBuffer().ElementCount(),
		    1.0f);

	return output;
}

void Renderer::RenderResult::SetDebugPixel(int pixelX, int pixelY)
{
	GlbDebugPixel[0] = pixelX;
	GlbDebugPixel[1] = pixelY;
}

struct TriangleSet
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
};

void Renderer::RenderResult::Draw()
{
	// Output
	Renderer::Buffer & backBuffer = BackBuffer();
	Renderer::Buffer & depthBuffer = DepthBuffer();

	// Input
	Camera camera;
	camera.pos = { 0.0f, 0.0f, 0.0f };
	camera.near = 0.1f;
	camera.far = 1000.0f; // use smaller value for better depth test.
	camera.fov = DegreeToRadian(90.0f);
	camera.aspectRatio = 4.0f / 3.0f;

	auto triangleList = TriangleSet::TwoIntersect();

	// Projection & Clipping
	GraphicsPipeline::Transform transform(camera);

	// Rasterization
	{
		GraphicsPipeline::RenderTarget renderTarget(Width(),
							    Height(),
							    backBuffer.Data());

		for ( auto & triangle : triangleList )
		{
			GraphicsPipeline::Rasterize(renderTarget.Width(),
						    renderTarget.Height(),
						    camera,
						    triangle,
						    transform,
						    [&] (int pixelX, int pixelY, const Vec3 & pos, const RGB & color)
						    {
							    // Depth test
							    float * pOldDepth	= reinterpret_cast<float *>(depthBuffer.At(pixelY, pixelX));

							    float oldDepth	= *pOldDepth;
							    float newDepth	= pos.z;

							    if (oldDepth <= newDepth)
							    {
								    return;
							    }
							    *pOldDepth = newDepth;

							    //renderTarget.SetPixel(pixelX,
										 // pixelY,
										 // static_cast< unsigned char >( pos.z * 255.0f ),
										 // static_cast< unsigned char >( pos.z * 255.0f ),
										 // static_cast< unsigned char >( pos.z * 255.0f ));
							    renderTarget.SetPixel(pixelX,
										  pixelY,
										  static_cast< unsigned char >( color.r * 255.0f ),
										  static_cast< unsigned char >( color.g * 255.0f ),
										  static_cast< unsigned char >( color.b * 255.0f ));
						    });
		}
	}

	SwapBuffer();
}
