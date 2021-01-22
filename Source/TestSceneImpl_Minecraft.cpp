#include "TestScene.h"

#include "VisualEffects.h"

namespace Graphics
{
	// --------------------------------------------------------------------------
	// Scene Objects
	// --------------------------------------------------------------------------

	namespace
	{
		struct TextureCube : Entity
		{
			Ptr<Renderable>		m_renderable;
			Vec3			m_center;
			float			m_size;

			TextureCube(Vec3 center, float size)
				: m_center(center)
				, m_size(size)
			{
			}
			virtual void		Initialize(RenderContext & context, VertexBuffer & vertexBuffer) override
			{
				ENSURE_TRUE(ROCube::IsVertexFormatCompatible(vertexBuffer.GetVertexFormat()));

				m_renderable.reset(new ROCube(m_center, m_size));
				m_renderable->Initialize(context, vertexBuffer);
			}
			virtual void		Update(double ms) override
			{
				m_renderable->Update(ms);
			}
			virtual void		Draw() override
			{
				m_renderable->Draw();
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
						m_cubes.emplace_back(Vec3 { ( float ) row, -1.0f, ( float ) col }, 1.0f);
					}
				}
			}
			virtual void		Initialize(RenderContext & context, VertexBuffer & vertexBuffer) override
			{
				ENSURE_TRUE(ROCube::IsVertexFormatCompatible(vertexBuffer.GetVertexFormat()));

				for ( TextureCube & cube : m_cubes )
				{
					cube.m_renderable.reset(new ROCube(cube.m_center, cube.m_size));
					cube.m_renderable->Initialize(context, vertexBuffer);
				}
			}
			virtual void		Update(double ms) override
			{
				for ( TextureCube & cube : m_cubes )
				{
					cube.m_renderable->Update(ms);
				}
			}
			virtual void		Draw() override
			{
				for ( TextureCube & cube : m_cubes )
				{
					cube.m_renderable->Draw();
				}
			}
		};
	}

	// --------------------------------------------------------------------------
	// Scene
	// --------------------------------------------------------------------------

	class TestScene1 : public IScene
	{
	public:
		void			OnLoad(Device & device, RenderContext & context)
		{
			m_device		= &device;
			m_context		= &context;

			// Setup shader

			m_texEffect.reset(new TextureEffect(L"Resources/grass.bmp"));
			m_texEffect->Initialize(device);

			m_texVertices		= m_device->CreateVertexBuffer(m_texEffect->GetVSInputFormat());

			// Setup display

			Rect rect		= context.GetOutputTarget().GetRect();

			// Setup scene

			m_root			= NewObject<Root>();
			m_texGroup		= NewObject<EntityGroup>();
			m_camera		= NewObject<Camera>();
			m_terrain		= NewObject<Terrain>(5);
			m_controller		= NewObject<Controller>();

			m_root->AddChild(m_camera);
			m_root->AddChild(m_texGroup);
			m_root->AddChild(m_controller);

			m_texGroup->AddChild(m_terrain);

			m_camera->SetAspectRatio(static_cast< float >( rect.right - rect.left ) / ( rect.bottom - rect.top ));

			m_controller->ConnectTo(m_camera, ConnectType::SAME);

			SceneObject::InitializeAll(m_root, *m_context, m_texVertices);
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
			m_texEffect->CBSetViewTransform(m_camera->GetViewTransform());
			m_texEffect->CBSetProjTransform(m_camera->GetProjTransform());
			m_texEffect->Apply(*m_context);
			m_camera->ObserveEntity(m_texGroup);
			m_camera->DrawObservedEntity();
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
		RenderContext *			m_context;

		std::vector<Ptr<SceneObject>>	m_sceneObjects;

		EntityGroup *			m_texGroup;
		VertexBuffer			m_texVertices;
		Ptr<TextureEffect>		m_texEffect;

		Root *				m_root;
		Camera *			m_camera;
		Controller *			m_controller;
		Terrain *			m_terrain;
	};

	Ptr<IScene>	CreateTestScene_Minecraft()
	{
		return Ptr<IScene>(new TestScene1());
	}
}