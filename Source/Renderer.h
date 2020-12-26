#pragma once

#include "Graphics.h"

namespace Rendering
{
	class HardcodedRenderer
	{
	public:
		using Context = Graphics::Pipeline::Context;

		HardcodedRenderer(Integer width, Integer height);
		~HardcodedRenderer() = default;

		HardcodedRenderer(const HardcodedRenderer &) = delete;
		HardcodedRenderer(HardcodedRenderer && other) = default;
		HardcodedRenderer & operator = (const HardcodedRenderer & other) = delete;
		HardcodedRenderer & operator = (HardcodedRenderer && other) = default;

		// Operations

		void		Resize(Integer width, Integer height);

		void		ClearSurface();
		void		Update(float milliSeconds);
		void		Draw();

		// Properties

		Context &	GetContext()
		{
			return m_context;
		}
		Camera &	GetCamera()
		{
			return m_camera;
		}

	private:

		Context		m_context;
		Camera		m_camera;
	};
}