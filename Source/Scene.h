#pragma once

#include "Graphics.h"
#include "Renderer.h"

namespace Graphics
{
	struct SceneState
	{
		Ptr<Camera1>		camera;
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

	class SceneRenderable : public Renderable1
	{
	public:
		// Operations

		virtual void		Initialize(RenderContext1 & renderContext, RenderInput & renderInput) override;
		virtual void		Update(double milliSeconds) override;

		// Properties

		SceneState &		GetSceneState()
		{
			return *m_refSceneState;
		}

	private:
		Ptr<SceneState>		m_refSceneState;
	};




	struct TreeNode
	{
		TreeNode *		firstChild;
		TreeNode *		lastChild;
		TreeNode *		rightSibling;
		TreeNode *		parent;

		TreeNode()
			: firstChild(nullptr)
			, lastChild(nullptr)
			, rightSibling(nullptr)
			, parent(nullptr)
		{}

		void			AddChild(TreeNode * child)
		{
			if (!firstChild)
			{
				firstChild = lastChild = child;
			}
			else
			{
				child->parent = this;
				lastChild->rightSibling = child;
				lastChild = child;
			}
		}
		
		static TreeNode *	Parent(TreeNode * pNode)
		{
			return pNode->parent;
		}
		static TreeNode *	FirstChild(TreeNode * pNode)
		{
			return pNode->firstChild;
		}
		static TreeNode *	NextChild(TreeNode * pNode)
		{
			return pNode->rightSibling;
		}
	};
	// struct Transform {};
	using Transform = Matrix4x4;

	enum class Connector
	{
		DEFAULT,
		THIRD_PERSON_VIEW,
	};

	struct SceneObject : TreeNode
	{
		Transform		transform;

		SceneObject() : transform(Matrix4x4::Identity())
		{
		}

		void			AddChild(SceneObject * pChild, Connector c = Connector::DEFAULT)
		{
			TreeNode::AddChild(pChild);
		}

		virtual void		Initialize(RenderContext & context, VertexBuffer & vertexBuffer) {}
		virtual void		Update(double ms) {}

		static void		InitializeAll(SceneObject * pRootObject, RenderContext & context, VertexBuffer & vertexBuffer)
		{
			ASSERT(pRootObject);
			pRootObject->Initialize(context, vertexBuffer);
			for (TreeNode * pNode = FirstChild(pRootObject); pNode; pNode = NextChild(pNode))
			{
				InitializeAll(static_cast<SceneObject *>(pNode), context, vertexBuffer);
			}
		}
		static void		UpdateAll(SceneObject * pRootObject, double ms)
		{
			ASSERT(pRootObject);
			pRootObject->Update(ms);
			for ( TreeNode * pNode = FirstChild(pRootObject); pNode; pNode = NextChild(pNode) )
			{
				UpdateAll(static_cast< SceneObject * >( pNode ), ms);
			}
		}
	};


	struct Light : SceneObject
	{
	};

	struct Entity : SceneObject {};

	struct Player : Entity
	{
		Ptr<Renderable>		renderable;

		virtual void		Initialize(RenderContext & context, VertexBuffer & vertexBuffer) override
		{
			renderable.reset(new ROTriangle({ 0.0f,   0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f },
							{ 1.0f,   0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f },
							{ 0.5f, 0.866f, 1.0f }, { 0.0f, 0.0f, 1.0f }));
			renderable->Initialize(context, vertexBuffer);
		}
		virtual void		Update(double ms) override
		{
			renderable->Update(ms);
		}
	};

	struct Terrain : Entity
	{
		Ptr<Renderable>		renderable;

		virtual void		Initialize(RenderContext & context, VertexBuffer & vertexBuffer) override
		{
			renderable.reset(new ROTriangle({ -1.0f,   0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f },
							{  0.0f,   0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f },
							{ -0.5f, 0.866f, 1.0f }, { 0.0f, 0.0f, 1.0f }));
			renderable->Initialize(context, vertexBuffer);
		}
		virtual void		Update(double ms) override
		{
			renderable->Update(ms);
		}
	};

	struct Controller : SceneObject
	{
		virtual void		Initialize(RenderContext & context, VertexBuffer & vertexBuffer) override
		{
		}
	};

	struct Camera : SceneObject
	{
	public:
		virtual void		Initialize(RenderContext & context, VertexBuffer & vertexBuffer) override
		{
			double W = context.GetOutputTarget().GetWidth();
			double H = context.GetOutputTarget().GetHeight();

			m_context		= &context;
			m_viewTransform		= Matrix4x4::Identity();
			m_projTransform		= Matrix4x4::PerspectiveFovLH(DegreeToRadian(90),
									      W / H,
									      0.1f,
									      1000.0f);
			m_context->SetViewTransform(m_viewTransform);
			m_context->SetProjectionTransform(m_projTransform);
		}
		virtual void		Update(double ms) override
		{
		}
	private:
		RenderContext *		m_context;
		Matrix4x4		m_viewTransform;
		Matrix4x4		m_projTransform;
	};

	struct Scene : SceneObject
	{
	};


	class SceneManager
	{
	public:
		static SceneManager &	Default();

		Scene *			CreateScene();
		Light *			CreateLight();
		Player *		CreatePlayer();
		Terrain *		CreateTerrain();
		Camera *		CreateCamera();
		Controller *		CreateController();

	private:
		Device *		m_device;
	};
}