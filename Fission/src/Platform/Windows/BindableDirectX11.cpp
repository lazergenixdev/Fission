#include "BindableDirectX11.h"
#include <Fission/Core/Window.hh>
#include <Fission/Base/Exception.hpp>
#include <d3dcompiler.h>

namespace Fission::Platform {

	void VertexBufferDX11::Destroy() { delete this; }
	void IndexBufferDX11::Destroy() { delete this; }
	void ConstantBufferDX11::Destroy() { delete this; }
	void Texture2DDX11::Destroy() { delete this; }
	void ShaderDX11::Destroy() { delete this; }
	void SamplerDX11::Destroy() { delete this; }
	void BlenderDX11::Destroy() { delete this; }
	void SwapChainDX11::Destroy() { delete this; }

	VertexBufferDX11::VertexBufferDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const CreateInfo & info )
		: m_pContext( pContext ), m_Count( info.vtxCount ), m_Stride( info.pVertexLayout->GetStride() ), m_Type( info.type )
	{
		D3D11_BUFFER_DESC bd = CD3D11_BUFFER_DESC{};
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.ByteWidth = m_Count * m_Stride;
		bd.StructureByteStride = m_Stride;
		if( m_Type == Type::Static )
		{
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.CPUAccessFlags = 0u;
		}
		else
		{
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}

		if( info.pVertexData == nullptr )
		{
			pDevice->CreateBuffer( &bd, nullptr, &m_pBuffer );
		}
		else
		{
			D3D11_SUBRESOURCE_DATA srd = {};
			srd.pSysMem = info.pVertexData;

			pDevice->CreateBuffer( &bd, &srd, &m_pBuffer );
		}
	}

	void VertexBufferDX11::Bind()
	{
		uint32_t offset = 0;
		m_pContext->IASetVertexBuffers( 0u, 1u, m_pBuffer.GetAddressOf(), &m_Stride, &offset );
	}

	void VertexBufferDX11::Unbind()
	{
		uint32_t offset = 0;
		ID3D11Buffer * pNullBuffer = nullptr;
		m_pContext->IASetVertexBuffers( 0u, 1u, &pNullBuffer, &m_Stride, &offset );
	}

	void VertexBufferDX11::SetData( const void * pVertexData, uint32_t vtxCount )
	{
		FISSION_ASSERT( m_Type == Type::Dynamic, "Cannot override static buffer." );

		if( vtxCount == 0 ) return;

		D3D11_MAPPED_SUBRESOURCE msr;
		m_pContext->Map( m_pBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &msr );

		memcpy( msr.pData, pVertexData, vtxCount * m_Stride );

		m_pContext->Unmap( m_pBuffer.Get(), 0u );
	}

	uint32_t VertexBufferDX11::GetCount()
	{
		return m_Count;
	}


	IndexBufferDX11::IndexBufferDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const CreateInfo & info )
		: m_pContext( pContext ), m_Count( info.idxCount ), m_Type( info.type )
	{
		switch( info.size )
		{
		case Size::UInt16:	m_Stride = 2u; break;
		case Size::UInt32:	m_Stride = 4u; break;
		default:return;
		}

		D3D11_BUFFER_DESC bd = CD3D11_BUFFER_DESC{};
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.ByteWidth = m_Count * m_Stride;
		bd.StructureByteStride = m_Stride;
		if( m_Type == Type::Static )
		{
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.CPUAccessFlags = 0u;
		}
		else
		{
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}

		if( info.pIndexData == nullptr )
		{
			pDevice->CreateBuffer( &bd, nullptr, &m_pBuffer );
		}
		else
		{
			D3D11_SUBRESOURCE_DATA srd = {};
			srd.pSysMem = info.pIndexData;

			pDevice->CreateBuffer( &bd, &srd, &m_pBuffer );
		}
	}

	void IndexBufferDX11::Bind()
	{
		uint32_t offset = 0;
		switch( m_Stride )
		{
		case 2u:
			m_pContext->IASetIndexBuffer( m_pBuffer.Get(), DXGI_FORMAT_R16_UINT, offset );
			break;
		case 4u:
			m_pContext->IASetIndexBuffer( m_pBuffer.Get(), DXGI_FORMAT_R32_UINT, offset );
			break;
		default:break;
		}
	}

	void IndexBufferDX11::Unbind()
	{
		m_pContext->IASetIndexBuffer( nullptr, DXGI_FORMAT_R16_UINT, 0u );
	}

	void IndexBufferDX11::SetData( const void * pIndexData, uint32_t idxCount )
	{
		FISSION_ASSERT( m_Type == Type::Dynamic, "Cannot override static buffer." );

		if( idxCount == 0 ) return;

		D3D11_MAPPED_SUBRESOURCE msr;
		m_pContext->Map( m_pBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &msr );

		memcpy( msr.pData, pIndexData, idxCount * m_Stride );

		m_pContext->Unmap( m_pBuffer.Get(), 0u );
	}

	uint32_t IndexBufferDX11::GetCount()
	{
		return m_Count;
	}

	ShaderDX11::ShaderDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const CreateInfo & info )
		: m_pContext( pContext )
	{
		HRESULT hr = S_OK;
		com_ptr<ID3DBlob> pPSBlob;
		com_ptr<ID3DBlob> pVSBlob;

		auto create_input_layout = [&] () {
			uint32_t numElements = info.pVertexLayout->GetCount();
			D3D11_INPUT_ELEMENT_DESC * pInputElements = new D3D11_INPUT_ELEMENT_DESC[numElements];

			uint32_t offset = 0u;
			for( uint32_t i = 0; i < numElements; i++ )
			{
				auto type = info.pVertexLayout->GetType( i );
				pInputElements[i] =
				{
					info.pVertexLayout->GetName( i ),
					0,
					get_format( type ),
					0,
					offset,
					D3D11_INPUT_PER_VERTEX_DATA,
					0
				};
				offset += Resource::VertexLayoutTypes::GetStride( type );
			}

			hr = pDevice->CreateInputLayout( pInputElements, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pInputLayout );
		};

		UINT CompileFlags = 0u;
#if defined(FISSION_DEBUG)
		CompileFlags |= D3DCOMPILE_DEBUG;
#else
		CompileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

		com_ptr<ID3DBlob> pErrorBlob;
		if( FAILED( 
			hr = D3DCompile(
				info.sourceCode.data(),
				info.sourceCode.size(),
				nullptr,
				nullptr,
				nullptr,
				"vs_main",
				"vs_4_0",
				CompileFlags, 0u,
				&pVSBlob,
				&pErrorBlob 
			)
		) )
		{
			throw std::logic_error( (const char *)pErrorBlob->GetBufferPointer() );
		}


		if( FAILED(
			hr = D3DCompile(
				info.sourceCode.data(),
				info.sourceCode.size(),
				nullptr,
				nullptr,
				nullptr,
				"ps_main",
				"ps_4_0",
				CompileFlags, 0u,
				&pPSBlob,
				&pErrorBlob 
			)
		) )
		{
			throw std::logic_error( (const char *)pErrorBlob->GetBufferPointer() );
		}

		// Create Shaders
		// todo: check for errors!
		hr = pDevice->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pVertexShader );
		hr = pDevice->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pPixelShader );
		create_input_layout();

	}

	void ShaderDX11::Bind()
	{
		m_pContext->IASetInputLayout( m_pInputLayout.Get() );
		m_pContext->VSSetShader( m_pVertexShader.Get(), nullptr, 0u );
		m_pContext->PSSetShader( m_pPixelShader.Get(), nullptr, 0u );
	}

	void ShaderDX11::Unbind()
	{
	}

	DXGI_FORMAT ShaderDX11::get_format( Resource::VertexLayoutTypes::Type type )
	{
		switch( type )
		{
		case Resource::VertexLayoutTypes::Float:
			return DXGI_FORMAT_R32_FLOAT;
		case Resource::VertexLayoutTypes::Float2:
			return DXGI_FORMAT_R32G32_FLOAT;
		case Resource::VertexLayoutTypes::Float3:
			return DXGI_FORMAT_R32G32B32_FLOAT;
		case Resource::VertexLayoutTypes::Float4:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case Resource::VertexLayoutTypes::Int:
			return DXGI_FORMAT_R32_SINT;
		case Resource::VertexLayoutTypes::Int2:
		case Resource::VertexLayoutTypes::Int3:
		case Resource::VertexLayoutTypes::Int4:
		default:
			throw 0x45;
		}
		return DXGI_FORMAT_UNKNOWN;
	}

	//ConstantBufferDX11::ConstantBufferDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, uint32_t slot, uint32_t size )
	//	: m_pContext( pContext ), m_ByteSize(size), m_BindSlot(slot), m_pData(std::make_unique<char[]>( size ))
	//{
	//	D3D11_BUFFER_DESC bd = CD3D11_BUFFER_DESC{};
	//	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//	bd.Usage = D3D11_USAGE_DYNAMIC;
	//	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//	bd.ByteWidth = std::max( size, 64u );

	//	pDevice->CreateBuffer( &bd, nullptr, &m_pBuffer );
	//}

	ConstantBufferDX11::ConstantBufferDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const CreateInfo & info )
		: m_pContext(pContext)
	{
		D3D11_BUFFER_DESC bd = CD3D11_BUFFER_DESC{};
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		if( info.type == Type::Static )
		{
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.CPUAccessFlags = 0;
		}
		else
		{
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}
		bd.ByteWidth = std::max( info.max_size, 64u );

		pDevice->CreateBuffer( &bd, nullptr, &m_pBuffer );
	}

	void ConstantBufferDX11::Bind() {}

	void ConstantBufferDX11::Bind( Target target, int slot )
	{
		switch( target )
		{
		case Fission::Resource::IFConstantBuffer::Target::Vertex:
			m_pContext->VSSetConstantBuffers( slot, 1, m_pBuffer.GetAddressOf() );
		break;
		case Fission::Resource::IFConstantBuffer::Target::Pixel:
			m_pContext->PSSetConstantBuffers( slot, 1, m_pBuffer.GetAddressOf() );
		break;
		default:throw std::logic_error("this don't make no fucking sense.");
		}
	}

	void ConstantBufferDX11::SetData( const void * pData, uint32_t size )
	{
		HRESULT hr;
		D3D11_MAPPED_SUBRESOURCE msr = {};
		hr = m_pContext->Map( m_pBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &msr );
		memcpy( msr.pData, pData, size );
		m_pContext->Unmap( m_pBuffer.Get(), 0u );
	}

	SamplerDX11::SamplerDX11( ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const CreateInfo& info )
		: m_pContext(pContext)
	{

		D3D11_SAMPLER_DESC sdesc = CD3D11_SAMPLER_DESC{};
		switch( info.filter )
		{
		case Filter::Linear: sdesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; break;
		case Filter::Point:  sdesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;  break;
		default:break;
		}
		pDevice->CreateSamplerState( &sdesc, &m_pSamplerState );
	}

	void SamplerDX11::Bind() {}

	void SamplerDX11::Bind( Target target, int slot )
	{
		switch( target )
		{
		case Target::Vertex:
			m_pContext->VSSetSamplers( slot, 1, m_pSamplerState.GetAddressOf() );
			break;
		case Target::Pixel:
			m_pContext->PSSetSamplers( slot, 1, m_pSamplerState.GetAddressOf() );
			break;
		default:throw std::logic_error( "this don't make no fucking sense." );
		}
	}

	Texture2DDX11::Texture2DDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const CreateInfo & info )
		: m_pContext( pContext ), m_Slot( 0u ), m_Width( 0u ), m_Height( 0u )
	{
		D3D11_TEXTURE2D_DESC bd = CD3D11_TEXTURE2D_DESC( DXGI_FORMAT_R8G8B8A8_UNORM, 0, 0 );
		bd.MipLevels = 1u;
		if( info.type == Type::Dynamic )
		{
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}

		if( info.pSurface ) {
			m_Width = bd.Width = info.pSurface->width();
			m_Height = bd.Height = info.pSurface->height();
			D3D11_SUBRESOURCE_DATA srd = {};
			srd.pSysMem = info.pSurface->data();
			srd.SysMemPitch = info.pSurface->width() * sizeof rgba8;
			srd.SysMemSlicePitch = (UINT)info.pSurface->byte_size();
			pDevice->CreateTexture2D( &bd, &srd, &m_pTexture );
		}
		else if( info.filePath ) {
			throw std::logic_error( "Branch Not Implemented" );
		} else {
			throw std::logic_error( "Branch Not Implemented" );
		}

		pDevice->CreateShaderResourceView( m_pTexture.Get(), nullptr, &m_pShaderResourceView );
	}

	void Texture2DDX11::Bind( int slot )
	{
		m_pContext->PSSetShaderResources( (UINT)slot, 1u, m_pShaderResourceView.GetAddressOf() );
	}

	void Texture2DDX11::Bind()
	{
		m_pContext->PSSetShaderResources( 0u, 1u, m_pShaderResourceView.GetAddressOf() );
	}

	void Texture2DDX11::Unbind()
	{
		ID3D11ShaderResourceView * psrv = nullptr;
		m_pContext->PSSetShaderResources( 0u, 1u, &psrv );
	}

	uint32_t Texture2DDX11::GetWidth()
	{
		return m_Width;
	}

	uint32_t Texture2DDX11::GetHeight()
	{
		return m_Height;
	}


	BlenderDX11::BlenderDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const CreateInfo & info )
		: m_pContext( pContext )
	{
		D3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC{};
		auto rt = &blendDesc.RenderTarget[0];
		rt->RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		switch( info.blend )
		{
		case Blend::Normal:
			rt->BlendEnable = true;
			rt->SrcBlend = D3D11_BLEND_SRC_ALPHA;
			rt->DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			rt->BlendOp = D3D11_BLEND_OP_ADD;
			rt->SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
			rt->DestBlendAlpha = D3D11_BLEND_ZERO;
			rt->BlendOpAlpha = D3D11_BLEND_OP_ADD;
			break;
		case Blend::Source:
			rt->BlendEnable = FALSE;
			break;
		case Blend::Add:
			// Todo: Fix this, add more blend options
			rt->BlendEnable = true;
			rt->SrcBlend = D3D11_BLEND_SRC_ALPHA;
			rt->DestBlend = D3D11_BLEND_ONE;
			rt->BlendOp = D3D11_BLEND_OP_ADD;
			rt->SrcBlendAlpha = D3D11_BLEND_ONE;
			rt->DestBlendAlpha = D3D11_BLEND_ZERO;
			rt->BlendOpAlpha = D3D11_BLEND_OP_ADD;
			break;
		default:
			throw std::logic_error( "Blend Mode unsupported right now :(" );
			break;
		}

		pDevice->CreateBlendState( &blendDesc, &m_pBlendState );
	}

	void BlenderDX11::Bind()
	{
		const FLOAT blend_factor = 1.0f;
		m_pContext->OMSetBlendState( m_pBlendState.Get(), &blend_factor, UINT_MAX );
	}

	void BlenderDX11::Unbind()
	{
		const FLOAT blend_factor = 1.0f;
		m_pContext->OMSetBlendState( nullptr, &blend_factor, UINT_MAX );
	}

	SwapChainDX11::SwapChainDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const CreateInfo & info )
		: m_Resolution( info.size ), m_pContext(pContext)
	{
		HRESULT hr = S_OK;

		DXGI_SWAP_CHAIN_DESC dSwapChain = {};
		dSwapChain.BufferDesc.Width = m_Resolution.width();
		dSwapChain.BufferDesc.Height = m_Resolution.height();
		dSwapChain.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		dSwapChain.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		dSwapChain.OutputWindow = info.pWindow->native_handle();
		dSwapChain.BufferCount = 2u;
		dSwapChain.SampleDesc.Count = 8u;
		dSwapChain.SampleDesc.Quality = 0u;
		dSwapChain.Windowed = TRUE;
		dSwapChain.BufferDesc.RefreshRate.Numerator = 200;
		dSwapChain.BufferDesc.RefreshRate.Denominator = 1;
	//	dSwapChain.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		dSwapChain.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		IDXGIDevice * pDXGIDevice = nullptr;
		hr = pDevice->QueryInterface( IID_PPV_ARGS( &pDXGIDevice ) );

		IDXGIAdapter * pDXGIAdapter = nullptr;
		hr = pDXGIDevice->GetAdapter( &pDXGIAdapter );
		pDXGIDevice->Release();

		IDXGIFactory * pDXGIFactory = nullptr;
		hr = pDXGIAdapter->GetParent( IID_PPV_ARGS( &pDXGIFactory ) );
		pDXGIAdapter->Release();

		if( FAILED( hr ) ) FISSION_THROW( "DXGI Error",.append( "Failed to create DXGI Factory." ) );

		hr = pDXGIFactory->CreateSwapChain( pDevice, &dSwapChain, &m_pSwapChain );
		if( FAILED( hr ) ) FISSION_THROW( "DXGI Error",.append( "Failed to create Swapchain." ) );
		pDXGIFactory->Release();

		com_ptr<ID3D11Texture2D> pBackBuffer;
		hr = m_pSwapChain->GetBuffer( 0u, IID_PPV_ARGS( &pBackBuffer ) );
		hr = pDevice->CreateRenderTargetView( pBackBuffer.Get(), nullptr, &m_pRenderTargetView );

		D3D11_TEXTURE2D_DESC dBuffer;
		pBackBuffer->GetDesc( &dBuffer );

		m_ViewPort.Width = (FLOAT)dBuffer.Width;
		m_ViewPort.Height = (FLOAT)dBuffer.Height;
		m_ViewPort.MinDepth = 0.0f;
		m_ViewPort.MaxDepth = 1.0f;
		m_ViewPort.TopLeftX = 0.0f;
		m_ViewPort.TopLeftY = 0.0f;

		m_Resolution = { (int)dBuffer.Width, (int)dBuffer.Height };
	}

	void SwapChainDX11::Resize(size2 size)
	{
	//	std::scoped_lock lock( m_Mutex );
		HRESULT hr = S_OK;
		m_pRenderTargetView.Reset();

		hr = m_pSwapChain->ResizeBuffers( 2u, size.w, size.h, DXGI_FORMAT_R8G8B8A8_UNORM, 0u );
		if(FAILED(hr)) OutputDebugStringA("Failed to Resize Buffers\n");

		com_ptr<ID3D11Device> pDevice;
		m_pContext->GetDevice( &pDevice );

		com_ptr<ID3D11Texture2D> pBackBuffer;
		hr = m_pSwapChain->GetBuffer( 0u, IID_PPV_ARGS( &pBackBuffer ) );
		if(FAILED(hr)) OutputDebugStringA("Failed to Get Back Buffer\n");
		hr = pDevice->CreateRenderTargetView( pBackBuffer.Get(), nullptr, &m_pRenderTargetView );
		if(FAILED(hr)) OutputDebugStringA("Failed to Create RTV\n");

		D3D11_TEXTURE2D_DESC dBuffer;
		pBackBuffer->GetDesc( &dBuffer );

		m_ViewPort.Width = (FLOAT)dBuffer.Width;
		m_ViewPort.Height = (FLOAT)dBuffer.Height;
		m_ViewPort.MinDepth = 0.0f;
		m_ViewPort.MaxDepth = 1.0f;
		m_ViewPort.TopLeftX = 0.0f;
		m_ViewPort.TopLeftY = 0.0f;

		m_Resolution = { (int)dBuffer.Width, (int)dBuffer.Height };
	}

	size2 SwapChainDX11::GetSize()
	{
		return m_Resolution;
	}

	void SwapChainDX11::SetFullscreen( bool fullscreen, Monitor * pMonitor )
	{
		m_pSwapChain->SetFullscreenState( (BOOL)fullscreen, NULL );
	}

	void SwapChainDX11::Clear( color clear_color )
	{
	//	std::scoped_lock lock( m_Mutex );
		m_pContext->ClearRenderTargetView( m_pRenderTargetView.Get(), (FLOAT*)&clear_color );
	}

	void SwapChainDX11::Present( vsync_ vsync )
	{
	//	std::scoped_lock lock( m_Mutex );
		if( FAILED( m_pSwapChain->Present( (uint32_t)vsync, 0u ) ) )
			FISSION_THROW("DirectX Exception", 
				.append("Failed to Present Swapchain.")
				.append("Reason","Honestly don't know, I should have checked the HRESULT, my bad dude.") 
			);
	}

	void SwapChainDX11::Bind()
	{
	//	std::scoped_lock lock( m_Mutex );
		m_pContext->RSSetViewports( 1u, &m_ViewPort );
		m_pContext->OMSetRenderTargets( 1u, m_pRenderTargetView.GetAddressOf(), nullptr );
	}

}
