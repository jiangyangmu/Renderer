#include "TestScene.h"

#include "VisualEffects.h"

namespace Graphics
{
	class EffectTestScene : public IScene
	{
	public:
		void			OnLoad(Device & device, RenderContext & context)
		{
			m_device		= &device;
			m_context		= &context;

			// Setup shader

			m_rgbEffect.reset(new RgbEffect());
			m_texEffect.reset(new TextureEffect(L"Resources/grid.bmp"));
			m_bpEffect.reset(new BlinnPhongEffect());

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

			Root * root		= SceneManager::Default().CreateRoot();

			m_rgbGroup		= SceneManager::Default().CreateEntityGroup();
			m_texGroup		= SceneManager::Default().CreateEntityGroup();
			m_bpGroup		= SceneManager::Default().CreateEntityGroup();
			Player * player		= SceneManager::Default().CreatePlayer();
			Animal * animal		= SceneManager::Default().CreateAnimal();
			Terrain * terrain	= SceneManager::Default().CreateTerrain();
			m_cameraPlayer		= SceneManager::Default().CreateCamera();
			m_cameraMiniMap		= SceneManager::Default().CreateCamera();
			m_controller		= SceneManager::Default().CreateController();

			root->AddChild(m_cameraPlayer);
			root->AddChild(m_cameraMiniMap);
			root->AddChild(m_rgbGroup);
			root->AddChild(m_texGroup);
			root->AddChild(m_bpGroup);
			root->AddChild(m_controller);

			m_rgbGroup->AddChild(terrain);
			m_texGroup->AddChild(player);
			m_bpGroup->AddChild(animal);

			m_cameraPlayer->SetAspectRatio(static_cast< float >( leftRect.right - leftRect.left ) / ( leftRect.bottom - leftRect.top ));
			m_cameraMiniMap->SetAspectRatio(static_cast< float >( rightRect.right - rightRect.left ) / ( rightRect.bottom - rightRect.top ));

			m_controller->ConnectTo(player, ConnectType::PLAYER);
			player->ConnectTo(m_cameraPlayer, ConnectType::THIRD_PERSON_VIEW);
			player->ConnectTo(m_cameraMiniMap, ConnectType::MINI_MAP_VIEW);

			m_root			= root;

			SceneObject::InitializeAll(m_root, *m_context, m_rgbVertices);
			SceneObject::InitializeAll(m_root, *m_context, m_texVertices);
			SceneObject::InitializeAll(m_root, *m_context, m_bpVertices);
		}
		void			OnUnload()
		{
		}
		void			OnUpdate(double ms)
		{
			SceneObject::UpdateAll(m_root, ms);
		}
		void			OnDraw()
		{
			RenderTarget * targets[]	= { &m_rdtgLeftRect, &m_rdtgRightRect };
			Camera * cameras[]		= { m_cameraPlayer, m_cameraMiniMap };
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
		Device *		m_device;
		RenderContext *		m_context;

		RenderTarget		m_rdtgLeftRect;
		RenderTarget		m_rdtgRightRect;

		EntityGroup *		m_rgbGroup;
		EntityGroup *		m_texGroup;
		EntityGroup *		m_bpGroup;
		
		VertexBuffer		m_rgbVertices;
		VertexBuffer		m_texVertices;
		VertexBuffer		m_bpVertices;
		
		Ptr<RgbEffect>		m_rgbEffect;
		Ptr<TextureEffect>	m_texEffect;
		Ptr<BlinnPhongEffect>	m_bpEffect;

		Root *			m_root;
		Camera *		m_cameraPlayer;
		Camera *		m_cameraMiniMap;
		Controller *		m_controller;
	};

	Ptr<IScene>	CreateTestScene_Effects()
	{
		return Ptr<IScene>(new EffectTestScene());
	}
}