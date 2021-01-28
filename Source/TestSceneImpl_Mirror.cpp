#include "TestScene.h"

#include "VisualEffects.h"

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

				m_renderable.reset(new ROTexRectangle(m_center, m_size, m_size, 0.0f, 1.0f, 1.0f, 0.0f));
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

		struct Terrain : Entity
		{
			std::vector<TextureCube>	m_cubes;

			Terrain(Integer size)
			{
				m_cubes.reserve(size * size);
				for ( Integer row = -size / 2; row <= size / 2; ++row )
				{
					for ( Integer col = -size / 2; col <= size / 2; ++col )
					{
						m_cubes.emplace_back(Vector3 { ( float ) row, -0.5f, ( float ) col }, 1.0f);
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

			// Setup terrain resources

			m_efGrass.reset(new TextureEffect(L"Resources/grass.bmp"));
			m_efGrass->Initialize(device);

			// Setup mirror resources

			m_ctxMirror		= m_device->CreateRenderContext();
			m_texMirror		= m_device->CreateTexture2D(256, 256, 3, 4, 0, nullptr);
			m_rtMirror		= m_device->CreateRenderTarget(m_texMirror, Rect { 0, 256, 0, 256 });
			m_scMirror		= m_device->CreateSwapChain(m_rtMirror);
			m_dsbMirror		= m_device->CreateDepthStencilBuffer(256, 256);

			m_ctxMirror.SetSwapChain(m_scMirror);
			m_ctxMirror.SetDepthStencilBuffer(m_dsbMirror);
			m_ctxMirror.SetOutputTarget(m_rtMirror);
			
			m_efMirror.reset(new TextureEffect(m_texMirror));
			m_efMirror->Initialize(device);

			// Setup shared resources

			// ASSERT(m_efGrass->GetVSInputFormat() == m_efMirror->GetVSInputFormat());
			m_vbTexture		= m_device->CreateVertexBuffer(m_efGrass->GetVSInputFormat());

			// Setup display

			Rect rect		= context.GetOutputTarget().GetRect();
			m_fScreenAspectRatio	= static_cast< float >( rect.right - rect.left ) / ( rect.bottom - rect.top );

			// Setup scene

			m_root			= NewObject<Root>();
			m_camera		= NewObject<Camera>();
			m_controller		= NewObject<Controller>();
			
			m_terrain		= NewObject<Terrain>(5);
			m_mirror		= NewObject<Mirror>(Vector3{0.0f, 2.5f, 2.5f}, 5.0f);

			m_controller->ConnectTo(m_camera, ConnectType::SAME);
			m_controller->pos = {5.0f, 3.0f, -5.0f};
			m_controller->hRotDeg = -45.0f;
			m_controller->vRotDeg = -10.0f;

			m_root->AddChild(m_camera);
			m_root->AddChild(m_controller);
			m_root->AddChild(m_terrain);
			m_root->AddChild(m_mirror);
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
			// Draw mirror texture

			m_camera->SetAspectRatio(1.0f);

			Matrix44 viewTransform;
			if ( m_camera->transform.GetInvertedMirroredMatrix(m_mirror->transform.translation + m_mirror->m_center,
									   -V3UnitZ(),
									   &viewTransform) )
			{
				m_scMirror.ResetBackBuffer(100);
				m_dsbMirror.Reset();
				m_efGrass->CBSetViewTransform(viewTransform);
				m_efGrass->CBSetProjTransform(m_camera->GetProjTransform());
				m_efGrass->Apply(m_ctxMirror);
				m_camera->ObserveEntity(m_terrain);
				m_camera->DrawObservedEntity(m_ctxMirror);
				m_scMirror.Swap();
			}

			// Draw terrain and mirror

			m_camera->SetAspectRatio(m_fScreenAspectRatio);

			m_efGrass->CBSetViewTransform(m_camera->GetViewTransform());
			m_efGrass->CBSetProjTransform(m_camera->GetProjTransform());
			m_efGrass->Apply(*m_ctxScreen);
			m_camera->ObserveEntity(m_terrain);
			m_camera->DrawObservedEntity(*m_ctxScreen);
			
			m_efMirror->CBSetViewTransform(m_camera->GetViewTransform());
			m_efMirror->CBSetProjTransform(m_camera->GetProjTransform());
			m_efMirror->Apply(*m_ctxScreen);
			m_camera->ObserveEntity(m_mirror);
			m_camera->DrawObservedEntity(*m_ctxScreen);
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
		float				m_fScreenAspectRatio;

		// Scene structure
		std::vector<Ptr<SceneObject>>	m_sceneObjects;
		Root *				m_root;
		Camera *			m_camera;
		Controller *			m_controller;
		Terrain *			m_terrain;
		Mirror *			m_mirror;

		// Shared resources
		VertexBuffer			m_vbTexture;

		// Terrain resources
		Ptr<TextureEffect>		m_efGrass;

		// Mirror resources
		Ptr<TextureEffect>		m_efMirror;
		RenderContext			m_ctxMirror;
		SwapChain			m_scMirror;
		DepthStencilBuffer		m_dsbMirror;
		RenderTarget			m_rtMirror;
		Texture2D			m_texMirror;
	};


	Ptr<IScene>	CreateTestScene_Mirror()
	{
		return Ptr<IScene>(new TestScene_Mirror());
	}
}