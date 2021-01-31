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

		m_vsData.model		= M44Identity();
		m_psData.model		= M44Identity();
	}
	void		RgbEffect::Apply(RenderContext & ctx)
	{
		ctx.SetVertexShader(m_vertexShader);
		ctx.SetPixelShader(m_pixelShader);

		ctx.VSSetConstantBuffer(&m_vsData);
		ctx.PSSetConstantBuffer(&m_psData);
	}
	void		RgbEffect::CBSetModelTransform(const Matrix44 & modelTransform)
	{
		m_vsData.model = m_psData.model = modelTransform;
	}
	void		RgbEffect::CBSetViewTransform(const Matrix44 & viewTransform)
	{
		m_vsData.view = m_psData.view = viewTransform;
	}
	void		RgbEffect::CBSetProjTransform(const Matrix44 & projTransform)
	{
		m_vsData.proj = m_psData.proj = projTransform;
	}
	void		RgbEffect::VSImpl(void * pVSOut, const void * pVSIn, const void * pContext)
	{
		const VS_IN & in	= *static_cast< const VS_IN * >( pVSIn );
		const VS_DATA & ctx	= *static_cast< const VS_DATA * >( pContext );
		VS_OUT & out		= *static_cast< VS_OUT * >( pVSOut );

		out.posCam		= V3Transform(in.posWld, M44Multiply(ctx.model, ctx.view));
		out.posNDC		= V3Transform(out.posCam, ctx.proj);
		out.color		= in.color;
	}
	void		RgbEffect::PSImpl(void * pPSOut, const void * pPSIn, const void * pContext)
	{
		const PS_IN & in	= *static_cast< const PS_IN * >( pPSIn );
		const PS_DATA & ctx	= *static_cast< const PS_DATA * >( pContext );
		PS_OUT & out		= *static_cast< PS_OUT * >( pPSOut );

		out.color		= in.color;
	}

	TextureEffect::TextureEffect(LPCWSTR lpTexFilePath)
		: m_texFilePath(lpTexFilePath)
	{
		m_vs = VSImpl;
		m_ps = PSImpl;
	}
	TextureEffect::TextureEffect(Texture2D texture2D)
		: m_texFilePath(NULL)
		, m_texture2D(texture2D)
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

		if ( m_texFilePath != NULL )
		{
			LONG nWidth;
			LONG nHeight;
			LPVOID lpPixelData = nullptr;

			win32::LoadBMP(m_texFilePath, &nWidth, &nHeight, &lpPixelData);
			ASSERT(nWidth > 0 && nHeight > 0 && lpPixelData != nullptr);

			m_texture2D	= device.CreateTexture2D(nWidth, nHeight, 4, 4, 0, lpPixelData);

			delete[] lpPixelData;
		}
		m_vsData.tex		= m_texture2D;
		m_psData.tex		= m_texture2D;

		m_vsData.model		= M44Identity();
		m_psData.model		= M44Identity();
	}
	void		TextureEffect::Apply(RenderContext & ctx)
	{
		ctx.SetVertexShader(m_vertexShader);
		ctx.SetPixelShader(m_pixelShader);

		ctx.VSSetConstantBuffer(&m_vsData);
		ctx.PSSetConstantBuffer(&m_psData);
	}
	void		TextureEffect::CBSetModelTransform(const Matrix44 & modelTransform)
	{
		m_vsData.model = m_psData.model = modelTransform;
	}
	void		TextureEffect::CBSetViewTransform(const Matrix44 & viewTransform)
	{
		m_vsData.view = m_psData.view = viewTransform;
	}
	void		TextureEffect::CBSetProjTransform(const Matrix44 & projTransform)
	{
		m_vsData.proj = m_psData.proj = projTransform;
	}
	void		TextureEffect::VSImpl(void * pVSOut, const void * pVSIn, const void * pContext)
	{
		const VS_IN & in	= *static_cast< const VS_IN * >( pVSIn );
		const VS_DATA & ctx	= *static_cast< const VS_DATA * >( pContext );
		VS_OUT & out		= *static_cast< VS_OUT * >( pVSOut );

		out.posCam		= V3Transform(in.posWld, M44Multiply(ctx.model, ctx.view));
		out.posNDC		= V3Transform(out.posCam, ctx.proj);
		out.uv			= in.uv;
	}
	void		TextureEffect::PSImpl(void * pPSOut, const void * pPSIn, const void * pContext)
	{
		const PS_IN & in	= *static_cast< const PS_IN * >( pPSIn );
		const PS_DATA & ctx	= *static_cast< const PS_DATA * >( pContext );
		PS_OUT & out		= *static_cast< PS_OUT * >( pPSOut );

		ctx.tex.Sample(in.uv.x, in.uv.y, reinterpret_cast< float * >( &out.color ));
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

		m_vsData.model		= M44Identity();
		m_psData.model		= M44Identity();
	}
	void		BlinnPhongEffect::Apply(RenderContext & ctx)
	{
		ctx.SetVertexShader(m_vertexShader);
		ctx.SetPixelShader(m_pixelShader);

		ctx.VSSetConstantBuffer(&m_vsData);
		ctx.PSSetConstantBuffer(&m_psData);
	}
	void		BlinnPhongEffect::CBSetModelTransform(const Matrix44 & modelTransform)
	{
		m_vsData.model = m_psData.model = modelTransform;
	}
	void		BlinnPhongEffect::CBSetViewTransform(const Matrix44 & viewTransform)
	{
		m_vsData.view = m_psData.view = viewTransform;
	}
	void		BlinnPhongEffect::CBSetProjTransform(const Matrix44 & projTransform)
	{
		m_vsData.proj = m_psData.proj = projTransform;
	}
	void		BlinnPhongEffect::CBSetCameraPosition(const Vector3 & cameraPosWld)
	{
		m_vsData.cameraPosWld = m_psData.cameraPosWld = cameraPosWld;
	}
	void		BlinnPhongEffect::VSImpl(void * pVSOut, const void * pVSIn, const void * pContext)
	{
		const VS_IN & in	= *static_cast< const VS_IN * >( pVSIn );
		const VS_DATA & ctx	= *static_cast< const VS_DATA * >( pContext );
		VS_OUT & out		= *static_cast< VS_OUT * >( pVSOut );

		out.posCam		= V3Transform(in.posWld, M44Multiply(ctx.model, ctx.view));
		out.posNDC		= V3Transform(out.posCam, ctx.proj);
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
	void		BlinnPhongEffect::ComputeBlinnPhong(Vector3 * rgb, Vector3 posWld, Vector3 eyeWld, Vector3 normWld, const MaterialParams & material, const LightParams & light)
	{
		Vector3 lightDir;
		float lightDistance;
		{
			lightDir	= posWld - light.posWld;
			lightDistance	= lightDir.Length();
			lightDir.Normalize();
		}

		// Ambient Color = C_material
		Vector3 ambient;
		{
			ambient	= V3Multiply(material.rgbiAmbient.xyz, light.rgbiAmbient.xyz);
		}

		// Diffuse Color = max( cos(-L, norm), 0) * ElementwiseProduce(C_light, C_material)
		Vector3 diffuse;
		{
			float decayFactor = Max(0.0f, V3Dot(-lightDir, normWld));

			diffuse =
				V3Scale(
					V3Multiply(light.rgbiDiffuse.xyz, material.rgbiDiffuse.xyz),
					decayFactor);
		}

		// Specular Color = max( cos(L', to-eye), 0) * ElementwiseProduce(C_light, C_material)
		Vector3 specular;
		{
			Vector3 reflectLightDir = V3Normalize(lightDir - V3Scale(normWld, 2 * V3Dot(normWld, lightDir)));
			Vector3 toEyeDir = V3Normalize(eyeWld - posWld);

			float decayFactor = Max(0.0f, V3Dot(reflectLightDir, toEyeDir));
			decayFactor = decayFactor * decayFactor;
			decayFactor = decayFactor * decayFactor;
			decayFactor = decayFactor * decayFactor;

			specular =
				V3Scale(
					V3Multiply(light.rgbiSpecular.xyz, material.rgbiSpecular.xyz),
					decayFactor);
		}

		float atteFactor = 1.0f / V3Dot(light.attenuation, Vector3 { 1.0f, lightDistance, lightDistance * lightDistance });

		Vector3 color = WeightedAdd(ambient,
					    diffuse,
					    specular,
					    material.rgbiAmbient.w,
					    material.rgbiDiffuse.w * atteFactor,
					    material.rgbiSpecular.w * atteFactor);

		*rgb = Vector3
		{
			Bound(0.0f, color.x, 1.0f),
			Bound(0.0f, color.y, 1.0f),
			Bound(0.0f, color.z, 1.0f),
		};
	}
}