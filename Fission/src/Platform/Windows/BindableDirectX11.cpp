#include "BindableDirectX11.h"
#include "Fission/Core/Window.h"
#include "Fission/Base/Exception.h"
#include <d3dcompiler.h>

namespace Fission::Platform {

	// glorious meta-programming
#define DEFINE_HLSL_TYPE(N,T,HT,HC,C,R) \
	struct N { \
        using type = T; \
        static constexpr D3D_SHADER_VARIABLE_TYPE hlsltype = HT; \
        static constexpr D3D_SHADER_VARIABLE_CLASS hlslclass = HC; \
        static constexpr uint32_t columns = C; \
        static constexpr uint32_t rows = R; \
    }

	DEFINE_HLSL_TYPE( HLSLFloat, float, D3D_SVT_FLOAT, D3D_SVC_SCALAR, 1, 1 );
	DEFINE_HLSL_TYPE( HLSLFloat2, base::vector2f, D3D_SVT_FLOAT, D3D_SVC_VECTOR, 2, 1 );
	DEFINE_HLSL_TYPE( HLSLFloat3, base::vector3f, D3D_SVT_FLOAT, D3D_SVC_VECTOR, 3, 1 );
	DEFINE_HLSL_TYPE( HLSLFloat4, base::vector4f, D3D_SVT_FLOAT, D3D_SVC_VECTOR, 4, 1 );

	DEFINE_HLSL_TYPE( HLSLInt, int, D3D_SVT_INT, D3D_SVC_SCALAR, 1, 1 );
	DEFINE_HLSL_TYPE( HLSLInt2, base::vector2i, D3D_SVT_INT, D3D_SVC_VECTOR, 2, 1 );
	DEFINE_HLSL_TYPE( HLSLInt3, base::vector3i, D3D_SVT_INT, D3D_SVC_VECTOR, 3, 1 );
	DEFINE_HLSL_TYPE( HLSLInt4, base::vector4i, D3D_SVT_INT, D3D_SVC_VECTOR, 4, 1 );

	DEFINE_HLSL_TYPE( HLSLFloat2x2, base::matrix2x2f, D3D_SVT_FLOAT, D3D_SVC_MATRIX_COLUMNS, 2, 2 );
	DEFINE_HLSL_TYPE( HLSLFloat3x2, base::matrix2x3f, D3D_SVT_FLOAT, D3D_SVC_MATRIX_COLUMNS, 3, 2 );
	DEFINE_HLSL_TYPE( HLSLFloat3x3, base::matrix3x3f, D3D_SVT_FLOAT, D3D_SVC_MATRIX_COLUMNS, 3, 3 );
	DEFINE_HLSL_TYPE( HLSLMatrix, base::matrix4x4f, D3D_SVT_FLOAT, D3D_SVC_MATRIX_COLUMNS, 4, 4 );

#undef DEFINE_HLSL_TYPE


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
		FISSION_ASSERT( m_Type == Type::Dynamic );

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
		FISSION_ASSERT( m_Type == Type::Dynamic );

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

	template <typename T>
	static HRESULT ReflectToConstantBuffers(
		std::vector<T> * pBuffers,
		ID3D11Device * pDevice,
		ID3D11DeviceContext * pContext,
		ID3DBlob * pBlob )
	{
		HRESULT hr;

		com_ptr<ID3D11ShaderReflection> pShaderReflection;
		// this function SHOULD never fail, since we only pass shaders we create
		hr = D3DReflect( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), __uuidof( ID3D11ShaderReflection ), &pShaderReflection );

		D3D11_SHADER_DESC shader_desc;
		pShaderReflection->GetDesc( &shader_desc );

