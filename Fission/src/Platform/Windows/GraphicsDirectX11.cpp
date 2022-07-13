#include "GraphicsDirectX11.h"
#include "WindowsWindow.h"
#include <Fission/Core/Console.hh>
#include <Fission/Base/Exception.hpp>

namespace Fission::Platform {

	bool DirectX11Module::Load()
	{
		if( LoadLibrary( "d3d11" ) )
		{
			bool bSuccess = true;

			bSuccess &= LoadFunction( &CreateDevice, "D3D11CreateDevice" );

			return bSuccess;
		}
		return false;
	}

	void DirectX11Module::UnLoad()
	{
		CreateDevice = NULL;
		WindowsModule::Free();
	}

	bool DirectX11Module::IsSupported()
	{
		// Check for DirectX11 Support.
		HRESULT hr = S_OK;
		com_ptr<ID3D11Device>			pDevice;
		com_ptr<ID3D11DeviceContext>	pImmediateContext;

		hr = CreateDevice(
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

//////////////////////////////////////////////////// GraphicsDirectX11 ////////////////////////////////////////////////////

	GraphicsDirectX11::GraphicsDirectX11( DirectX11Module * pModule )
	{
		HRESULT hr = S_OK;

		static constexpr D3D_FEATURE_LEVEL FeatureLevelsWant[] = {
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0 
		};
		D3D_FEATURE_LEVEL FeatureLevelGot;

		UINT uCreateFlags = 0u;

#ifdef FISSION_DEBUG
		uCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		hr = pModule->CreateDevice(
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

		// TODO: use DXErr for a little more information
		if( FAILED( hr ) ) { FISSION_THROW( "DirectX Exception", .append( "Failed to Create D3D11 Device." ) ); }

		// Set the native handle
		m_NativeHandle.pDevice        = m_pDevice.Get();
		m_NativeHandle.pDeviceContext = m_pImmediateContext.Get();

		// Print what version of D3D we got
		switch( FeatureLevelGot ) 
		{
		case D3D_FEATURE_LEVEL_11_1: Console::WriteLine( "Using DirectX 11.1"/Colors::Lime ); break;
		case D3D_FEATURE_LEVEL_11_0: Console::WriteLine( "Using DirectX 11.0"/Colors::Lime ); break;
		default:throw std::logic_error("this don't make no fucking sense");
		}

		// TODO: remove all this junk
		m_pImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
		// disable depth testing
		D3D11_DEPTH_STENCIL_DESC desc;
		ZeroMemory( &desc, sizeof( desc ) );
		desc.DepthEnable = false;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		desc.StencilEnable = false;
		desc.FrontFace.StencilFailOp = desc.FrontFace.StencilDepthFailOp = desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		desc.BackFace = desc.FrontFace;
		m_pDevice->CreateDepthStencilState( &desc, &pDepthStencil );
		m_pImmediateContext->OMSetDepthStencilState( pDepthStencil.Get(), 0 );
		// Set Sampler
		D3D11_SAMPLER_DESC sdesc = CD3D11_SAMPLER_DESC{};
		sdesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		sdesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		sdesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sdesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sdesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		m_pDevice->CreateSamplerState( &sdesc, &pSamplerState );
		m_pImmediateContext->PSSetSamplers( 0u, 1u, pSamplerState.GetAddressOf() );
		// Set Rasterizer
		D3D11_RASTERIZER_DESC rdesc = CD3D11_RASTERIZER_DESC{};
		rdesc.FillMode = D3D11_FILL_SOLID;
		rdesc.CullMode = D3D11_CULL_NONE;
		rdesc.ScissorEnable = false;
		rdesc.DepthClipEnable = false;
		m_pDevice->CreateRasterizerState( &rdesc, &pRasterizerState );
		m_pImmediateContext->RSSetState( pRasterizerState.Get() );
	}

	IFGraphics::API GraphicsDirectX11::GetAPI() { return API::DirectX11; }

	void GraphicsDirectX11::Draw( uint32_t vertexCount, uint32_t vertexOffset )
	{
		m_pImmediateContext->Draw( vertexCount, vertexOffset );
	}

	void GraphicsDirectX11::DrawIndexed( uint32_t indexCount, uint32_t indexOffset, uint32_t vertexOffset )
	{
		m_pImmediateContext->DrawIndexed( indexCount, indexOffset, vertexOffset );
	}

	void GraphicsDirectX11::SetClipRect( base::rectf rect )
	{
		const D3D11_RECT clip_rect = base::rect<LONG>(rect).as<D3D11_RECT>();
		m_pImmediateContext->RSSetScissorRects( 1, &clip_rect );
	}

	Resource::IFFrameBuffer* GraphicsDirectX11::CreateFrameBuffer( const FrameBuffer::CreateInfo & info ) {
		FISSION_THROW_NOT_IMPLEMENTED();
		return nullptr;
	}

	Resource::IFVertexBuffer* GraphicsDirectX11::CreateVertexBuffer( const VertexBuffer::CreateInfo & info ) {
		return new VertexBufferDX11( m_pDevice.Get(), m_pImmediateContext.Get(), info );
	}

	Resource::IFIndexBuffer* GraphicsDirectX11::CreateIndexBuffer( const IndexBuffer::CreateInfo & info ) {
		return new IndexBufferDX11( m_pDevice.Get(), m_pImmediateContext.Get(), info );
	}

	Resource::IFConstantBuffer* GraphicsDirectX11::CreateConstantBuffer( const ConstantBuffer::CreateInfo & info ) {
		return new ConstantBufferDX11( m_pDevice.Get(), m_pImmediateContext.Get(), info );
	}

	Resource::IFShader* GraphicsDirectX11::CreateShader( const Shader::CreateInfo & info ) {
		return new ShaderDX11( m_pDevice.Get(), m_pImmediateContext.Get(), info );
	}

	Resource::IFTexture2D* GraphicsDirectX11::CreateTexture2D( const Texture2D::CreateInfo & info ) {
		return new Texture2DDX11( m_pDevice.Get(), m_pImmediateContext.Get(), info );
	}

	Resource::IFBlender* GraphicsDirectX11::CreateBlender( const Blender::CreateInfo & info ) {
		return new BlenderDX11( m_pDevice.Get(), m_pImmediateContext.Get(), info );
	}

	Resource::IFSwapChain* GraphicsDirectX11::CreateSwapChain( const SwapChain::CreateInfo & info ) {
		return new SwapChainDX11( m_pDevice.Get(), m_pImmediateContext.Get(), info );
	}

	IFGraphics::native_handle_type GraphicsDirectX11::native_handle()
	{
		return &m_NativeHandle;
	}

	void GraphicsDirectX11::Destroy() { delete this; }

}
