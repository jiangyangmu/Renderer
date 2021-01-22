#include "VisualEffects.h"

#include "win32/Win32App.h"

namespace Graphics
{

	RgbEffect::RgbEffect()
	{
		m_vs = VSImpl;
		m_ps = PSImpl;
	}
	void		RgbEffect::Initialize(Device & device)
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
	void		RgbEffect::Apply(RenderContext & context)
	{
		context.SetVertexShader(m_vertexShader);
		context.SetPixelShader(m_pixelShader);

		context.VSSetConstantBuffer(&m_vsData);
		context.PSSetConstantBuffer(&m_psData);
	}
	void		RgbEffect::CBSetViewTransform(const Matrix4x4 & viewTransform)
	{
		m_vsData.view = m_psData.view = viewTransform;
	}
	void		RgbEffect::CBSetProjTransform(const Matrix4x4 & projTransform)
	{
		m_vsData.proj = m_psData.proj = projTransform;
	}
	void		RgbEffect::VSImpl(void * pVSOut, const void * pVSIn, const void * pContext)
	{
		const VS_IN & in	= *static_cast< const VS_IN * >( pVSIn );
		const VS_DATA & context	= *static_cast< const VS_DATA * >( pContext );
		VS_OUT & out		= *static_cast< VS_OUT * >( pVSOut );

		out.posCam		= Vec3::Transform(in.posWld, context.view);
		out.posNDC		= Vec3::Transform(out.posCam, context.proj);
		out.color		= in.color;
	}
	void		RgbEffect::PSImpl(void * pPSOut, const void * pPSIn, const void * pContext)
	{
		const PS_IN & in	= *static_cast< const PS_IN * >( pPSIn );
		const PS_DATA & context	= *static_cast< const PS_DATA * >( pContext );
		PS_OUT & out		= *static_cast< PS_OUT * >( pPSOut );

		out.color		= in.color;
	}

	TextureEffect::TextureEffect(LPCWSTR lpTexFilePath) : m_texFilePath(lpTexFilePath)
	{
		m_vs = VSImpl;
		m_ps = PSImpl;
	}
	void		TextureEffect::Initialize(Device & device)
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
	void		TextureEffect::Apply(RenderContext & context)
	{
		context.SetVertexShader(m_vertexShader);
		context.SetPixelShader(m_pixelShader);

		context.VSSetConstantBuffer(&m_vsData);
		context.PSSetConstantBuffer(&m_psData);
	}
	void		TextureEffect::CBSetViewTransform(const Matrix4x4 & viewTransform)
	{
		m_vsData.view = m_psData.view = viewTransform;
	}
	void		TextureEffect::CBSetProjTransform(const Matrix4x4 & projTransform)
	{
		m_vsData.proj = m_psData.proj = projTransform;
	}
	void		TextureEffect::VSImpl(void * pVSOut, const void * pVSIn, const void * pContext)
	{
		const VS_IN & in	= *static_cast< const VS_IN * >( pVSIn );
		const VS_DATA & context	= *static_cast< const VS_DATA * >( pContext );
		VS_OUT & out		= *static_cast< VS_OUT * >( pVSOut );

		out.posCam		= Vec3::Transform(in.posWld, context.view);
		out.posNDC		= Vec3::Transform(out.posCam, context.proj);
		out.uv			= in.uv;
	}
	void		TextureEffect::PSImpl(void * pPSOut, const void * pPSIn, const void * pContext)
	{
		const PS_IN & in	= *static_cast< const PS_IN * >( pPSIn );
		const PS_DATA & context	= *static_cast< const PS_DATA * >( pContext );
		PS_OUT & out		= *static_cast< PS_OUT * >( pPSOut );

		context.tex.Sample(in.uv.x, in.uv.y, reinterpret_cast< float * >( &out.color ));
	}

	BlinnPhongEffect::BlinnPhongEffect(const MaterialParams & materialParams, const LightParams & lightParams)
	{
		m_vs = VSImpl;
		m_ps = PSImpl;

		m_vsData.material = m_psData.material = materialParams;
		m_vsData.light = m_psData.light = lightParams;
	}
	void		BlinnPhongEffect::Initialize(Device & device)
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
	}
	void		BlinnPhongEffect::Apply(RenderContext & context)
	{
		context.SetVertexShader(m_vertexShader);
		context.SetPixelShader(m_pixelShader);

		context.VSSetConstantBuffer(&m_vsData);
		context.PSSetConstantBuffer(&m_psData);
	}
	void		BlinnPhongEffect::CBSetViewTransform(const Matrix4x4 & viewTransform)
	{
		m_vsData.view = m_psData.view = viewTransform;
	}
	void		BlinnPhongEffect::CBSetProjTransform(const Matrix4x4 & projTransform)
	{
		m_vsData.proj = m_psData.proj = projTransform;
	}
	void		BlinnPhongEffect::CBSetCameraPosition(const Vec3 & cameraPosWld)
	{
		m_vsData.cameraPosWld = m_psData.cameraPosWld = cameraPosWld;
	}
	void		BlinnPhongEffect::VSImpl(void * pVSOut, const void * pVSIn, const void * pContext)
	{
		const VS_IN & in	= *static_cast< const VS_IN * >( pVSIn );
		const VS_DATA & ctx	= *static_cast< const VS_DATA * >( pContext );
		VS_OUT & out		= *static_cast< VS_OUT * >( pVSOut );

		out.posCam		= Vec3::Transform(in.posWld, ctx.view);
		out.posNDC		= Vec3::Transform(out.posCam, ctx.proj);
		out.posWld		= in.posWld;
		out.normWld		= in.normWld;
	}
	void		BlinnPhongEffect::PSImpl(void * pPSOut, const void * pPSIn, const void * pContext)
	{
		const PS_IN & in	= *static_cast< const PS_IN * >( pPSIn );
		const PS_DATA & ctx	= *static_cast< const PS_DATA * >( pContext );
		PS_OUT & out		= *static_cast< PS_OUT * >( pPSOut );

		ComputeBlinnPhong(&out.color, in.posWld, ctx.cameraPosWld, in.normWld, ctx.material, ctx.light);
	}
	void		BlinnPhongEffect::ComputeBlinnPhong(Vec3 * rgb, Vec3 posWld, Vec3 eyeWld, Vec3 normWld, const MaterialParams & material, const LightParams & light)
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
}