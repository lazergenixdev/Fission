#include "BindableDirectX11.h"
//#include "hr_Exception.h"
#include <d3dcompiler.h>
#include <filesystem>

#pragma comment( lib, "d3dcompiler" )

namespace lazer::Platform {

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
		if( m_Type != Type::Dynamic )
			throw std::logic_error( "Cannot edit an vertex buffer that is static." );

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
		if( m_Type != Type::Dynamic )
			throw std::logic_error("Cannot edit an index buffer that is static.");

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
		com_ptr<ID3DBlob> pBlob;

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

			hr = pDevice->CreateInputLayout( pInputElements, numElements, pBlob->GetBufferPointer(), pBlob->GetBufferSize(), &m_pInputLayout );
		};

		if( info.source_code.empty() )
		{
			std::wstring vsPath = info.name + L"VS.cso";
			std::wstring psPath = info.name + L"PS.cso";

			D3DReadFileToBlob( vsPath.c_str(), &pBlob );
			pDevice->CreateVertexShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pVertexShader );

			create_input_layout();

			D3DReadFileToBlob( psPath.c_str(), &pBlob );
			pDevice->CreatePixelShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pPixelShader );
		}
		// compile from source code
		else
		{
			UINT CompileFlags = 0u;
#if defined(_DEBUG)
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
					&pBlob,
					&pErrorBlob 
				)
			) )
			{
				throw std::logic_error( (const char *)pErrorBlob->GetBufferPointer() );
			}
			pDevice->CreateVertexShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pVertexShader );

			create_input_layout();

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
					&pBlob,
					&pErrorBlob 
				)
			) )
			{
				throw std::logic_error( (const char *)pErrorBlob->GetBufferPointer() );
			}
			hr = pDevice->CreatePixelShader( pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &m_pPixelShader );

		}

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
		case lazer::Resource::VertexLayoutTypes::Float:
			return DXGI_FORMAT_R32_FLOAT;
		case lazer::Resource::VertexLayoutTypes::Float2:
			return DXGI_FORMAT_R32G32_FLOAT;
		case lazer::Resource::VertexLayoutTypes::Float3:
			return DXGI_FORMAT_R32G32B32_FLOAT;
		case lazer::Resource::VertexLayoutTypes::Float4:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case lazer::Resource::VertexLayoutTypes::Int:
			return DXGI_FORMAT_R32_SINT;
		case lazer::Resource::VertexLayoutTypes::Int2:
		case lazer::Resource::VertexLayoutTypes::Int3:
		case lazer::Resource::VertexLayoutTypes::Int4:
		default:
			throw 0x45;
		}
		return DXGI_FORMAT_UNKNOWN;
	}

	ConstantBufferDX11::ConstantBufferDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const CreateInfo & info )
		: m_pContext( pContext ), m_Type( info.type ), m_ByteSize( info.ByteSize ), m_Slot( info.slot )
	{
		D3D11_BUFFER_DESC bd = CD3D11_BUFFER_DESC{};
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bd.ByteWidth = std::max( info.ByteSize, 64u );

		pDevice->CreateBuffer( &bd, nullptr, &m_pBuffer );
	}

	void ConstantBufferDX11::Bind()
	{
		switch( m_Type )
		{
		case Type::Vertex:
			m_pContext->VSSetConstantBuffers( m_Slot, 1u, m_pBuffer.GetAddressOf() );
			break;
		case Type::Pixel:
			m_pContext->PSSetConstantBuffers( m_Slot, 1u, m_pBuffer.GetAddressOf() );
			break;

		default:break;
		}
	}

	void ConstantBufferDX11::Unbind()
	{
	}

	void ConstantBufferDX11::Update( void * pData )
	{
		D3D11_MAPPED_SUBRESOURCE msr = {};
		m_pContext->Map( m_pBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &msr );
		memcpy( msr.pData, pData, m_ByteSize );
		m_pContext->Unmap( m_pBuffer.Get(), 0u );
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
			srd.SysMemPitch = info.pSurface->width() * sizeof coloru;
			srd.SysMemSlicePitch = info.pSurface->byte_size();
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

}
