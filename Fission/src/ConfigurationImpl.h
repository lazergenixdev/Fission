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
			std::optional<Window::Flags>	flags;
			std::optional<vec2i>			position;
			std::optional<vec2i>			size;
			std::optional<int>				monitor_idx = 0;
		};

		std::unordered_map<Window::SaveID, OptWindowProperties> m_WindowPropertyMap;
	};

}

