#pragma once

#include "Graphics.h"

namespace Graphics
{
	// Input event driven scene update
	// * SceneObject - structure
	// * SceneObjectController - manage input event handling
	// * SceneObjectConnector - populate transformation

	// Time event driven scene update
	struct SceneState
	{
		Ptr<Camera>		camera;
		Lights::Light		light;

		// entity
		Materials::BlinnPhong	material;
		LPCWSTR textureURL;
		std::vector<std::vector<Pipeline::Vertex>> vertices;
		
		// controllers, connectors
	};

	class SceneLoader
	{
	public:
		static SceneState	Default(Integer width, Integer height);
	};
}