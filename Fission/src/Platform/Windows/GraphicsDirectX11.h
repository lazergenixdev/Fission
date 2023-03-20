#pragma once
#include <Fission/Core/Graphics.hh>
#include "BindableDirectX11.h"
#include "WindowsModule.h"
#include <d3d11.h>
#include <d3dcompiler.h>

namespace Fission::Platform
{
	struct DirectX11Module : public WindowsModule
	{
		constexpr DirectX11Module() noexcept = default;

		virtual bool Load() override;
		virtual void UnLoad() override;

		bool IsSupported();

		PFN_D3D11_CREATE_DEVICE CreateDevice = 0;
	};

	class GraphicsDirectX11 : public Graphics
	{
	public:
		GraphicsDirectX11( DirectX11Module * pModule );

		virtual API GetAPI() override;

		virtual void Draw( uint32_t vertexCount, uint32_t vertexOffset ) override;

		virtual void DrawIndexed( uint32_t indexCount, uint32_t indexOffset, uint32_t vertexOffset ) override;

		virtual void SetClipRect( rf32 rect ) override;
		
		virtual gfx::FrameBuffer*    CreateFrameBuffer   ( const gfx::FrameBuffer::CreateInfo & info    ) override;
		virtual gfx::VertexBuffer*   CreateVertexBuffer  ( const gfx::VertexBuffer::CreateInfo & info   ) override;
		virtual gfx::ConstantBuffer* CreateConstantBuffer( const gfx::ConstantBuffer::CreateInfo & info ) override;
		virtual gfx::IndexBuffer*    CreateIndexBuffer   ( const gfx::IndexBuffer::CreateInfo & info    ) override;
		virtual gfx::Sampler*        CreateSampler       ( const gfx::Sampler::CreateInfo & info        ) override;
		virtual gfx::Shader*         CreateShader        ( const gfx::Shader::CreateInfo & info         ) override;
		virtual gfx::Texture2D*      CreateTexture2D     ( const gfx::Texture2D::CreateInfo & info      ) override;
		virtual gfx::Blender*        CreateBlender       ( const gfx::Blender::CreateInfo & info        ) override;
		virtual gfx::SwapChain*      CreateSwapChain     ( const gfx::SwapChain::CreateInfo & info      ) override;
																  
		virtual native_handle_type native_handle() override;

		virtual void Destroy() override;
	private:
		com_ptr<ID3D11Device>               m_pDevice;
		com_ptr<ID3D11DeviceContext>        m_pImmediateContext;

		native_type_dx11                    m_NativeHandle;

		// Temperary -- TODO: Remove these.
		com_ptr<ID3D11DepthStencilState>    pDepthStencil;
		com_ptr<ID3D11SamplerState>         pSamplerState;
		com_ptr<ID3D11RasterizerState>      pRasterizerState;

	};

}
