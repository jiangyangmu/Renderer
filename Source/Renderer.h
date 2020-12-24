#pragma once

#include "Graphics.h"

namespace Rendering
{
	class HardcodedRenderer
	{
	public:
		using Context = Graphics::Pipeline::Context;

		static std::unique_ptr<HardcodedRenderer> Create();

		~HardcodedRenderer() = default;

		HardcodedRenderer(const HardcodedRenderer &) = delete;
		HardcodedRenderer(HardcodedRenderer && other) = default;
		HardcodedRenderer & operator = (const HardcodedRenderer & other) = delete;
		HardcodedRenderer & operator = (HardcodedRenderer && other) = default;

		// Operations

		void		ClearSurface();
		void		Update(float milliSeconds);
		void		Draw();

		// Properties

		Context &	GetContext()
		{
			return m_context;
		}

	private:
		HardcodedRenderer(Integer width, Integer height);

		Context		m_context;
	};
}