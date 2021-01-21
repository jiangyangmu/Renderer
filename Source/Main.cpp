#include "Renderer.h"
#include "RenderWindow.h"
#include "Resource.h"
#include "Scene.h"
#include "win32/Win32App.h"

namespace Graphics
{
	class Effect
	{
	public:
		using VS = VertexShaderFunc;
		using PS = PixelShaderFunc;

		virtual			~Effect() = default;

		virtual void		Initialize(Device & device) = 0;
		virtual void		Apply(RenderContext & context) = 0;

		virtual void		CBSetViewTransform(const Matrix4x4 & viewTransform) = 0;
		virtual void		CBSetProjTransform(const Matrix4x4 & projTransform) = 0;

		VS			GetVS()
		{
			return m_vs;
		}
		PS			GetPS()
		{
			return m_ps;
		}
		VertexFormat		GetVSInputFormat()
		{
			return m_vsIn;
		}
		VertexFormat		GetVSOutputFormat()
		{
			return m_vsOut;
		}
		VertexFormat		GetPSInputFormat()
		{
			return m_psIn;
		}
		VertexFormat		GetPSOutputFormat()
		{
			return m_psOut;
		}

	protected:
		VS		m_vs;
		PS		m_ps;

		VertexFormat	m_vsIn;
		VertexFormat	m_vsOut;
		VertexFormat	m_psIn;
		VertexFormat	m_psOut;
	};

	class SimpleEffect : public Effect
	{
	public:
		SimpleEffect()
		{
			m_vs = VSImpl;
			m_ps = PSImpl;
		}

		virtual void		Initialize(Device & device) override
		{
			m_vsIn			= device.CreateVertexFormat(VertexFieldType::POSITION, VertexFieldType::COLOR);
			m_vsOut			= device.CreateVertexFormat(VertexFieldType::POSITION, VertexFieldType::SV_POSITION, VertexFieldType::COLOR);
			m_psIn			= m_vsOut;
			m_psOut			= device.CreateVertexFormat(VertexFieldType::COLOR);
			ASSERT(m_vsIn.Size() == sizeof(VS_IN));
			ASSERT(m_vsOut.Size() == sizeof(VS_OUT));
			ASSERT(m_psIn.Size() == sizeof(PS_IN));
			ASSERT(m_psOut.Size() == sizeof(PS_OUT));

			m_vertexShader		= device.CreateVertexShader(m_vs, m_vsIn, m_vsOut);
			m_pixelShader		= device.CreatePixelShader(m_ps, m_psIn, m_psOut);
		}
		virtual void		Apply(RenderContext & context) override
		{
			context.SetVertexShader(m_vertexShader);
			context.SetPixelShader(m_pixelShader);

			context.VSSetConstantBuffer(&m_vsData);
			context.PSSetConstantBuffer(&m_psData);
		}

		virtual void		CBSetViewTransform(const Matrix4x4 & viewTransform) override
		{
			m_vsData.view = m_psData.view = viewTransform;
		}
		virtual void		CBSetProjTransform(const Matrix4x4 & projTransform) override
		{
			m_vsData.proj = m_psData.proj = projTransform;
		}

	private:
		struct VS_IN
		{
			Vec3 posWld;
			Vec3 color;
		};
		struct VS_OUT
		{
			Vec3 posCam;
			Vec3 posNDC;
			Vec3 color;
		};
		struct VS_DATA
		{
			Matrix4x4 view;
			Matrix4x4 proj;
		};

		typedef VS_OUT PS_IN;
		typedef VS_DATA PS_DATA;
		struct PS_OUT
		{
			Vec3 color;
		};

		static void VSImpl(void * pVSOut, const void * pVSIn, const void * pContext)
		{
			const VS_IN & in	= *static_cast< const VS_IN * >( pVSIn );
			const VS_DATA & context	= *static_cast< const VS_DATA * >( pContext );
			VS_OUT & out		= *static_cast< VS_OUT * >( pVSOut );

			out.posCam		= Vec3::Transform(in.posWld, context.view);
			out.posNDC		= Vec3::Transform(out.posCam, context.proj);
			out.color		= in.color;
		}
		static void PSImpl(void * pPSOut, const void * pPSIn, const void * pContext)
		{
			const PS_IN & in	= *static_cast< const PS_IN * >( pPSIn );
			const PS_DATA & context	= *static_cast< const PS_DATA * >( pContext );
			PS_OUT & out		= *static_cast< PS_OUT * >( pPSOut );

			out.color		= in.color;
		}

