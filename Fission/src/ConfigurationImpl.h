#pragma once
#include "Fission/Core/Configuration.h"
#include "Fission/Core/Window.h"

// Internal API

namespace Fission
{

	class ConfigImpl
	{

	public:

		static ConfigImpl & Get();

		void GetWindowProperties( Window::Properties * pFallback );

		void SetWindowProperties( const Window::Properties & _Properties );

	private:

		friend class Config;

		struct OptWindowProperties
		{
			std::optional<Window::Flag_t>	flags;
			std::optional<base::vector2i>	position;
			std::optional<base::size>		size;
			std::optional<int>				monitor_idx = 0;
		};

		std::unordered_map<Window::SaveID, OptWindowProperties> m_WindowPropertyMap;
	};

}

