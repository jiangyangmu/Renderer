#include "TestScene.h"

namespace Graphics
{
	// --------------------------------------------------------------------------
	// Scene Objects
	// --------------------------------------------------------------------------

	namespace
	{
		struct Mirror : Entity
		{
			Ptr<Renderable>		m_renderable;
			Vector3			m_center;
			float			m_size;

			Mirror(Vector3 center, float size)
				: m_center(center)
				, m_size(size)
			{
			}
			virtual void		Initialize(RenderContext & context, VertexBuffer & vertexBuffer) override
			{
				ENSURE_TRUE(ROTexRectangle::IsVertexFormatCompatible(vertexBuffer.GetVertexFormat()));

				m_renderable.reset(new ROTexRectangle(m_center, m_size, m_size, 0.0f, m_size, m_size, 0.0f));
				m_renderable->Initialize(vertexBuffer);
			}
			virtual void		Update(double ms) override
			{
				m_renderable->Update(ms);
			}
			virtual void		Draw(RenderContext & context) override
			{
				m_renderable->Draw(context);
			}
		};

		struct TextureCube : Entity
		{
			Ptr<Renderable>		m_renderable;
			Vector3			m_center;
			float			m_size;

			TextureCube(Vector3 center, float size)
				: m_center(center)
				, m_size(size)
			{
			}
			virtual void		Initialize(RenderContext & context, VertexBuffer & vertexBuffer) override
			{
				ENSURE_TRUE(ROCube::IsVertexFormatCompatible(vertexBuffer.GetVertexFormat()));

				m_renderable.reset(new ROCube(m_center, m_size));
				m_renderable->Initialize(vertexBuffer);
			}
			virtual void		Update(double ms) override
			{
				m_renderable->Update(ms);
			}
			virtual void		Draw(RenderContext & context) override
			{
				m_renderable->Draw(context);
			}
		};

		struct CubeChunk : Entity
		{
			std::vector<TextureCube>	m_cubes;

			CubeChunk(Integer nWidth, Integer nHeight, Integer nDepth)
			{
				m_cubes.reserve(nWidth * nHeight * nDepth);
				
				for (Integer h = nHeight - 1; h >= 0; --h)
				{
					for (Integer d = 0; d < nDepth; ++d)
					{
						for (Integer w = 0; w < nWidth; ++w)
						{
							m_cubes.emplace_back(Vector3 { w + 0.5f, h + 0.5f, d + 0.5f}, 1.0f);
						}
					}
				}
			}
			virtual void		Initialize(RenderContext & context, VertexBuffer & vertexBuffer) override
			{
				ENSURE_TRUE(ROCube::IsVertexFormatCompatible(vertexBuffer.GetVertexFormat()));

				for ( TextureCube & cube : m_cubes )
				{
					cube.m_renderable.reset(new ROCube(cube.m_center, cube.m_size));
					cube.m_renderable->Initialize(vertexBuffer);
				}
			}
			virtual void		Update(double ms) override
			{
				for ( TextureCube & cube : m_cubes )
				{
					cube.m_renderable->Update(ms);
				}
			}
			virtual void		Draw(RenderContext & context) override
			{
				for ( TextureCube & cube : m_cubes )
				{
					cube.m_renderable->Draw(context);
				}
			}
		};
	}

	// --------------------------------------------------------------------------
	// Scene
	// --------------------------------------------------------------------------

