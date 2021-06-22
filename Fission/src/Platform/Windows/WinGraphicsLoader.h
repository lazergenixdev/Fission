#pragma once
#include "../GraphicsLoader.h"
#include "GraphicsDirectX11.h"

namespace Fission
{
	namespace Platform
	{
		class WindowsGraphicsLoader : public GraphicsLoader
		{
		public:
			WindowsGraphicsLoader() = default;

			virtual void Initialize() override;

			virtual void CreateGraphics( GraphicsState * pState, IFGraphics** ppGraphics ) override;

			virtual bool CheckSupport( IFGraphics::API api ) override;

			virtual void Destroy() override;

		private:
			DirectX11Module m_DirectX11Module;

			WindowsModule * pLoadedModule = nullptr;

			struct
			{
				bool DirectX11Support   = false;
				bool DirectX12Support   = false;
				bool VulkanSupport      = false;
				bool OpenGLSupport      = false;
			} Cache;
		};
	}

} // namespace Fission