#include "GraphicsDirectX11.h"
#include "BindableDirectX11.h"
#include "WindowsWindow.h"
#include "Fission/Core/Console.h"

namespace Fission::Platform {

	GraphicsDirectX11::GraphicsDirectX11()
	{
		HRESULT hr = S_OK;

		static constexpr D3D_FEATURE_LEVEL FeatureLevelsWant[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
		D3D_FEATURE_LEVEL FeatureLevelGot;

		UINT uCreateFlags = 0u;

#ifdef FISSION_DEBUG
		uCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		hr = D3D11CreateDevice(
			nullptr,								/* Graphics Adapter */
			D3D_DRIVER_TYPE_HARDWARE,				/* Driver Type */
			nullptr,								/* Software */
			uCreateFlags,							/* Create Flags */
			FeatureLevelsWant,						/* Feature Levels requested */
			(UINT)std::size( FeatureLevelsWant ),	/* Number of Feature Levels requested */
			D3D11_SDK_VERSION,						/* SDK Version */
			&m_pDevice,								/* pp Device */
			&FeatureLevelGot,						/* Feature Level received */
			&m_pImmediateContext					/* pp Device Context */
		);

		if( FAILED( hr ) ) throw exception("DirectX Exception", _lazer_exception_msg.append("Failed to Create Device."));

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

		// Set Sampler | todo: move to texture?
		ID3D11SamplerState * ss;
		D3D11_SAMPLER_DESC sdesc = CD3D11_SAMPLER_DESC{};
		sdesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		sdesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		sdesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sdesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sdesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		m_pDevice->CreateSamplerState( &sdesc, &ss );
		m_pImmediateContext->PSSetSamplers( 0u, 1u, &ss );

		// Enable Clip Rects in the Raster State
		ID3D11RasterizerState * rs;
		D3D11_RASTERIZER_DESC rdesc = CD3D11_RASTERIZER_DESC{};
		rdesc.FillMode = D3D11_FILL_SOLID;
		rdesc.CullMode = D3D11_CULL_NONE;
		rdesc.ScissorEnable = false;
		rdesc.DepthClipEnable = false;
		m_pDevice->CreateRasterizerState( &rdesc, &rs );
		m_pImmediateContext->RSSetState( rs );

		//// Set Clip Rect to where we render
		//D3D11_RECT r;
		//r.left = r.top = 0;
		//r.bottom = m_Resolution.y;
		//r.right = m_Resolution.x;
		//m_pImmediateContext->RSSetScissorRects( 1, &r );

	}

	GraphicsDirectX11::~GraphicsDirectX11()
	{
		m_pImmediateContext.Reset();
		m_pDevice.Reset();
	}

	Graphics::API GraphicsDirectX11::GetAPI() { return API::DirectX11; }

	void GraphicsDirectX11::SetVSync( bool vsync )
	{
		m_SyncInterval = vsync ? 1 : 0;
	}

	bool GraphicsDirectX11::GetVSync() { return m_SyncInterval; }

	void GraphicsDirectX11::SetFrameBuffer( Resource::FrameBuffer * buffer )
	{
		auto dx11_buffer = static_cast<FrameBufferDX11 *>( buffer );

		ID3D11RenderTargetView * rtv = dx11_buffer->GetRenderTargetView();

		m_pImmediateContext->RSSetViewports( 1u, dx11_buffer->GetViewPort() );
		m_pImmediateContext->OMSetRenderTargets( 1u, &rtv, nullptr );
	}

	void GraphicsDirectX11::Draw( uint32_t vertexCount, uint32_t vertexOffset )
	{
		m_pImmediateContext->Draw( vertexCount, vertexOffset );
	}

	void GraphicsDirectX11::DrawIndexed( uint32_t indexCount, uint32_t indexOffset, uint32_t vertexOffset )
	{
		m_pImmediateContext->DrawIndexed( indexCount, indexOffset, vertexOffset );
	}

	scoped<Resource::FrameBuffer> GraphicsDirectX11::CreateFrameBuffer( const FrameBuffer::CreateInfo & info ) {
		return CreateScoped<FrameBufferDX11>( m_pDevice.Get(), m_pImmediateContext.Get(), info );
	}

	scoped<Resource::VertexBuffer> GraphicsDirectX11::CreateVertexBuffer( const VertexBuffer::CreateInfo & info ) {
		return CreateScoped<VertexBufferDX11>( m_pDevice.Get(), m_pImmediateContext.Get(), info );
	}

	scoped<Resource::IndexBuffer> GraphicsDirectX11::CreateIndexBuffer( const IndexBuffer::CreateInfo & info ) {
		return CreateScoped<IndexBufferDX11>( m_pDevice.Get(), m_pImmediateContext.Get(), info );
	}

	scoped<Resource::Shader> GraphicsDirectX11::CreateShader( const Shader::CreateInfo & info ) {
		return CreateScoped<ShaderDX11>( m_pDevice.Get(), m_pImmediateContext.Get(), info );
	}

	scoped<Resource::Texture2D> GraphicsDirectX11::CreateTexture2D( const Texture2D::CreateInfo & info ) {
		return CreateScoped<Texture2DDX11>( m_pDevice.Get(), m_pImmediateContext.Get(), info );
	}

	scoped<Resource::Blender> GraphicsDirectX11::CreateBlender( const Blender::CreateInfo & info ) {
		return CreateScoped<BlenderDX11>( m_pDevice.Get(), m_pImmediateContext.Get(), info );
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
