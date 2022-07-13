//#pragma once
//#include "Fission/Core/Configuration.h"
//#include "Fission/Core/Window.h"
//
//// Internal API
//
//namespace Fission
//{
//
//	class ConfigImpl
//	{
//
//	public:
//
//		static ConfigImpl & Get();
//
//		void GetWindowProperties( IFWindow::Properties * pFallback );
//
//		void SetWindowProperties( const IFWindow::Properties & _Properties );
//
//	private:
//
//		friend class Config;
//
//		struct OptWindowProperties
//		{
//			std::optional<IFWindow::Flag_t>	flags;
//			std::optional<v2i32>	position;
//			std::optional<size2>		size;
//			std::optional<int>				monitor_idx = 0;
//		};
//
//		std::unordered_map<IFWindow::SaveID, OptWindowProperties> m_WindowPropertyMap;
//	};
//
//}
//
