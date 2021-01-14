#include "Scene.h"

namespace Graphics
{
	void			SceneObject::InitializeAll(SceneObject * pRootObject, RenderContext & context, VertexBuffer & vertexBuffer)
	{
		ASSERT(pRootObject);
		pRootObject->Initialize(context, vertexBuffer);
		for ( TreeNode * pNode = FirstChild(pRootObject); pNode; pNode = NextChild(pNode) )
		{
			InitializeAll(static_cast< SceneObject * >( pNode ), context, vertexBuffer);
		}
	}
	void			SceneObject::UpdateAll(SceneObject * pRootObject, double ms)
	{
		ASSERT(pRootObject);
		pRootObject->Update(ms);
		for ( TreeNode * pNode = FirstChild(pRootObject); pNode; pNode = NextChild(pNode) )
		{
			UpdateAll(static_cast< SceneObject * >( pNode ), ms);
		}
	}
	void			SceneObject::ApplyChangeToConnectionTree(SceneObject * pRootObject, Transform * pSourceTransform)
	{
		SceneObject * pMaster;
		SceneObject * pSlave;
		Transform * pPassTransform;
		float posY;

		ASSERT(pRootObject);

		if ( pRootObject->vConnectSlaves.empty() )
		{
			return;
		}

		ASSERT(pSourceTransform);

		pMaster = pRootObject;
		for ( Connection & connection : pRootObject->vConnectSlaves )
		{
			pSlave		= connection.pTargetObject;
			pPassTransform	= nullptr;

			switch ( connection.type )
			{
				case ConnectType::DEFAULT:
					pPassTransform = pSourceTransform;
					pSlave->transform = *pSourceTransform;
					break;
				case ConnectType::PLAYER:
					pPassTransform = pSourceTransform;
					pSlave->transform = *pSourceTransform;
					//pSlave->transform.f12 = pSlave->transform.f21 = pSlave->transform.f22 = pSlave->transform.f23 = pSlave->transform.f32 = 0.0f;
					break;
				case ConnectType::FIRST_PERSON_VIEW:
					pSlave->transform = *pSourceTransform;
					break;
				case ConnectType::THIRD_PERSON_VIEW:
					pSlave->transform = *pSourceTransform;
					break;
				case ConnectType::MINI_MAP_VIEW:
					posY = pSlave->transform.f42;
					pSlave->transform = *pSourceTransform;
					pSlave->transform.f42 = posY;
					//pSlave->transform.f12 = pSlave->transform.f21 = pSlave->transform.f22 = pSlave->transform.f23 = pSlave->transform.f32 = 0.0f;
					break;
				default:
					break;
			}

			ApplyChangeToConnectionTree(pSlave, pPassTransform);
		}
	}

	void			Entity::DrawAll(Entity * pEntity)
	{
		ASSERT(pEntity);
		pEntity->Draw();
		for ( TreeNode * pNode = FirstChild(pEntity); pNode; pNode = NextChild(pNode) )
		{
			DrawAll(static_cast< Entity * >( pNode ));
		}
	}

	void			Camera::Draw()
	{
		double W = m_context->GetOutputTarget().GetWidth();
		double H = m_context->GetOutputTarget().GetHeight();

		m_context->SetViewTransform(transform);
		m_context->SetProjectionTransform(
			Matrix4x4::PerspectiveFovLH(DegreeToRadian(90),
						    W / H,
						    0.1f,
						    1000.0f));

		Entity::DrawAll(m_observeTarget);
	}