	private:
		VertexShader	m_vertexShader;
		PixelShader	m_pixelShader;
		VS_DATA		m_vsData;
		PS_DATA		m_psData;
	};

	class TextureEffect : public Effect
	{
	public:
		TextureEffect(LPCWSTR lpTexFilePath)
			: m_texFilePath(lpTexFilePath)
		{
			m_vs = VSImpl;
			m_ps = PSImpl;
		}

		virtual void		Initialize(Device & device) override
		{
			m_vsIn			= device.CreateVertexFormat(VertexFieldType::POSITION, VertexFieldType::TEXCOORD);
			m_vsOut			= device.CreateVertexFormat(VertexFieldType::POSITION, VertexFieldType::SV_POSITION, VertexFieldType::TEXCOORD);
			m_psIn			= m_vsOut;
			m_psOut			= device.CreateVertexFormat(VertexFieldType::COLOR);
			ASSERT(m_vsIn.Size() == sizeof(VS_IN));
			ASSERT(m_vsOut.Size() == sizeof(VS_OUT));
			ASSERT(m_psIn.Size() == sizeof(PS_IN));
			ASSERT(m_psOut.Size() == sizeof(PS_OUT));

			m_vertexShader		= device.CreateVertexShader(m_vs, m_vsIn, m_vsOut);
			m_pixelShader		= device.CreatePixelShader(m_ps, m_psIn, m_psOut);


			Texture2D texture;
			{
				LONG nWidth;
				LONG nHeight;
				LPVOID lpPixelData = nullptr;

				win32::LoadBMP(m_texFilePath, &nWidth, &nHeight, &lpPixelData);
				ASSERT(nWidth > 0 && nHeight > 0 && lpPixelData != nullptr);

				texture		= device.CreateTexture2D(nWidth, nHeight, 4, 4, 0, lpPixelData);

				delete[] lpPixelData;
			}
			m_vsData.tex		= texture;
			m_psData.tex		= texture;
		}
		virtual void		Apply(RenderContext & context) override
		{
			context.SetVertexShader(m_vertexShader);
			context.SetPixelShader(m_pixelShader);

			context.VSSetConstantBuffer(&m_vsData);
			context.PSSetConstantBuffer(&m_psData);
		}

		virtual void		CBSetViewTransform(const Matrix4x4 & viewTransform) override
		{
			m_vsData.view = m_psData.view = viewTransform;
		}
		virtual void		CBSetProjTransform(const Matrix4x4 & projTransform) override
		{
			m_vsData.proj = m_psData.proj = projTransform;
		}

	private:
		struct VS_IN
		{
			Vec3 posWld;
			Vec2 uv;
		};
		struct VS_OUT
		{
			Vec3 posCam;
			Vec3 posNDC;
			Vec2 uv;
		};
		struct VS_DATA
		{
			Matrix4x4 view;
			Matrix4x4 proj;
			Texture2D tex;
		};

		typedef VS_OUT PS_IN;
		typedef VS_DATA PS_DATA;
		struct PS_OUT
		{
			Vec3 color;
		};

		static void VSImpl(void * pVSOut, const void * pVSIn, const void * pContext)
		{
			const VS_IN & in	= *static_cast< const VS_IN * >( pVSIn );
			const VS_DATA & context	= *static_cast< const VS_DATA * >( pContext );
			VS_OUT & out		= *static_cast< VS_OUT * >( pVSOut );

			out.posCam		= Vec3::Transform(in.posWld, context.view);
			out.posNDC		= Vec3::Transform(out.posCam, context.proj);
			out.uv			= in.uv;
		}
		static void PSImpl(void * pPSOut, const void * pPSIn, const void * pContext)
		{
			const PS_IN & in	= *static_cast< const PS_IN * >( pPSIn );
			const PS_DATA & context	= *static_cast< const PS_DATA * >( pContext );
			PS_OUT & out		= *static_cast< PS_OUT * >( pPSOut );

			context.tex.Sample(in.uv.x, in.uv.y, reinterpret_cast<float *>(&out.color));
		}

	private:
		VertexShader	m_vertexShader;
		PixelShader	m_pixelShader;
		VS_DATA		m_vsData;
		PS_DATA		m_psData;

		LPCWSTR		m_texFilePath;
	};

	class BlinnPhongEffect : public Effect
	{
	public:
		BlinnPhongEffect()
		{
			m_vs = VSImpl;
			m_ps = PSImpl;
		}

