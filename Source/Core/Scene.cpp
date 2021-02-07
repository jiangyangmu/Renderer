#include "Scene.h"

namespace Graphics
{
	void			Transform::GetInvertedMirroredMatrix(const Vector3 & posMirror, const Vector3 & normMirror, Matrix44 * pMirroredMatrix)
	{
		const Vector3 & posOrig = translation.xyz;
		const Vector3 & dirOrig = V3Transform(V3UnitZ(), GetRotationXYZMatrix());
		const Vector3 & upOrig = V3UnitY();
		Vector3 posMirr;
		Vector3 dirMirr;
		Vector3 upMirr;

		MirrorRayPlane(posMirror, normMirror, posOrig, dirOrig, &posMirr, &dirMirr);
		MirrorRayPlane(posMirror, normMirror, posOrig, upOrig, &posMirr, &upMirr);

		*pMirroredMatrix = M44LookToLH(posMirr, dirMirr, upMirr);
	}

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
				case ConnectType::SAME:
					pPassTransform = pSourceTransform;
					pSlave->transform = *pSourceTransform;
					break;
				case ConnectType::FIRST_PERSON_VIEW:
					pSlave->transform = *pSourceTransform;
					break;
				case ConnectType::THIRD_PERSON_VIEW:
					pSlave->transform = *pSourceTransform;
					break;
				case ConnectType::MINI_MAP_VIEW:
					pSlave->transform.tx = pSourceTransform->tx;
					pSlave->transform.ty = 5.0f;
					pSlave->transform.tz = pSourceTransform->tz;
					pSlave->transform.rx = ConvertToRadians(90.0f);
					pSlave->transform.ry = 0.0f;
					pSlave->transform.rz = 0.0f;
					break;
				default:
					break;
			}

			ApplyChangeToConnectionTree(pSlave, pPassTransform);
		}
	}

	void			Entity::DrawAll(Entity * pEntity, RenderContext & context, Effect & effect)
	{
		ASSERT(pEntity);
		effect.CBSetModelTransform(pEntity->transform.GetMatrix());
		pEntity->Draw(context);
		for ( TreeNode * pNode = FirstChild(pEntity); pNode; pNode = NextChild(pNode) )
		{
			DrawAll(static_cast< Entity * >( pNode ), context, effect);
		}
	}

	void			Camera::DrawObservedEntity(RenderContext & context, Effect & effect)
	{
		Entity::DrawAll(m_observedEntity, context, effect);
	}

	void			Controller::Initialize(RenderContext & context, VertexBuffer & vertexBuffer)
	{
		RenderWindow * pWindow;

		ENSURE_TRUE(
			static_cast< IUnknown && >( context.GetRenderTarget() ).QueryInterface(&pWindow));

		pWindow->RegisterEventListener(&this->GetOnMouseMoveEventHandler(),
					       &this->GetOnKeyDownEventHandler(),
					       &this->GetOnKeyUpEventHandler());
	}
	void			Controller::Update(double ms)
	{
		const Vector3 up	= { 0.0f, 1.0f, 0.0f };
		const Vector3 fwd	= { 0.0f, 0.0f, 1.0f };
		const Vector3 rt	= { 1.0f, 0.0f, 0.0f };

		float hRotRad;
		float vRotRad;
		Vector3 forwardDir;
		Vector3 rightDir;

		hRotRad		= ConvertToRadians(hRotDeg);
		vRotRad		= ConvertToRadians(Bound(-80.0f, vRotDeg, 80.0f));

		forwardDir	= V3Transform(fwd, M44RotationAxisLH(up, hRotRad));
		forwardDir.y	= 0.0f;
		forwardDir	= V3Normalize(forwardDir);

		rightDir	= V3CrossLH(up, forwardDir);
		rightDir.y	= 0.0f;
		rightDir	= V3Normalize(rightDir);

		float duration	= static_cast< float >( ms / 1000.0f );

		Vector3 delta =
		{
			( forwardFactor * forwardDir.x + rightFactor * rightDir.x ),
			( upFactor * up.y ),
			( forwardFactor * forwardDir.z + rightFactor * rightDir.z ),
		};
		delta		= V3Scale(V3Normalize(delta), duration * speed);
		
		if ( forwardFactor != 0.0f || rightFactor != 0.0f || upFactor != 0.0f )
		{
			pos	= pos + delta;
		}
		if ( hFactor != 0.0f )
		{
			hRotDeg	+= 0.2f * ms * hFactor;
		}
		if ( vFactor != 0.0f )
		{
			vRotDeg	+= 0.2f * ms * vFactor;
		}

		transform.translation.xyz = pos;
		transform.rx = -vRotRad;
		transform.ry = hRotRad;
		transform.rz = 0.0f;
		//printf("Mouse Pos: %d, %d, %d\n", pos.x, pos.y, pos.z);

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
			vRotDeg = Bound(-80.0f, vRotDeg, 80.0f);
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
			case 'Z': hFactor = 1.0f; break;
			case 'C': hFactor = -1.0f; break;
			case 'R': vFactor = 1.0f; break;
			case 'F': vFactor = -1.0f; break;
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
			case 'C': hFactor = 0.0f; break;
			case 'R':
			case 'F': vFactor = 0.0f; break;
			default: break;
		}
	}

	SceneRenderer::SceneRenderer(RenderWindow & window) : m_window(window)
		, m_scene(nullptr)
	{
		RenderTarget target;
		Rect rect;

		m_device		= Device::Default();

		rect			= Rect { 0, m_window.GetWidth(), 0, m_window.GetHeight() };
		target			= m_device.CreateRenderTarget(&m_window, rect);

		m_context		= m_device.CreateRenderContext();
		m_swapChain		= m_device.CreateSwapChain(target);
		m_depthStencilBuffer	= m_device.CreateDepthStencilBuffer(target.GetWidth(), target.GetHeight());

		m_context.SetSwapChain(m_swapChain);
		m_context.SetDepthStencilBuffer(m_depthStencilBuffer);
		m_context.SetRenderTarget(target);
	}
	void			SceneRenderer::SwitchScene(IScene & scene)
	{
		if ( m_scene )
		{
			m_scene->OnUnload();
		}
		m_scene = &scene;
		m_scene->OnLoad(m_device, m_context);
	}
	void			SceneRenderer::Present()
	{
		m_swapChain.Swap();
	}
	void			SceneRenderer::Clear()
	{
		m_swapChain.ResetBackBuffer();
		//m_swapChain.ResetBackBuffer(Rect { m_window.GetWidth() / 2, m_window.GetWidth(), 0, m_window.GetHeight() }, 50);
		m_depthStencilBuffer.ResetDepthBuffer();
	}
	void			SceneRenderer::Update(double ms)
	{
		if ( m_scene )
		{
			m_scene->OnUpdate(ms);
		}
	}
	void			SceneRenderer::Draw()
	{
		if ( m_scene )
		{
			m_scene->OnDraw();
		}
	}
}