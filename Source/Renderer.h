#pragma once

#include "Resource.h"

namespace Graphics
{
	// --------------------------------------------------------------------------
	// Rendering Interface
	// --------------------------------------------------------------------------

	class Renderable
	{
	public:
		virtual			~Renderable() = default;
		virtual void		Initialize(RenderContext & renderContext, VertexBuffer & vertexBuffer) {}
		virtual void		Update(double ms) {}
		virtual void		Draw() {}
	};

	class IRenderer
	{
	public:
		virtual			~IRenderer() = default;
		virtual void		Present() = 0;
		virtual void		Clear() = 0;
		virtual void		Update(double milliSeconds) = 0;
		virtual void		Draw() = 0;
	};

	// --------------------------------------------------------------------------
	// Rendering Implementation
	// --------------------------------------------------------------------------

	class ROTriangle : public Renderable
	{
	public:
		ROTriangle(Vec3 pos0, Vec3 rgb0, Vec3 pos1, Vec3 rgb1, Vec3 pos2, Vec3 rgb2);
		virtual			~ROTriangle();

		virtual void		Initialize(RenderContext & renderContext, VertexBuffer & vertexBuffer) override;
		virtual void		Draw() override;

	private:
		struct Vertex
		{
			Vec3 pos;
			Vec3 rgb;
		};
		Vertex			m_vertex[ 3 ];
		VertexRange		m_vertexRange;
		VertexBuffer *		m_refVertexBuffer;
		RenderContext *		m_refContext;
	};

}