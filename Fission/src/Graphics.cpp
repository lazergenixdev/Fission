#include "Fission/Core/Graphics/Graphics.h"

#ifdef FISSION_PLATFORM_WINDOWS
#include "Platform/Windows/GraphicsDirectX11.h"
#include "Platform/Windows/GraphicsOpenGL.h"
#endif // FISSION_PLATFORM_WINDOWS

#include <gl/GL.h>

namespace Fission {

	namespace Private
	{
		enum {
			msaa_Off = 0
		};

		static Graphics::API Settings_API = Graphics::API::Default;
		static vsync_ Setting_VSync = vsync_On;
		static int Setting_MSAA = msaa_Off;

	}

	bool Graphics::IsSupported( API api )
	{
		switch( api )
		{
#ifdef FISSION_PLATFORM_WINDOWS
		case Graphics::API::DirectX11:
			return Platform::GraphicsDirectX11::CheckSupport();
	//	case Graphics::API::OpenGL:
	//		return Platform::GraphicsOpenGL::CheckSupport();
	//	case Graphics::API::DirectX12:
	//        return Platform::GraphicsDirectX12::CheckSupport();
	//	case Graphics::API::Vulkan:
	//        return Platform::GraphicsVulkan::CheckSupport();
#elif FISSION_PLATFORM_ANDRIOD
		case Graphics::API::OpenGL:
			return Platform::GraphicsOpenGL::CheckSupport();
		case Graphics::API::Vulkan:
	        return Platform::GraphicsVulkan::CheckSupport();
#endif // FISSION_PLATFORM_*
		default:
			return false;
		}
	}

	std::unique_ptr<Graphics> Graphics::Create( const State & _State )
	{
		Private::Settings_API = _State.api;

		switch( Private::Settings_API )
		{
	#ifdef FISSION_PLATFORM_WINDOWS
		case Graphics::API::Default:
		case Graphics::API::DirectX11:
			return std::make_unique<Platform::GraphicsDirectX11>();
		//case Graphics::API::OpenGL:
		//	return std::make_unique<Platform::GraphicsOpenGL>( _Ptr_Window, _Properties.resolution );
		//case Graphics::API::DirectX12:
		//	return std::make_unique<Platform::GraphicsDirectX12>( _Ptr_Window, _Properties.resolution );
		//case Graphics::API::Vulkan:
		//	return std::make_unique<Platform::GraphicsVulkan>( _Ptr_Window, _Properties.resolution );
	#elif FISSION_PLATFORM_ANDRIOD
		case Graphics::API::Default:
		case Graphics::API::OpenGL:
			return std::make_unique<Platform::GraphicsOpenGL>( _Ptr_Window, _Properties.resolution );
		case Graphics::API::Vulkan:
			return std::make_unique<Platform::GraphicsVulkan>( _Ptr_Window, _Properties.resolution );
	#endif // FISSION_PLATFORM_*
		default:
			return nullptr;
		}
	}

	void Graphics::SetVSync( vsync_ vsync_interval )
	{
		Private::Setting_VSync = vsync_interval;
	}

	vsync_ Graphics::GetVSync()
	{
		return Private::Setting_VSync;
	}

}