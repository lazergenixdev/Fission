#include "GraphicsDirectX11.h"
#include "BindableDirectX11.h"
#include "WindowsWindow.h"
#include "Fission/Core/Console.h"

// todo: this is not epic
#define ThrowFailedHR( func, garbo ) func

namespace Fission::Platform {

	GraphicsDirectX11::GraphicsDirectX11( Window * pWindow, vec2i resolution )
		: m_Resolution( resolution ), m_pParentWindow( pWindow )
	{
		assert( pWindow );

		HRESULT hr = S_OK;

		DXGI_SWAP_CHAIN_DESC dSwapChain = {};
		dSwapChain.BufferDesc.Width = resolution.x;
		dSwapChain.BufferDesc.Height = resolution.y;
		dSwapChain.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		dSwapChain.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		dSwapChain.OutputWindow = pWindow->native_handle();
		dSwapChain.BufferCount = 2u;
		dSwapChain.SampleDesc.Count = 8u;
		dSwapChain.SampleDesc.Quality = 0u;
		dSwapChain.Windowed = TRUE;
	//	dSwapChain.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // not needed

		static constexpr D3D_FEATURE_LEVEL FeatureLevelsWant[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
		D3D_FEATURE_LEVEL FeatureLevelGot;

		hr = D3D11CreateDeviceAndSwapChain(
			nullptr,								/* Graphics Adapter */
			D3D_DRIVER_TYPE_HARDWARE,				/* Driver Type */
			nullptr,								/* Software */
			0u,										/* Create Flags */
			FeatureLevelsWant,						/* Feature Levels requested */
			(UINT)std::size( FeatureLevelsWant ),	/* Number of Feature Levels requested */
			D3D11_SDK_VERSION,						/* SDK Version */
			&dSwapChain,							/* pp Swap Chain Descriptor */
			&m_pSwapChain,							/* pp Swap Chain */
			&m_pDevice,								/* pp Device */
			&FeatureLevelGot,						/* Feature Level received */
			&m_pImmediateContext					/* pp Device Context */
		);

		if( FAILED( hr ) ) throw exception("DirectX Exception", _lazer_exception_msg.append("Failed to Create Device and SwapChain."));

		m_NativeHandle.pDevice = m_pDevice.Get();
		m_NativeHandle.pDeviceContext = m_pImmediateContext.Get();

		switch( FeatureLevelGot ) 
		{
		case D3D_FEATURE_LEVEL_11_1:
			Console::WriteLine( Colors::Lime, L"Using DirectX 11.1" );
			break;
		case D3D_FEATURE_LEVEL_11_0:
			Console::WriteLine( Colors::Lime, L"Using DirectX 11.0" );
			break;
		default:
			throw std::logic_error("this don't make no fucking sense");
		}

		{
			com_ptr<ID3D11Resource> pBackBuffer;
			hr = m_pSwapChain->GetBuffer( 0u, IID_PPV_ARGS( &pBackBuffer ) );
			hr = m_pDevice->CreateRenderTargetView( pBackBuffer.Get(), nullptr, &m_pRenderTargetView );
		}

		{
			D3D11_VIEWPORT vp;
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.Width = (FLOAT)m_Resolution.x;
			vp.Height = (FLOAT)m_Resolution.y;
			m_pImmediateContext->RSSetViewports( 1u, &vp );
		}

		m_pImmediateContext->OMSetRenderTargets( 1u, m_pRenderTargetView.GetAddressOf(), nullptr );

		// this all needs to be moved !epic

		// disable depth testing
		ID3D11DepthStencilState * pDepthStencilState;
		D3D11_DEPTH_STENCIL_DESC desc;
		ZeroMemory( &desc, sizeof( desc ) );
		desc.DepthEnable = false;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		desc.StencilEnable = false;
		desc.FrontFace.StencilFailOp = desc.FrontFace.StencilDepthFailOp = desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		desc.BackFace = desc.FrontFace;

		m_pDevice->CreateDepthStencilState( &desc, &pDepthStencilState );
		m_pImmediateContext->OMSetDepthStencilState( pDepthStencilState, 0 );

		// todo: find where the best place to have this is
		m_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

		ID3D11SamplerState * ss;
		D3D11_SAMPLER_DESC sdesc = CD3D11_SAMPLER_DESC{};
		sdesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sdesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sdesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sdesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sdesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		m_pDevice->CreateSamplerState( &sdesc, &ss );
		m_pImmediateContext->PSSetSamplers( 0u, 1u, &ss );

		ID3D11RasterizerState * rs;
		D3D11_RASTERIZER_DESC rdesc = CD3D11_RASTERIZER_DESC{};
		rdesc.FillMode = D3D11_FILL_SOLID;
		rdesc.CullMode = D3D11_CULL_NONE;
		rdesc.ScissorEnable = true;
		rdesc.DepthClipEnable = false;
		m_pDevice->CreateRasterizerState( &rdesc, &rs );
		m_pImmediateContext->RSSetState( rs );

		D3D11_RECT r;
		r.left = r.top = 0;
		r.bottom = 720;
		r.right = 1280;
		m_pImmediateContext->RSSetScissorRects( 1, &r );

	}

	GraphicsDirectX11::~GraphicsDirectX11()
	{
		m_pRenderTargetView.Reset();
		m_pSwapChain.Reset();
		m_pImmediateContext.Reset();
		m_pDevice.Reset();
	}

	Graphics::API GraphicsDirectX11::GetAPI() { return API::DirectX11; }

	void GraphicsDirectX11::SetVSync( bool vsync )
	{
		m_SyncInterval = vsync ? 1 : 0;
	}

	bool GraphicsDirectX11::GetVSync() { return m_SyncInterval; }

	vec2i GraphicsDirectX11::GetResolution() { return m_Resolution; }

	vec2f GraphicsDirectX11::to_screen( vec2i mouse_pos )
	{
		const vec2f window_size = (vec2f)m_pParentWindow->GetSize();
		const vec2f pos_norm = (vec2f)mouse_pos / window_size;
		return pos_norm * (vec2f)m_Resolution;
	}

	// Todo: might not always want to clear the screen
	void GraphicsDirectX11::BeginFrame()
	{
		FLOAT col[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

		m_pImmediateContext->ClearRenderTargetView( m_pRenderTargetView.Get(), col );
	}

	void GraphicsDirectX11::EndFrame()
	{
		HRESULT hr;
		hr = m_pSwapChain->Present( m_SyncInterval, 0u );
		if( FAILED( hr ) ) throw exception("DirectX Exception", 
			_lazer_exception_msg.append("Failed to present").append("Reason","Honestly don't know, I should have checked the HRESULT, my bad dude.") );
	}

	void GraphicsDirectX11::Draw( uint32_t vertexCount, uint32_t vertexOffset )
	{
		m_pImmediateContext->Draw( vertexCount, vertexOffset );
	}

	void GraphicsDirectX11::DrawIndexed( uint32_t indexCount, uint32_t indexOffset, uint32_t vertexOffset )
	{
		m_pImmediateContext->DrawIndexed( indexCount, indexOffset, vertexOffset );
	}

	std::unique_ptr<Resource::VertexBuffer> GraphicsDirectX11::CreateVertexBuffer( const VertexBuffer::CreateInfo & info ) {
		return std::make_unique<VertexBufferDX11>( m_pDevice.Get(), m_pImmediateContext.Get(), info );
	}

	std::unique_ptr<Resource::IndexBuffer> GraphicsDirectX11::CreateIndexBuffer( const IndexBuffer::CreateInfo & info ) {
		return std::make_unique<IndexBufferDX11>( m_pDevice.Get(), m_pImmediateContext.Get(), info );
	}

	std::unique_ptr<Resource::Shader> GraphicsDirectX11::CreateShader( const Shader::CreateInfo & info ) {
		return std::make_unique<ShaderDX11>( m_pDevice.Get(), m_pImmediateContext.Get(), info );
	}

	std::unique_ptr<Resource::ConstantBuffer> GraphicsDirectX11::CreateConstantBuffer( const ConstantBuffer::CreateInfo & info ) {
		return std::make_unique<ConstantBufferDX11>( m_pDevice.Get(), m_pImmediateContext.Get(), info );
	}

	std::unique_ptr<Resource::Texture2D> GraphicsDirectX11::CreateTexture2D( const Texture2D::CreateInfo & info ) {
		return std::make_unique<Texture2DDX11>( m_pDevice.Get(), m_pImmediateContext.Get(), info );
	}

	std::unique_ptr<Resource::Blender> GraphicsDirectX11::CreateBlender( const Blender::CreateInfo & info ) {
		return std::make_unique<BlenderDX11>( m_pDevice.Get(), m_pImmediateContext.Get(), info );
	}

	bool GraphicsDirectX11::CheckSupport()
	{
		HRESULT hr = S_OK;

		com_ptr<ID3D11Device>			pDevice;
		com_ptr<ID3D11DeviceContext>	pImmediateContext;

		hr = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			0u,
			nullptr,
			0u,
			D3D11_SDK_VERSION,
			&pDevice,
			nullptr,
			&pImmediateContext
		);

		return SUCCEEDED( hr ); // Device ??  well ok then thanks for the free device
	}

	Graphics::native_handle_type GraphicsDirectX11::native_handle()
	{
		return &m_NativeHandle;
	}

}