		// Search through all buffer to find any constant buffers
		for( int i = 0; i < (int)shader_desc.BoundResources; ++i )
		{
			D3D11_SHADER_INPUT_BIND_DESC bind_desc;
			pShaderReflection->GetResourceBindingDesc( i, &bind_desc );
			if( bind_desc.Type != D3D_SIT_CBUFFER ) continue; // continue to next bound resource if not constant buffer

			// Get constant buffer description
			D3D11_SHADER_BUFFER_DESC desc;
			auto pConstantBuffer = pShaderReflection->GetConstantBufferByName( bind_desc.Name );
			pConstantBuffer->GetDesc( &desc );

			// Add constant buffer to our own list of buffers
			pBuffers->emplace_back( pDevice, pContext, bind_desc.BindPoint, desc.Size );
			auto & cb = pBuffers->back();

			// search through all variables held in the buffer to add to our own buffer
			for( int i = 0; i < (int)desc.Variables; ++i )
			{
				D3D11_SHADER_VARIABLE_DESC vdesc;
				D3D11_SHADER_TYPE_DESC tdesc;
				auto * pVar = pConstantBuffer->GetVariableByIndex( i );
				auto * pType = pVar->GetType();
				pVar->GetDesc( &vdesc );
				pType->GetDesc( &tdesc );

				// Add variable to our constant buffer
				ConstantBufferDX11::Variable var;
				var.m_class = tdesc.Class;
				var.m_type = tdesc.Type;
				var.m_columns = tdesc.Columns;
				var.m_rows = tdesc.Rows;
				var.m_offset = vdesc.StartOffset;
				cb.AddVariable( vdesc.Name, var );
			}
		}

		return S_OK;
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
				info.source_code.data(),
				info.source_code.size(),
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
				info.source_code.data(),
				info.source_code.size(),
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

