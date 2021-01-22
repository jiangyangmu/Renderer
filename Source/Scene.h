#pragma once

#include "Buffer.h"
#include "Renderer.h"
#include "RenderWindow.h"

namespace Graphics
{
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

	struct SceneObject;

	// struct Transform {};
	using Transform = Matrix4x4;

	enum class ConnectType
	{
		DEFAULT,		// set posXYZ, oriXYZ / pass source
		PLAYER,			// set posXYZ, oriXZ / pass source
		FIRST_PERSON_VIEW,	// set posXYZ, oriXYZ / pass null
		THIRD_PERSON_VIEW,	// set (posXYZ - f * oriXYZ), oriXYZ / pass null
		MINI_MAP_VIEW,		// set posXZ, oriXZ / pass null
	};
	struct Connection
	{
		ConnectType		type;
		SceneObject *		pTargetObject;
	};

	struct SceneObject : protected TreeNode
	{
		Transform		transform;

		SceneObject *		pConnectMaster;
		std::vector<Connection> vConnectSlaves;

		SceneObject()
			: transform(Matrix4x4::Identity())
			, pConnectMaster(nullptr)
		{
		}

		void			AddChild(SceneObject * pChild)
		{
			TreeNode::AddChild(pChild);
		}
		void			ConnectTo(SceneObject * pSlave, ConnectType type)
		{
			ASSERT(pSlave->pConnectMaster == nullptr);
			pSlave->pConnectMaster = this;
			vConnectSlaves.emplace_back(Connection{ type, pSlave });
		}

		virtual void		Initialize(RenderContext & context, VertexBuffer & vertexBuffer) {}
		virtual void		Update(double ms) {}

		static void		InitializeAll(SceneObject * pRootObject, RenderContext & context, VertexBuffer & vertexBuffer);
		static void		UpdateAll(SceneObject * pRootObject, double ms);
		static void		ApplyChangeToConnectionTree(SceneObject * pRootObject, Transform * pSourceTransform);
	};

	struct Entity : SceneObject
	{
		virtual void		Draw() {}

		static void		DrawAll(Entity * pEntity);
	};

	struct Light : Entity
	{
	};

	struct Player : Entity
	{
		Ptr<Renderable>		renderable;

		virtual void		Initialize(RenderContext & context, VertexBuffer & vertexBuffer) override
		{
			if (ROCube::IsVertexFormatCompatible(vertexBuffer.GetVertexFormat()))
			{
				renderable.reset(new ROCube({ -2.0f, 0.0f, 3.0f }, 1.0f));
				renderable->Initialize(context, vertexBuffer);
			}
		}
		virtual void		Update(double ms) override
		{
			renderable->Update(ms);
		}
		virtual void		Draw() override
		{
			renderable->Draw();
		}
	};

	struct Animal : Entity
	{
		Ptr<Renderable>		renderable;

		virtual void		Initialize(RenderContext & context, VertexBuffer & vertexBuffer) override
		{
			if ( ROBlinnPhongCube::IsVertexFormatCompatible(vertexBuffer.GetVertexFormat()) )
			{
				renderable.reset(new ROBlinnPhongCube({ 1.0f, 0.0f, 3.0f }, 1.0f));
				renderable->Initialize(context, vertexBuffer);
			}
		}
		virtual void		Update(double ms) override
		{
			renderable->Update(ms);
		}
		virtual void		Draw() override
		{
			renderable->Draw();
		}
	};

	struct Terrain : Entity
	{
		Ptr<Renderable>		renderable;

		virtual void		Initialize(RenderContext & context, VertexBuffer & vertexBuffer) override
		{
			if (ROTriangle::IsVertexFormatCompatible(vertexBuffer.GetVertexFormat()))
			{
				renderable.reset(new ROTriangle({ -1.0f,   0.0f, 3.0f }, { 1.0f, 0.0f, 0.0f },
								{  0.0f,   0.0f, 3.0f }, { 0.0f, 1.0f, 0.0f },
								{ -0.5f, 0.866f, 3.0f }, { 0.0f, 0.0f, 1.0f }));
				renderable->Initialize(context, vertexBuffer);
			}
		}
		virtual void		Update(double ms) override
		{
			renderable->Update(ms);
		}
		virtual void		Draw() override
		{
			renderable->Draw();
		}
	};

	struct Controller : SceneObject
	{
		// look at
		bool			init = true;
		int			pixelX = 0;
		int			pixelY = 0;
		float			hRotDeg = 0.0f;
		float			vRotDeg = 0.0f;

		// move
		const float		speed = 10.0f;
		float			forwardFactor = 0.0f;
		float			rightFactor = 0.0f;
		float			upFactor = 0.0f;
		float			vFactor = 0.0f;
		float			hFactor = 0.0f;
		Vec3			pos = {0.0f, 0.0f, 0.0f};

		virtual void		Initialize(RenderContext & context, VertexBuffer & vertexBuffer) override;
		virtual void		Update(double ms) override;

		_RECV_EVENT_DECL1(Controller, OnMouseMove);
		_RECV_EVENT_DECL1(Controller, OnKeyDown);
		_RECV_EVENT_DECL1(Controller, OnKeyUp);
	};

	struct Camera : SceneObject
	{
	public:
		Camera()
			: m_context(nullptr)
			, m_observedEntity(nullptr)
			, m_aspectRatio(1.6f)
		{
		}

		virtual void		Initialize(RenderContext & context, VertexBuffer & vertexBuffer) override
		{
			m_context		= &context;
		}

		void			ObserveEntity(Entity * pEntity)
		{
			m_observedEntity	= pEntity;
		}
		void			DrawObservedEntity();

		void			SetAspectRatio(float value)
		{
			m_aspectRatio		= value;
		}
		const Matrix4x4 &	GetViewTransform()
		{
			return transform;
		}
		Matrix4x4		GetProjTransform()
		{
			return Matrix4x4::PerspectiveFovLH(DegreeToRadian(90),
							   m_aspectRatio,
							   0.1f,
							   1000.0f);
		}

	private:
		RenderContext *		m_context;
		Entity *		m_observedEntity;
		float			m_aspectRatio;
	};

	struct EntityGroup : Entity
	{
		void			AddChild(Entity * pEntity)
		{
			Entity::AddChild(pEntity);
		}
	};

	struct Root : SceneObject
	{
	};


	class SceneManager
	{
	public:
		static SceneManager &	Default();

		Root *			CreateRoot();
		EntityGroup *		CreateEntityGroup();
		Light *			CreateLight();
		Player *		CreatePlayer();
		Animal *		CreateAnimal();
		Terrain *		CreateTerrain();
		Camera *		CreateCamera();
		Controller *		CreateController();

	private:
		Device *		m_device;
	};


	class IScene
	{
	public:
		virtual			~IScene() = default;
		virtual void		OnLoad(Device & device, RenderContext & context) = 0;
		virtual void		OnUnload() = 0;
		virtual void		OnUpdate(double ms) = 0;
		virtual void		OnDraw() = 0;
	};

	class SceneRenderer : public IRenderer
	{
	public:
		SceneRenderer(RenderWindow & window);

		void			SwitchScene(IScene & scene);

		virtual void		Present() override;
		virtual void		Clear() override;
		virtual void		Update(double ms) override;
		virtual void		Draw() override;

		// TODO: handle window resize

	private:
		RenderWindow &		m_window;

		Device			m_device;
		SwapChain		m_swapChain;
		RenderContext		m_context;
		DepthStencilBuffer	m_depthStencilBuffer;

		IScene *		m_scene;
	};
}