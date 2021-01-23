#include "TestScene.h"

#include "VisualEffects.h"

namespace Graphics
{
	// --------------------------------------------------------------------------
	// Scene Objects
	// --------------------------------------------------------------------------

	namespace
	{
		struct Player : Entity
		{
		};

		struct TextureCube : Entity
		{
			Ptr<Renderable>		renderable;

			virtual void		Initialize(RenderContext & context, VertexBuffer & vertexBuffer) override
			{
				if ( ROCube::IsVertexFormatCompatible(vertexBuffer.GetVertexFormat()) )
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

		struct BPCube : Entity
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

		struct RgbTriangle : Entity
		{
			Ptr<Renderable>		renderable;

			virtual void		Initialize(RenderContext & context, VertexBuffer & vertexBuffer) override
			{
				if ( ROTriangle::IsVertexFormatCompatible(vertexBuffer.GetVertexFormat()) )
				{
					renderable.reset(new ROTriangle({ -1.0f,   0.0f, 3.0f }, { 1.0f, 0.0f, 0.0f },
									{ 0.0f,   0.0f, 3.0f }, { 0.0f, 1.0f, 0.0f },
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
	}

	// --------------------------------------------------------------------------
	// Scene
	// --------------------------------------------------------------------------

	class EffectTestScene : public IScene
	{
	public:
		virtual void			OnLoad(Device & device, RenderContext & context) override
		{
			m_device		= &device;
			m_context		= &context;

			// Setup shader

			m_rgbEffect.reset(new RgbEffect());
			m_texEffect.reset(new TextureEffect(L"Resources/grid.bmp"));
			m_bpEffect.reset(new BlinnPhongEffect(
				BlinnPhongEffect::MaterialParams
				{ {0.1f, 0.1f, 0.1f, 1.0f},	// ambient
				{1.0f, 0.0f, 0.0f, 1.0f},	// diffuse
				{0.0f, 0.0f, 1.0f, 1.0f} },	// specular
				BlinnPhongEffect::LightParams
				{ {2.5f, 1.0f, 0.0f},		// position
				{1.0f, 0.0f, 0.0f},		// attenuation
				{1.0f, 1.0f, 1.0f, 1.0f},	// ambient
				{1.0f, 1.0f, 1.0f, 1.0f},	// diffuse
				{1.0f, 1.0f, 1.0f, 1.0f} }	// specular
			));

			m_rgbEffect->Initialize(device);
			m_texEffect->Initialize(device);
			m_bpEffect->Initialize(device);

			m_rgbVertices		= m_device->CreateVertexBuffer(m_rgbEffect->GetVSInputFormat());
			m_texVertices		= m_device->CreateVertexBuffer(m_texEffect->GetVSInputFormat());
			m_bpVertices		= m_device->CreateVertexBuffer(m_bpEffect->GetVSInputFormat());

			// Setup display

			Rect rect		= context.GetOutputTarget().GetRect();
			Rect leftRect		= Rect { 0, rect.right / 2, 0, rect.bottom };
			Rect rightRect		= Rect { rect.right / 2, rect.right, 0, rect.bottom };

			m_rdtgLeftRect		= device.CreateRenderTarget(context.GetOutputTarget(), leftRect);
			m_rdtgRightRect		= device.CreateRenderTarget(context.GetOutputTarget(), rightRect);

			// Setup scene

			m_root			= NewObject<Root>();
			m_rgbGroup		= NewObject<EntityGroup>();
			m_texGroup		= NewObject<EntityGroup>();
			m_bpGroup		= NewObject<EntityGroup>();
			m_cameraMain		= NewObject<Camera>();
			m_cameraTopView		= NewObject<Camera>();
			m_controller		= NewObject<Controller>();

			Player * player		= NewObject<Player>();
			RgbTriangle * triangle	= NewObject<RgbTriangle>();
			TextureCube * cube1	= NewObject<TextureCube>();
			BPCube * cube2		= NewObject<BPCube>();

			m_root->AddChild(m_cameraMain);
			m_root->AddChild(m_cameraTopView);
			m_root->AddChild(m_rgbGroup);
			m_root->AddChild(m_texGroup);
			m_root->AddChild(m_bpGroup);
			m_root->AddChild(m_controller);
			m_root->AddChild(player);

			m_rgbGroup->AddChild(triangle);
			m_texGroup->AddChild(cube1);
			m_bpGroup->AddChild(cube2);

			m_cameraMain->SetAspectRatio(static_cast< float >( leftRect.right - leftRect.left ) / ( leftRect.bottom - leftRect.top ));
			m_cameraTopView->SetAspectRatio(static_cast< float >( rightRect.right - rightRect.left ) / ( rightRect.bottom - rightRect.top ));

			m_controller->ConnectTo(player, ConnectType::SAME);
			player->ConnectTo(m_cameraMain, ConnectType::THIRD_PERSON_VIEW);
			player->ConnectTo(m_cameraTopView, ConnectType::MINI_MAP_VIEW);

			SceneObject::InitializeAll(m_root, *m_context, m_rgbVertices);
			SceneObject::InitializeAll(m_root, *m_context, m_texVertices);
			SceneObject::InitializeAll(m_root, *m_context, m_bpVertices);
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
			RenderTarget * targets[]	= { &m_rdtgLeftRect, &m_rdtgRightRect };
			Camera * cameras[]		= { m_cameraMain, m_cameraTopView };
			Effect * effects[]		= { m_rgbEffect.get(), m_texEffect.get(), m_bpEffect.get() };
			EntityGroup * groups[]		= { m_rgbGroup, m_texGroup, m_bpGroup };

			for ( Integer iView = 0; iView < 2; ++iView )
			{
				RenderTarget * target = targets[ iView ];
				Camera * camera = cameras[ iView ];

				m_context->SetOutputTarget(*target);

				for ( Integer i = 0; i < 3; ++i )
				{
					Effect * effect = effects[ i ];
					EntityGroup * group = groups[ i ];

					effect->CBSetViewTransform(camera->GetViewTransform());
					effect->CBSetProjTransform(camera->GetProjTransform());
					if ( i == 2 ) static_cast< BlinnPhongEffect * >( effect )->CBSetCameraPosition(m_controller->pos);
					effect->Apply(*m_context);
					camera->ObserveEntity(group);
					camera->DrawObservedEntity();
				}
			}
		}

	private:
		template <typename T>
		T *			NewObject()
		{
			T * pObject = new T();
			m_sceneObjects.emplace_back(Ptr<T>(pObject));
			return pObject;
		}

		Device *			m_device;
		RenderContext *			m_context;

		RenderTarget			m_rdtgLeftRect;
		RenderTarget			m_rdtgRightRect;

		VertexBuffer			m_rgbVertices;
		VertexBuffer			m_texVertices;
		VertexBuffer			m_bpVertices;

		Ptr<RgbEffect>			m_rgbEffect;
		Ptr<TextureEffect>		m_texEffect;
		Ptr<BlinnPhongEffect>		m_bpEffect;

		std::vector<Ptr<SceneObject>>	m_sceneObjects;
		Root *				m_root;
		Camera *			m_cameraMain;
		Camera *			m_cameraTopView;
		Controller *			m_controller;
		EntityGroup *			m_rgbGroup;
		EntityGroup *			m_texGroup;
		EntityGroup *			m_bpGroup;
	};


	Ptr<IScene>	CreateTestScene_Effects()
	{
		return Ptr<IScene>(new EffectTestScene());
	}
}