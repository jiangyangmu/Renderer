#include "Scene.h"

namespace Graphics
{
	bool			Transform::GetInvertedMirroredMatrix(const Vec3 & posMirror, const Vec3 & normMirror, Matrix4x4 * pMirroredMatrix)
	{
		Vec3 dir = Vec3::Transform(Vec3::UnitZ(),
					   Matrix4x4::RotationAxisLH(Vec3::UnitY(), ry) *
					   Matrix4x4::RotationAxisLH(Vec3::UnitX(), rx) *
					   Matrix4x4::RotationAxisLH(Vec3::UnitZ(), rz));
		Vec3 dirInv = -dir;
		Vec3 dirRef = Vec3::Normalize(dir - Vec3::Scale(normMirror, 2 * Vec3::Dot(normMirror, dir)));

		float lenInvDir = dirInv.Length();
		float lenNorm = normMirror.Length();

		ASSERT(0.9f < lenInvDir && lenInvDir < 1.1f);
		ASSERT(0.9f < lenNorm && lenNorm < 1.1f);

		Vec3 posIntersect;
		if ( !ComputeRayPlaneIntersectionPoint(posMirror, normMirror, translation, dir, &posIntersect) )
		{
			return false;
		}

		float fDist = ( posIntersect - translation ).Length();
		Vec3 pos = posIntersect - Vec3::Scale(dirRef, fDist);

		float fACos = acosf(Vec3::Dot(dirInv, normMirror) / ( lenInvDir * lenNorm ));
		Vec3 axis = Vec3::CrossLH(dirInv, normMirror);

		// TODO: mirror camera should only map the mirror part of near plane to texture.
		*pMirroredMatrix =
			Matrix4x4::Translation(-pos.x, -pos.y, -pos.z) *
			Matrix4x4::RotationAxisLH(axis, PI - 2.0f * fACos) *
			Matrix4x4::RotationAxisLH(Vec3::UnitY(), -ry) *
			Matrix4x4::RotationAxisLH(Vec3::UnitX(), -rx) *
			Matrix4x4::RotationAxisLH(Vec3::UnitZ(), -rz)
			;

		return true;
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
					pSlave->transform.ty = -5.0f;
					pSlave->transform.tz = pSourceTransform->tz;
					pSlave->transform.rx = DegreeToRadian(-90.0f);
					pSlave->transform.ry = 0.0f;
					pSlave->transform.rz = 0.0f;
					break;
				default:
					break;
			}

			ApplyChangeToConnectionTree(pSlave, pPassTransform);
		}
	}

	void			Entity::DrawAll(Entity * pEntity, RenderContext & context)
	{
		ASSERT(pEntity);
		pEntity->Draw(context);
		for ( TreeNode * pNode = FirstChild(pEntity); pNode; pNode = NextChild(pNode) )
		{
			DrawAll(static_cast< Entity * >( pNode ), context);
		}
	}

	void			Camera::DrawObservedEntity(RenderContext & context)
	{
		Entity::DrawAll(m_observedEntity, context);
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
		vRotRad		= DegreeToRadian(Bound(-80.0f, vRotDeg, 80.0f));

		forwardDir	= Vec3::Transform(fwd, Matrix4x4::RotationAxisLH(up, hRotRad));
		forwardDir.y	= 0.0f;
		forwardDir	= Vec3::Normalize(forwardDir);

		rightDir	= Vec3::CrossLH(up, forwardDir);
		rightDir.y	= 0.0f;
		rightDir	= Vec3::Normalize(rightDir);

		float duration	= static_cast< float >( ms / 1000.0f );

		Vec3 delta =
		{
			( forwardFactor * forwardDir.x + rightFactor * rightDir.x ),
			( upFactor * up.y ),
			( forwardFactor * forwardDir.z + rightFactor * rightDir.z ),
		};
		delta		= Vec3::Scale(Vec3::Normalize(delta), duration * speed);
		
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

		transform.translation = pos;
		transform.rx = -vRotRad;
		transform.ry = hRotRad;
		transform.rz = 0.0f;

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
		m_context.SetOutputTarget(target);
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
		m_depthStencilBuffer.Reset();
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