		virtual void		Initialize(Device & device) override
		{
			m_vsIn			= device.CreateVertexFormat(VertexFieldType::POSITION, VertexFieldType::NORMAL);
			m_vsOut			= device.CreateVertexFormat(VertexFieldType::POSITION, VertexFieldType::SV_POSITION, VertexFieldType::POSITION, VertexFieldType::NORMAL);
			m_psIn			= m_vsOut;
			m_psOut			= device.CreateVertexFormat(VertexFieldType::COLOR);
			ASSERT(m_vsIn.Size() == sizeof(VS_IN));
			ASSERT(m_vsOut.Size() == sizeof(VS_OUT));
			ASSERT(m_psIn.Size() == sizeof(PS_IN));
			ASSERT(m_psOut.Size() == sizeof(PS_OUT));

			m_vertexShader		= device.CreateVertexShader(m_vs, m_vsIn, m_vsOut);
			m_pixelShader		= device.CreatePixelShader(m_ps, m_psIn, m_psOut);

			m_vsData.material = m_psData.material = MaterialParams
			{
				{0.1f, 0.1f, 0.1f, 1.0f}, // ambient
				{1.0f, 0.0f, 0.0f, 1.0f}, // diffuse
				{0.0f, 0.0f, 1.0f, 1.0f}, // specular
			};

			m_vsData.light = m_psData.light = LightParams
			{
				{2.5f, 1.0f, 0.0f}, // position
				{1.0f, 0.0f, 0.0f}, // attenuation
				{1.0f, 1.0f, 1.0f, 1.0f}, // ambient
				{1.0f, 1.0f, 1.0f, 1.0f}, // diffuse
				{1.0f, 1.0f, 1.0f, 1.0f}, // specular
			};
		}
		virtual void		Apply(RenderContext & context) override
		{
			context.SetVertexShader(m_vertexShader);
			context.SetPixelShader(m_pixelShader);

			context.VSSetConstantBuffer(&m_vsData);
			context.PSSetConstantBuffer(&m_psData);
		}

		virtual void		CBSetViewTransform(const Matrix4x4 & viewTransform) override
		{
			m_vsData.view = m_psData.view = viewTransform;
		}
		virtual void		CBSetProjTransform(const Matrix4x4 & projTransform) override
		{
			m_vsData.proj = m_psData.proj = projTransform;
		}
		void			CBSetCameraPosition(const Vec3 & cameraPosWld)
		{
			m_vsData.cameraPosWld = m_psData.cameraPosWld = cameraPosWld;
		}

	private:
		struct MaterialParams
		{
			Vec4 rgbiAmbient;
			Vec4 rgbiDiffuse;
			Vec4 rgbiSpecular;
		};
		struct LightParams
		{
			Vec3 posWld;
			Vec3 attenuation;
			Vec4 rgbiAmbient;
			Vec4 rgbiDiffuse;
			Vec4 rgbiSpecular;
		};
		struct VS_IN
		{
			Vec3 posWld;
			Vec3 normWld;
		};
		struct VS_OUT
		{
			Vec3 posCam;
			Vec3 posNDC;
			Vec3 posWld;
			Vec3 normWld;
		};
		struct VS_DATA
		{
			Matrix4x4	view;
			Matrix4x4	proj;
			Vec3		cameraPosWld;
			MaterialParams	material;
			LightParams	light;
		};

		typedef VS_OUT PS_IN;
		typedef VS_DATA PS_DATA;
		struct PS_OUT
		{
			Vec3 color;
		};

