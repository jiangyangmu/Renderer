#include "Scene.h"

using Graphics::Pipeline::Vertex;
using namespace Graphics;

static const std::vector<std::vector<Vertex>> & One()
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

static const std::vector<std::vector<Vertex>> & Two()
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

static const std::vector<std::vector<Vertex>> & IntersectionTest()
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

static const std::vector<std::vector<Vertex>> & TextureTest()
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

static const std::vector<std::vector<Vertex>> & PerspectiveProjectionTest()
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

static const std::vector<std::vector<Vertex>> & CameraTest()
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
				Vec3 color = ( ( x + z ) % 2 == 0 ) ? Vec3 { 0.0f, 0.8f, 0.0f } : Vec3 { 1.0f, 1.0f, 1.0f };

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

static const std::vector<std::vector<Vertex>> & LightingTest()
{
	static std::vector<std::vector<Vertex>> vertices =
	{
		// RGB
		{},
		// Texture
		{},
		// Normal, material
		{
			Pipeline::MakeVertex({-0.5f, -0.5f, 1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f}),
			Pipeline::MakeVertex({ 0.5f, -0.5f, 1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f}),
			Pipeline::MakeVertex({ 0.5f,  0.5f, 1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f}),
			Pipeline::MakeVertex({-0.5f, -0.5f, 1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f}),
			Pipeline::MakeVertex({ 0.5f,  0.5f, 1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f}),
			Pipeline::MakeVertex({-0.5f,  0.5f, 1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 1.0f}),
		},
	};
	return vertices;
}

namespace Graphics
{
	SceneState SceneLoader::Default(Integer width, Integer height)
	{
		SceneState defaultScene =
		{
			Ptr<Camera>(new Camera(
				0.1f,				// z near
				1000.0f,			// z far (use smaller value for better depth test.)
				DegreeToRadian(90.0f),		// field of view
				( ( double ) width ) / height,	// aspect ratio
				Vec3::Zero()			// position
			)),
			Lights::Light
			{
				{ 1.0f, 1.0f, 1.0f }, // color
				{ 0.3f, 0.3f, 0.1f }, // position
				{ 0.0f, 0.0f, 1.0f }, // attenuation
			},
			Materials::BlinnPhong
			{
				Materials::Ambient{{1.0f, 1.0f, 1.0f}},
				Materials::Diffuse{{1.0f, 1.0f, 1.0f}},
				Materials::Specular{{1.0f, 1.0f, 1.0f}},
				0.0f,
				0.8f,
				0.2f,
			},
			TEXT("Resources/duang.bmp"),
			One()
		};

		return defaultScene;
	}

}