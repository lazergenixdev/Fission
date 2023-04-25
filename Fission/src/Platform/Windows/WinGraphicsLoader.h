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

			virtual void CreateGraphics( GraphicsState * pState, Graphics** ppGraphics ) override;

			virtual bool CheckSupport( Graphics::API api ) override;

			virtual void Destroy() override;

		private:
			DirectX11Module m_DirectX11Module;

			WindowsModule * pLoadedModule = nullptr;

			struct
			{
				bool DirectX11Support   = false;
				bool DirectX12Support   = false;
				bool VulkanSupport      = false;
			} Cache;
		};
	}

} // namespace Fission