	class TestScene_Mirror : public IScene
	{
	public:
		virtual void			OnLoad(Device & device, RenderContext & context) override
		{
			m_device		= &device;
			m_ctxScreen		= &context;
			m_depthStencilBuffer	= context.GetDepthStencilBuffer();

			// Setup terrain resources

			m_efObject.reset(new TextureEffect(L"Resources/grid.bmp"));
			m_efObject->Initialize(device);

			// Setup mirror resources

			m_efMirror.reset(new TextureEffect(L"Resources/grey.bmp"));
			m_efMirror->Initialize(device);

			// Setup shared resources

			// ASSERT(m_efObject->GetVSInputFormat() == m_efMirror->GetVSInputFormat());
			m_vbTexture		= m_device->CreateVertexBuffer(m_efObject->GetVSInputFormat());

			// Setup display

			Rect rect		= context.GetRenderTarget().GetRect();
			Rect leftRect		= Rect { 0, rect.right / 2, 0, rect.bottom };
			Rect rightRect		= Rect { rect.right / 2, rect.right, 0, rect.bottom };

			m_rdtgScreen		= context.GetRenderTarget();
			m_rdtgLeft		= device.CreateRenderTarget(context.GetRenderTarget(), leftRect);
			m_rdtgRight		= device.CreateRenderTarget(context.GetRenderTarget(), rightRect);

			m_fScreenAspectRatio	= static_cast< float >( rect.right - rect.left ) / ( rect.bottom - rect.top );
			m_fLeftAspectRatio	= static_cast< float >( leftRect.right - leftRect.left ) / ( leftRect.bottom - leftRect.top );
			m_fRightAspectRatio	= static_cast< float >( rightRect.right - rightRect.left ) / ( rightRect.bottom - rightRect.top );

			// Setup scene

			m_root			= NewObject<Root>();
			m_camera		= NewObject<Camera>();
			m_controller		= NewObject<Controller>();

			m_terrain		= NewObject<CubeChunk>(5, 1, 5);
			m_terrain->transform.translation = {-2.5f, 0.0f, -2.5f};
			
			m_mirror1		= NewObject<Mirror>(Vector3 { 0.0f, 0.0f, 5.0f }, 10.0f);
			m_mirror2		= NewObject<Mirror>(Vector3 { 0.0f, 0.0f, 0.0f }, 10.0f);
			m_mirror2->transform.ty = -5.0f;
			m_mirror2->transform.rx = ConvertToRadians(90.0f);
			
			m_camera->SetAspectRatio(m_fScreenAspectRatio);

			m_controller->ConnectTo(m_camera, ConnectType::SAME);
			m_controller->pos = {6.0f, 3.0f, -6.0f};
			m_controller->hRotDeg = -40.0f;
			m_controller->vRotDeg = -30.0f;

			m_root->AddChild(m_camera);
			m_root->AddChild(m_controller);
			m_root->AddChild(m_terrain);
			m_root->AddChild(m_mirror1);
			m_root->AddChild(m_mirror2);
			SceneObject::InitializeAll(m_root, *m_ctxScreen, m_vbTexture);
		}
		virtual void			OnUnload() override
		{
		}
		virtual void			OnUpdate(double ms) override
		{
			SceneObject::UpdateAll(m_root, ms);
		}
		virtual void			OnDraw() override
		{
			DepthStencilState dssDefault = { true, true, DepthWriteMask::ALL, 0 };
			DepthStencilState dssWriteStencil = { true, false, DepthWriteMask::ZERO, 0xff };

			Mirror * pMirrorList[] = { m_mirror1, m_mirror2 };

			for ( Mirror * pMirror : pMirrorList )
			{
				m_ctxScreen->OMSetDepthStencilState(dssDefault);
			
				// 1. reset stencil to 1
				m_depthStencilBuffer.ResetStencilBuffer(1);

				// 2. main cam - draw object
				m_ctxScreen->RSSetFlipHorizontal(false);
				m_efObject->CBSetViewTransform(m_camera->GetViewTransform());
				m_efObject->CBSetProjTransform(m_camera->GetProjTransform());
				m_efObject->Apply(*m_ctxScreen);
				m_camera->ObserveEntity(m_terrain);
				m_camera->DrawObservedEntity(*m_ctxScreen, *m_efObject);

				// 3. reset stencil to 0, enable stencil write, disable depth write
				m_ctxScreen->OMSetDepthStencilState(dssWriteStencil);
				m_depthStencilBuffer.ResetStencilBuffer(0);
			
				// 4. draw mirror to stencil
				m_efMirror->CBSetViewTransform(m_camera->GetViewTransform());
				m_efMirror->CBSetProjTransform(m_camera->GetProjTransform());
				m_efMirror->Apply(*m_ctxScreen);
				m_camera->ObserveEntity(pMirror);
				m_camera->DrawObservedEntity(*m_ctxScreen, *m_efMirror);

				// 5. disable stencil write, enable depth write
				m_depthStencilBuffer.ResetDepthBuffer();
				m_ctxScreen->OMSetDepthStencilState(dssDefault);
				m_ctxScreen->RSSetFlipHorizontal(true);

				// 6. mirror cam - draw object
				Matrix44 viewTransform;
				Vector3 posMirror = pMirror->transform.translation.xyz + pMirror->m_center;
				Vector3 normMirror = V3Transform(-V3UnitZ(), pMirror->transform.GetRotationXYZMatrix());
				m_camera->transform.GetInvertedMirroredMatrix(posMirror, normMirror, &viewTransform);
				m_efObject->CBSetViewTransform(viewTransform);
				m_efObject->CBSetProjTransform(m_camera->GetProjTransform());
				m_efObject->Apply(*m_ctxScreen);
				m_camera->ObserveEntity(m_terrain);
				m_camera->DrawObservedEntity(*m_ctxScreen, *m_efObject);
			}
		}

	private:
		template <typename T, typename ... TArgs>
		T *			NewObject(TArgs ... args)
		{
			T * pObject = new T(args ...);
			m_sceneObjects.emplace_back(Ptr<T>(pObject));
			return pObject;
		}

		Device *			m_device;
		RenderContext *			m_ctxScreen;
		DepthStencilBuffer		m_depthStencilBuffer;

		RenderTarget			m_rdtgScreen;
		RenderTarget			m_rdtgLeft;
		RenderTarget			m_rdtgRight;

		float				m_fScreenAspectRatio;
		float				m_fLeftAspectRatio;
		float				m_fRightAspectRatio;

		// Scene structure
		std::vector<Ptr<SceneObject>>	m_sceneObjects;
		Root *				m_root;
		Camera *			m_camera;
		Controller *			m_controller;
		CubeChunk *			m_terrain;
		Mirror *			m_mirror1;
		Mirror *			m_mirror2;

		// Shared resources
		VertexBuffer			m_vbTexture;

		// Terrain resources
		Ptr<TextureEffect>		m_efObject;

		// Mirror resources
		Ptr<TextureEffect>		m_efMirror;
	};


	Ptr<IScene>	CreateTestScene_Mirror()
	{
		return Ptr<IScene>(new TestScene_Mirror());
	}
}