		// Create Constant Buffers
		ReflectToConstantBuffers( &m_VertexCBuffers, pDevice, pContext, pVSBlob.Get() );
		ReflectToConstantBuffers( &m_PixelCBuffers, pDevice, pContext, pPSBlob.Get() );

	}

	void ShaderDX11::Bind()
	{
		for( auto && b : m_PixelCBuffers ) b.Bind();
		for( auto && b : m_VertexCBuffers ) b.Bind();

		m_pContext->IASetInputLayout( m_pInputLayout.Get() );
		m_pContext->VSSetShader( m_pVertexShader.Get(), nullptr, 0u );
		m_pContext->PSSetShader( m_pPixelShader.Get(), nullptr, 0u );
	}

	void ShaderDX11::Unbind()
	{
	}
	
	bool ShaderDX11::SetVariable( const char * name, float val ) { return _Set<HLSLFloat>( name, val ); }
	bool ShaderDX11::SetVariable( const char * name, base::vector2f val ) { return _Set<HLSLFloat2>( name, val ); }
	bool ShaderDX11::SetVariable( const char * name, base::vector3f val ) { return _Set<HLSLFloat3>( name, val ); }
	bool ShaderDX11::SetVariable( const char * name, base::vector4f val ) { return _Set<HLSLFloat4>( name, val ); }

	bool ShaderDX11::SetVariable( const char * name, int val ) { return _Set<HLSLInt>( name, val ); }
	bool ShaderDX11::SetVariable( const char * name, base::vector2i val ) { return _Set<HLSLInt2>( name, val ); }
	bool ShaderDX11::SetVariable( const char * name, base::vector3i val ) { return _Set<HLSLInt3>( name, val ); }
	bool ShaderDX11::SetVariable( const char * name, base::vector4i val ) { return _Set<HLSLInt4>( name, val ); }

	bool ShaderDX11::SetVariable( const char * name, base::matrix2x2f val ) { return _Set<HLSLFloat2x2>( name, val ); }
	bool ShaderDX11::SetVariable( const char * name, base::matrix2x3f val ) { return _Set<HLSLFloat3x2>( name, val ); }
	bool ShaderDX11::SetVariable( const char * name, base::matrix3x3f val ) { return _Set<HLSLFloat3x3>( name, val ); }
	bool ShaderDX11::SetVariable( const char * name, base::matrix4x4f val ) { return _Set<HLSLMatrix>( name, val ); }

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

	ConstantBufferDX11::ConstantBufferDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, uint32_t slot, uint32_t size )
		: m_pContext( pContext ), m_ByteSize(size), m_BindSlot(slot), m_pData(std::make_unique<char[]>( size ))
	{
		D3D11_BUFFER_DESC bd = CD3D11_BUFFER_DESC{};
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bd.ByteWidth = std::max( size, 64u );

		pDevice->CreateBuffer( &bd, nullptr, &m_pBuffer );
	}

	PixelConstantBufferDX11::PixelConstantBufferDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, uint32_t slot, uint32_t size )
		: ConstantBufferDX11( pDevice, pContext, slot, size )
	{}

	VertexConstantBufferDX11::VertexConstantBufferDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, uint32_t slot, uint32_t size )
		: ConstantBufferDX11( pDevice, pContext, slot, size )
	{}

	void PixelConstantBufferDX11::Bind()
	{
		ConstantBufferDX11::Bind();
		m_pContext->PSSetConstantBuffers( m_BindSlot, 1u, m_pBuffer.GetAddressOf() );
	}

	void VertexConstantBufferDX11::Bind()
	{
		ConstantBufferDX11::Bind();
		m_pContext->VSSetConstantBuffers( m_BindSlot, 1u, m_pBuffer.GetAddressOf() );
	}

	void ConstantBufferDX11::Bind()
	{
		if( !m_bDirty ) return;
		D3D11_MAPPED_SUBRESOURCE msr = {};
		m_pContext->Map( m_pBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &msr );
		memcpy( msr.pData, m_pData.get(), m_ByteSize );
		m_pContext->Unmap( m_pBuffer.Get(), 0u );
		m_bDirty = false;
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
			srd.SysMemPitch = info.pSurface->width() * sizeof rgba_color8;
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
		dSwapChain.SampleDesc.Count = 4u;
		dSwapChain.SampleDesc.Quality = 0u;
		dSwapChain.Windowed = TRUE;
		dSwapChain.BufferDesc.RefreshRate.Numerator = 200;
		dSwapChain.BufferDesc.RefreshRate.Denominator = 1;
		dSwapChain.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		com_ptr<IDXGIFactory> pDXGIFactory;
		hr = CreateDXGIFactory( __uuidof( IDXGIFactory ), &pDXGIFactory );
		if( FAILED( hr ) ) FISSION_THROW( "DXGI Error",.append( "Failed to create DXGI Factory." ) );

		hr = pDXGIFactory->CreateSwapChain( pDevice, &dSwapChain, &m_pSwapChain );
		if( FAILED( hr ) ) FISSION_THROW( "DXGI Error",.append( "Failed to create Swapchain." ) );

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

	base::size SwapChainDX11::GetSize()
	{
		return m_Resolution;
	}

	void SwapChainDX11::SetFullscreen( bool fullscreen, Monitor * pMonitor )
	{
		m_pSwapChain->SetFullscreenState( (BOOL)fullscreen, NULL );
	}

	void SwapChainDX11::Clear( color clear_color )
	{
		m_pContext->ClearRenderTargetView( m_pRenderTargetView.Get(), (FLOAT*)&clear_color );
	}

	void SwapChainDX11::Present( vsync_ vsync )
	{
		if( FAILED( m_pSwapChain->Present( (uint32_t)vsync, 0u ) ) )
			FISSION_THROW("DirectX Exception", 
				.append("Failed to Present Swapchain.")
				.append("Reason","Honestly don't know, I should have checked the HRESULT, my bad dude.") 
			);
	}

	void SwapChainDX11::Bind()
	{
		m_pContext->RSSetViewports( 1u, &m_ViewPort );
		m_pContext->OMSetRenderTargets( 1u, m_pRenderTargetView.GetAddressOf(), nullptr );
	}

}
