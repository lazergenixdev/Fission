#include "Fission/Core/Graphics/Graphics.h"

#ifdef FISSION_PLATFORM_WINDOWS
#include "Platform/Windows/GraphicsDirectX11.h"
//#include "Platform/Windows/GraphicsOpenGL.h"
#endif // FISSION_PLATFORM_WINDOWS


namespace Fission {

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

	std::unique_ptr<Graphics> Graphics::Create( Window * pWindow, API api, vec2i resolution )
	{
		switch( api )
		{
	#ifdef FISSION_PLATFORM_WINDOWS
		case Graphics::API::Default:
		case Graphics::API::DirectX11:
			return std::make_unique<Platform::GraphicsDirectX11>( pWindow, resolution );
		//case Graphics::API::OpenGL:
		//	return std::make_unique<Platform::GraphicsOpenGL>( pWindow, resolution );
		//case Graphics::API::DirectX12:
		//	return std::make_unique<Platform::GraphicsDirectX12>( pWindow, resolution );
		//case Graphics::API::Vulkan:
		//	return std::make_unique<Platform::GraphicsVulkan>( pWindow, resolution );
	#elif FISSION_PLATFORM_ANDRIOD
		case Graphics::API::Default:
		case Graphics::API::OpenGL:
			return std::make_unique<Platform::GraphicsOpenGL>( pWindow, resolution );
		case Graphics::API::Vulkan:
			return std::make_unique<Platform::GraphicsVulkan>( pWindow, resolution );
	#endif // FISSION_PLATFORM_*
		default:
			return nullptr;
		}
	}

}