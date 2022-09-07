#include "WinGraphicsLoader.h"
#include "Fission/Base/Exception.hpp"
#include "../NoopGraphics.h"

namespace Fission::Platform
{
	void WindowsGraphicsLoader::Initialize()
	{
		if( m_DirectX11Module.Load() )
		{
			Cache.DirectX11Support = m_DirectX11Module.IsSupported();
			m_DirectX11Module.UnLoad();
		}
		Cache.OpenGLSupport = true; // It's always true, lets be honest.

		auto vkdll = Module( LoadLibraryExA( "vulkan-1.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32 ) );
		Cache.VulkanSupport = (bool)vkdll; // TODO: Fix when add native vulkan support.
	}

	bool WindowsGraphicsLoader::CheckSupport( IFGraphics::API api )
	{
		switch( api )
		{
		case Fission::IFGraphics::API::DirectX11:return  Cache.DirectX11Support;
		case Fission::IFGraphics::API::DirectX12:return  Cache.DirectX12Support;
		case Fission::IFGraphics::API::Vulkan:return     Cache.VulkanSupport;
		case Fission::IFGraphics::API::OpenGL:return     Cache.OpenGLSupport;
		case Fission::IFGraphics::API::Noop:return       true; // Noop is always supported for debugging.
		default:return false; // API::Default is NOT a Graphics API, only a helper for creating graphics.
		}
	}

	void WindowsGraphicsLoader::CreateGraphics( GraphicsState * pState, IFGraphics ** ppGraphics )
	{
		*ppGraphics = NULL;
		switch( pState->api )
		{
		case IFGraphics::API::Default:
		case IFGraphics::API::DirectX11:
		{
			if( pLoadedModule != &m_DirectX11Module )
			{
				if( pLoadedModule ) pLoadedModule->UnLoad();
				if( !m_DirectX11Module.Load() )
					FISSION_THROW( "Graphics Loader Exception",.append("Could not load d3d11.dll.") );
				pLoadedModule = &m_DirectX11Module;
			}

			*ppGraphics = new GraphicsDirectX11( &m_DirectX11Module );
		}
		break;
		
		case IFGraphics::API::Noop:
			*ppGraphics = new Noop::Graphics();
		break;
		
		case IFGraphics::API::OpenGL:
			*ppGraphics = new GraphicsOpenGL();
		break;

		default:break;
		}
	}

	void WindowsGraphicsLoader::Destroy() { delete this; }

}
