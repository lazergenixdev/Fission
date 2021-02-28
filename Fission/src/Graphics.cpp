#include "LazerEngine/Core/Graphics/Graphics.h"

#ifdef LAZER_PLATFORM_WINDOWS
#include "Platform/Windows/GraphicsDirectX11.h"
//#include "Platform/Windows/GraphicsOpenGL.h"
#endif // LAZER_PLATFORM_WINDOWS


namespace lazer {

	bool Graphics::IsSupported( API api )
	{
		switch( api )
		{
#ifdef LAZER_PLATFORM_WINDOWS
		case Graphics::API::DirectX11:
			return Platform::GraphicsDirectX11::CheckSupport();
	//	case Graphics::API::OpenGL:
	//		return Platform::GraphicsOpenGL::CheckSupport();
	//	case Graphics::API::DirectX12:
	//        return Platform::GraphicsDirectX12::CheckSupport();
	//	case Graphics::API::Vulkan:
	//        return Platform::GraphicsVulkan::CheckSupport();
#elif LAZER_PLATFORM_ANDRIOD
		case Graphics::API::OpenGL:
			return Platform::GraphicsOpenGL::CheckSupport();
		case Graphics::API::Vulkan:
	        return Platform::GraphicsVulkan::CheckSupport();
#endif // LAZER_PLATFORM_
		default:
			return false;
		}
	}

	std::unique_ptr<Graphics> Graphics::Create( Window * pWindow, API api, vec2i resolution )
	{
		switch( api )
		{
	#ifdef LAZER_PLATFORM_WINDOWS
		case Graphics::API::Default:
		case Graphics::API::DirectX11:
			return std::make_unique<Platform::GraphicsDirectX11>( pWindow, resolution );
		//case Graphics::API::OpenGL:
		//	return std::make_unique<Platform::GraphicsOpenGL>( pWindow, resolution );
		//case Graphics::API::DirectX12:
		//	return std::make_unique<Platform::GraphicsDirectX12>( pWindow, resolution );
		//case Graphics::API::Vulkan:
		//	return std::make_unique<Platform::GraphicsVulkan>( pWindow, resolution );
	#elif LAZER_PLATFORM_ANDRIOD
		case Graphics::API::Default:
		case Graphics::API::OpenGL:
			return std::make_unique<Platform::GraphicsOpenGL>( pWindow, resolution );
		case Graphics::API::Vulkan:
			return std::make_unique<Platform::GraphicsVulkan>( pWindow, resolution );
	#endif // LAZER_PLATFORM_
		default:
			return nullptr;
		}
	}

}