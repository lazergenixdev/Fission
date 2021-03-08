#pragma once
#include "Fission/Core/Graphics/Graphics.h"
#include <d3d11.h>

namespace Fission::Platform
{

	class GraphicsDirectX11 : public Graphics
	{
	public:
		GraphicsDirectX11( Window * pWindow, vec2i resolution );

		virtual ~GraphicsDirectX11() override;

		virtual API GetAPI() override;

		virtual void SetVSync( bool vsync ) override;

		virtual bool GetVSync() override;

		virtual vec2i GetResolution() override;

		virtual vec2f to_screen( vec2i mouse_pos ) override;

		virtual void BeginFrame() override;

		virtual void EndFrame() override;

		virtual void Draw( uint32_t vertexCount, uint32_t vertexOffset ) override;

		virtual void DrawIndexed( uint32_t indexCount, uint32_t indexOffset, uint32_t vertexOffset ) override;

		virtual std::unique_ptr<VertexBuffer>	CreateVertexBuffer	( const VertexBuffer::CreateInfo & info		) override;
		virtual std::unique_ptr<IndexBuffer>	CreateIndexBuffer	( const IndexBuffer::CreateInfo & info		) override;
		virtual std::unique_ptr<Shader>			CreateShader		( const Shader::CreateInfo & info			) override;
		virtual std::unique_ptr<Texture2D>		CreateTexture2D		( const Texture2D::CreateInfo & info		) override;
		virtual std::unique_ptr<Blender>		CreateBlender		( const Blender::CreateInfo & info			) override;

		virtual native_handle_type native_handle() override;

	public:

		static bool CheckSupport();

	private:

		vec2i								m_Resolution;
		uint32_t							m_SyncInterval = 1u;

		bool								m_Fullscreen = false;

		com_ptr<ID3D11Device>				m_pDevice;
		com_ptr<ID3D11DeviceContext>		m_pImmediateContext;
		com_ptr<IDXGISwapChain>				m_pSwapChain;

		com_ptr<ID3D11RenderTargetView>		m_pRenderTargetView;

		Window *							m_pParentWindow;

		native_type_dx11					m_NativeHandle;
	};

}