	void			Controller::Initialize(RenderContext & context, VertexBuffer & vertexBuffer)
	{
		RenderWindow * pWindow;

		ENSURE_TRUE(
			static_cast< IUnknown && >( context.GetOutputTarget() ).QueryInterface(&pWindow));

		_BIND_EVENT(OnMouseMove, *pWindow, *this);
		_BIND_EVENT(OnKeyDown, *pWindow, *this);
		_BIND_EVENT(OnKeyUp, *pWindow, *this);
	}
	void			Controller::Update(double ms)
	{
		const Vec3 up	= { 0.0f, 1.0f, 0.0f };
		const Vec3 fwd	= { 0.0f, 0.0f, 1.0f };
		const Vec3 rt	= { 1.0f, 0.0f, 0.0f };

		float hRotRad;
		float vRotRad;
		Vec3 forwardDir;
		Vec3 rightDir;

		hRotRad		= DegreeToRadian(hRotDeg);
		vRotRad		= DegreeToRadian(Bound(-90.0f, vRotDeg, 90.0f));

		forwardDir	= Vec3::Transform(fwd, Matrix4x4::RotationAxisLH(rt, -vRotRad) * Matrix4x4::RotationAxisLH(up, hRotRad));
		forwardDir.y	= 0.0f;
		forwardDir	= Vec3::Normalize(forwardDir);

		rightDir	= Vec3::CrossLH(up, forwardDir);
		rightDir.y	= 0.0f;
		rightDir	= Vec3::Normalize(rightDir);

		float duration	= static_cast< float >( ms / 1000.0f );

		Vec3 delta =
		{
			( forwardFactor * forwardDir.x + rightFactor * rightDir.x ) * duration * speed,
			( upFactor * up.y ) * duration * speed,
			( forwardFactor * forwardDir.z + rightFactor * rightDir.z ) * duration * speed,
		};

		if ( forwardFactor != 0.0f || rightFactor != 0.0f || upFactor != 0.0f )
		{
			pos	= pos + delta;
		}
		if ( vFactor != 0.0f )
		{
			vRotDeg	+= 0.2f * ms * vFactor;
			vRotRad	= DegreeToRadian(Bound(-90.0f, vRotDeg, 90.0f));
		}

		transform	=
			Matrix4x4::Translation(-pos.x, -pos.y, -pos.z) *
			Matrix4x4::RotationAxisLH(up, -hRotRad) *
			Matrix4x4::RotationAxisLH(rt, vRotRad);

		ApplyChangeToConnectionTree(this, &transform);
	}
	_RECV_EVENT_IMPL(Controller, OnMouseMove) ( void * sender, const win32::MouseEventArgs & args )
	{
		if ( init )
		{
			init = false;
		}
		else
		{
			hRotDeg += 0.2f * ( args.pixelX - pixelX );
			vRotDeg -= 0.2f * ( args.pixelY - pixelY );
			vRotDeg = Bound(-90.0f, vRotDeg, 90.0f);
		}
		pixelX = args.pixelX;
		pixelY = args.pixelY;
	}
	_RECV_EVENT_IMPL(Controller, OnKeyDown) ( void * sender, const win32::KeyboardEventArgs & args )
	{
		switch ( args.virtualKeyCode )
		{
			case 'W': forwardFactor = 1.0f; break;
			case 'S': forwardFactor = -1.0f; break;
			case 'A': rightFactor = -1.0f; break;
			case 'D': rightFactor = 1.0f; break;
			case 'Q': upFactor = -1.0f; break;
			case 'E': upFactor = 1.0f; break;
			case 'Z': vFactor = 1.0f; break;
			case 'C': vFactor = -1.0f; break;
			default: break;
		}
	}
	_RECV_EVENT_IMPL(Controller, OnKeyUp) ( void * sender, const win32::KeyboardEventArgs & args )
	{
		switch ( args.virtualKeyCode )
		{
			case 'W':
			case 'S': forwardFactor = 0.0f; break;
			case 'A':
			case 'D': rightFactor = 0.0f; break;
			case 'Q':
			case 'E': upFactor = 0.0f; break;
			case 'Z':
			case 'C': vFactor = 0.0f; break;
			default: break;
		}
	}

	SceneManager &		SceneManager::Default()
	{
		static SceneManager sceneManager;
		return sceneManager;
	}
	Scene *			SceneManager::CreateScene()
	{
		return new Scene();
	}
	EntityGroup *		SceneManager::CreateEntityGroup()
	{
		return new EntityGroup();
	}
	Light *			SceneManager::CreateLight()
	{
		return new Light();
	}
	Player *		SceneManager::CreatePlayer()
	{
		return new Player();
	}
	Terrain *		SceneManager::CreateTerrain()
	{
		return new Terrain();
	}
	Camera *		SceneManager::CreateCamera()
	{
		return new Camera();
	}
	Controller *		SceneManager::CreateController()
	{
		return new Controller();
	}
}