		static void VSImpl(void * pVSOut, const void * pVSIn, const void * pContext)
		{
			const VS_IN & in	= *static_cast< const VS_IN * >( pVSIn );
			const VS_DATA & ctx	= *static_cast< const VS_DATA * >( pContext );
			VS_OUT & out		= *static_cast< VS_OUT * >( pVSOut );

			out.posCam		= Vec3::Transform(in.posWld, ctx.view);
			out.posNDC		= Vec3::Transform(out.posCam, ctx.proj);
			out.posWld		= in.posWld;
			out.normWld		= in.normWld;
		}
		static void PSImpl(void * pPSOut, const void * pPSIn, const void * pContext)
		{
			const PS_IN & in	= *static_cast< const PS_IN * >( pPSIn );
			const PS_DATA & ctx	= *static_cast< const PS_DATA * >( pContext );
			PS_OUT & out		= *static_cast< PS_OUT * >( pPSOut );

			ComputeBlinnPhong(&out.color, in.posWld, ctx.cameraPosWld, in.normWld, ctx.material, ctx.light);
		}
		static inline void ComputeBlinnPhong(Vec3 * rgb, Vec3 posWld, Vec3 eyeWld, Vec3 normWld, const MaterialParams & material, const LightParams & light)
		{
			Vec3 lightDir;
			float lightDistance;
			{
				lightDir	= posWld - light.posWld;
				lightDistance	= lightDir.Length();
				lightDir.Normalize();
			}


			// Ambient Color = C_material
			Vec3 ambient;
			{
				ambient	= Vec3::ElementwiseProduct(material.rgbiAmbient.xyz, light.rgbiAmbient.xyz);
			}

			// Diffuse Color = max( cos(-L, norm), 0) * ElementwiseProduce(C_light, C_material)
			Vec3 diffuse;
			{
				float decayFactor = Max(0.0f, Vec3::Dot(-lightDir, normWld));

				diffuse =
					Vec3::Scale(
						Vec3::ElementwiseProduct(light.rgbiDiffuse.xyz, material.rgbiDiffuse.xyz),
						decayFactor);
			}

			// Specular Color = max( cos(L', to-eye), 0) * ElementwiseProduce(C_light, C_material)
			Vec3 specular;
			{
				Vec3 reflectLightDir = Vec3::Normalize(lightDir - Vec3::Scale(normWld, 2 * Vec3::Dot(normWld, lightDir)));
				Vec3 toEyeDir = Vec3::Normalize(eyeWld - posWld);

				float decayFactor = Max(0.0f, Vec3::Dot(reflectLightDir, toEyeDir));
				decayFactor = decayFactor * decayFactor;
				decayFactor = decayFactor * decayFactor;
				decayFactor = decayFactor * decayFactor;

				specular =
					Vec3::Scale(
						Vec3::ElementwiseProduct(light.rgbiSpecular.xyz, material.rgbiSpecular.xyz),
						decayFactor);
			}

			float atteFactor = 1.0f / Vec3::Dot(light.attenuation, Vec3 { 1.0f, lightDistance, lightDistance * lightDistance });

			Vec3 color = WeightedAdd(ambient,
						 diffuse,
						 specular,
						 material.rgbiAmbient.w,
						 material.rgbiDiffuse.w * atteFactor,
						 material.rgbiSpecular.w * atteFactor);
			
			*rgb = Vec3
			{
				Bound(0.0f, color.x, 1.0f),
				Bound(0.0f, color.y, 1.0f),
				Bound(0.0f, color.z, 1.0f),
			};
		}

	private:
		VertexShader	m_vertexShader;
		PixelShader	m_pixelShader;
		VS_DATA		m_vsData;
		PS_DATA		m_psData;
	};

