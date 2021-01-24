#pragma once

#include "Resource.h"

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

	class RgbEffect : public Effect
	{
	public:
		RgbEffect();

		virtual void		Initialize(Device & device) override;
		virtual void		Apply(RenderContext & context) override;

		virtual void		CBSetViewTransform(const Matrix4x4 & viewTransform) override;
		virtual void		CBSetProjTransform(const Matrix4x4 & projTransform) override;

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

		static void VSImpl(void * pVSOut, const void * pVSIn, const void * pContext);
		static void PSImpl(void * pPSOut, const void * pPSIn, const void * pContext);

	private:
		VertexShader	m_vertexShader;
		PixelShader	m_pixelShader;
		VS_DATA		m_vsData;
		PS_DATA		m_psData;
	};

	class TextureEffect : public Effect
	{
	public:
		explicit TextureEffect(LPCWSTR lpTexFilePath);
		explicit TextureEffect(Texture2D texture2D);

		virtual void		Initialize(Device & device) override;
		virtual void		Apply(RenderContext & context) override;

		virtual void		CBSetViewTransform(const Matrix4x4 & viewTransform) override;
		virtual void		CBSetProjTransform(const Matrix4x4 & projTransform) override;

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

		static void VSImpl(void * pVSOut, const void * pVSIn, const void * pContext);
		static void PSImpl(void * pPSOut, const void * pPSIn, const void * pContext);

	private:
		VertexShader	m_vertexShader;
		PixelShader	m_pixelShader;
		VS_DATA		m_vsData;
		PS_DATA		m_psData;

		// Texture Data
		LPCWSTR		m_texFilePath;
		Texture2D	m_texture2D;
	};

	class BlinnPhongEffect : public Effect
	{
	public:
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

	public:
		BlinnPhongEffect(const MaterialParams & materialParams, const LightParams & lightParams);

		virtual void		Initialize(Device & device) override;
		virtual void		Apply(RenderContext & context) override;

		virtual void		CBSetViewTransform(const Matrix4x4 & viewTransform) override;
		virtual void		CBSetProjTransform(const Matrix4x4 & projTransform) override;
		void			CBSetCameraPosition(const Vec3 & cameraPosWld);

	private:
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

		static void VSImpl(void * pVSOut, const void * pVSIn, const void * pContext);
		static void PSImpl(void * pPSOut, const void * pPSIn, const void * pContext);
		static void ComputeBlinnPhong(Vec3 * rgb, Vec3 posWld, Vec3 eyeWld, Vec3 normWld, const MaterialParams & material, const LightParams & light);

	private:
		VertexShader	m_vertexShader;
		PixelShader	m_pixelShader;
		VS_DATA		m_vsData;
		PS_DATA		m_psData;
	};
}