	class MyScene
	{
	public:
		void			OnLoad(Device & device, RenderContext & context)
		{
			m_device		= &device;
			m_context		= &context;

			// Setup shader

			m_rgbEffect.reset(new SimpleEffect());
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

			Scene * scene		= SceneManager::Default().CreateScene();

			m_rgbGroup		= SceneManager::Default().CreateEntityGroup();
			m_texGroup		= SceneManager::Default().CreateEntityGroup();
			m_bpGroup		= SceneManager::Default().CreateEntityGroup();
			Player * player		= SceneManager::Default().CreatePlayer();
			Animal * animal		= SceneManager::Default().CreateAnimal();
			Terrain * terrain	= SceneManager::Default().CreateTerrain();
			m_cameraPlayer		= SceneManager::Default().CreateCamera();
			m_cameraMiniMap		= SceneManager::Default().CreateCamera();
			m_controller		= SceneManager::Default().CreateController();

			scene->AddChild(m_cameraPlayer);
			scene->AddChild(m_cameraMiniMap);
			scene->AddChild(m_rgbGroup);
			scene->AddChild(m_texGroup);
			scene->AddChild(m_bpGroup);
			scene->AddChild(m_controller);

			m_rgbGroup->AddChild(terrain);
			m_texGroup->AddChild(player);
			m_bpGroup->AddChild(animal);

			m_cameraPlayer->SetAspectRatio(static_cast<float>(leftRect.right - leftRect.left) / (leftRect.bottom - leftRect.top));
			m_cameraMiniMap->SetAspectRatio(static_cast<float>(rightRect.right - rightRect.left) / (rightRect.bottom - rightRect.top));

			m_controller->ConnectTo(player, ConnectType::PLAYER);
			player->ConnectTo(m_cameraPlayer, ConnectType::THIRD_PERSON_VIEW);
			player->ConnectTo(m_cameraMiniMap, ConnectType::MINI_MAP_VIEW);

			m_scene			= scene;

			SceneObject::InitializeAll(m_scene, *m_context, m_rgbVertices);
			SceneObject::InitializeAll(m_scene, *m_context, m_texVertices);
			SceneObject::InitializeAll(m_scene, *m_context, m_bpVertices);
		}
		void			OnUnload()
		{
		}
		void			OnUpdate(double ms)
		{
			SceneObject::UpdateAll(m_scene, ms);
		}
		void			OnDraw()
		{
			RenderTarget * targets[]	= { &m_rdtgLeftRect, &m_rdtgRightRect };
			Camera * cameras[]		= { m_cameraPlayer, m_cameraMiniMap };
			Effect * effects[]		= { m_rgbEffect.get(), m_texEffect.get(), m_bpEffect.get() };
			EntityGroup * groups[]		= { m_rgbGroup, m_texGroup, m_bpGroup };

			for (Integer iView = 0; iView < 2; ++iView)
			{
				RenderTarget * target = targets[iView];
				Camera * camera = cameras[iView];

				m_context->SetOutputTarget(*target);

				for (Integer i = 0; i < 3; ++i)
				{
					Effect * effect = effects[i];
					EntityGroup * group = groups[i];

					effect->CBSetViewTransform(camera->GetViewTransform());
					effect->CBSetProjTransform(camera->GetProjTransform());
					if (i == 2) static_cast<BlinnPhongEffect *>(effect)->CBSetCameraPosition(m_controller->pos);
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
		VertexBuffer		m_rgbVertices; // referred by SceneObject
		VertexBuffer		m_texVertices; // referred by SceneObject
		VertexBuffer		m_bpVertices;
		Ptr<SimpleEffect>	m_rgbEffect;
		Ptr<TextureEffect>	m_texEffect;
		Ptr<BlinnPhongEffect>	m_bpEffect;

		Scene *			m_scene;
		Camera *		m_cameraPlayer;
		Camera *		m_cameraMiniMap;
		Controller *		m_controller;
	};

	class SceneRenderer : public IRenderer
	{
	public:
		SceneRenderer(RenderWindow & window)
			: m_window(window)
			, m_scene(nullptr)
		{
			RenderTarget target;
			Rect rect;

			m_device		= Device::Default();

			rect			= Rect { 0, m_window.GetWidth(), 0, m_window.GetHeight() };
			target			= m_device.CreateRenderTarget(&m_window, rect);

			m_context		= m_device.CreateRenderContext();
			m_swapChain		= m_device.CreateSwapChain(target.GetWidth(), target.GetHeight());
			m_depthStencilBuffer	= m_device.CreateDepthStencilBuffer(target.GetWidth(), target.GetHeight());

			m_context.SetSwapChain(m_swapChain);
			m_context.SetDepthStencilBuffer(m_depthStencilBuffer);
			m_context.SetOutputTarget(target);
		}

		void			SwitchScene(MyScene & scene)
		{
			if (m_scene)
			{
				m_scene->OnUnload();
			}
			m_scene = &scene;
			m_scene->OnLoad(m_device, m_context);
		}

		virtual void		Present() override
		{
			m_swapChain.Swap();
			m_window.Paint(m_window.GetWidth(), m_window.GetHeight(), m_swapChain.FrameBuffer());
		}
		virtual void		Clear() override
		{
			m_swapChain.ResetBackBuffer();
			m_swapChain.ResetBackBuffer(Rect { m_window.GetWidth() / 2, m_window.GetWidth(), 0, m_window.GetHeight() }, 50);
			m_depthStencilBuffer.Reset();
		}
		virtual void		Update(double ms) override
		{
			if ( m_scene )
			{
				m_scene->OnUpdate(ms);
			}
		}
		virtual void		Draw() override
		{
			if ( m_scene )
			{
				m_scene->OnDraw();
			}
		}

		// TODO: handle window resize

	private:
		RenderWindow &		m_window;
		
		Device			m_device;
		SwapChain		m_swapChain;
		RenderContext		m_context;
		DepthStencilBuffer	m_depthStencilBuffer;
		
		MyScene *		m_scene;
	};
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
		      _In_opt_ HINSTANCE hPrevInstance,
		      _In_ LPWSTR lpCmdLine,
		      _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	win32::Application app;

	int ret;
	{
		Integer W = 800;
		Integer H = 600;

		Graphics::RenderWindow renderWindow(L"My Renderer", hInstance, W, H);
		W = renderWindow.GetWidth();
		H = renderWindow.GetHeight();

		Graphics::SceneRenderer renderer(renderWindow);
		renderWindow.SetRenderer(renderer);

		Graphics::MyScene scene;
		renderer.SwitchScene(scene);

		ret = app.Run(renderWindow);
	}

	return ret;
}
