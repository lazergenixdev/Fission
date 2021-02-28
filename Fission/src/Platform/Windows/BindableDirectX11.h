#pragma once
#include "Fission/Core/Graphics/Bindable.h"
#include <d3d11.h>

namespace Fission::Platform {

	class VertexBufferDX11 : public Resource::VertexBuffer
	{
	public:
		VertexBufferDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const Resource::VertexBuffer::CreateInfo & info );

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void SetData( const void * pVertexData, uint32_t vtxCount ) override;

		virtual uint32_t GetCount() override;

	private:
		ID3D11DeviceContext * m_pContext;
		com_ptr<ID3D11Buffer> m_pBuffer;

		const Type m_Type;

		uint32_t m_Stride;
		uint32_t m_Count;
	};

	class IndexBufferDX11 : public Resource::IndexBuffer
	{
	public:
		IndexBufferDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const Resource::IndexBuffer::CreateInfo & info );

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void SetData( const void * pIndexData, uint32_t idxCount ) override;

		virtual uint32_t GetCount() override;

	private:
		ID3D11DeviceContext * m_pContext;
		com_ptr<ID3D11Buffer> m_pBuffer;

		const Type m_Type;

		uint32_t m_Stride;
		uint32_t m_Count;
	};

	class ShaderDX11 : public Resource::Shader
	{
	public:
		ShaderDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const Resource::Shader::CreateInfo & info );

		virtual void Bind() override;
		virtual void Unbind() override;

	private:
		static DXGI_FORMAT get_format( Resource::VertexLayoutTypes::Type type );
	private:

		ID3D11DeviceContext * m_pContext;
		com_ptr<ID3D11InputLayout> m_pInputLayout;
		com_ptr<ID3D11VertexShader> m_pVertexShader;
		com_ptr<ID3D11PixelShader> m_pPixelShader;
		std::wstring m_Name;
	};

	class ConstantBufferDX11 : public Resource::ConstantBuffer
	{
	public:
		ConstantBufferDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const Resource::ConstantBuffer::CreateInfo & info );

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void Update( void * pData ) override;

	private:
		ID3D11DeviceContext * m_pContext;
		com_ptr<ID3D11Buffer> m_pBuffer;
		Type m_Type;
		uint32_t m_Slot;
		uint32_t m_ByteSize;
	};

	class Texture2DDX11 : public Resource::Texture2D
	{
	public:
		Texture2DDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const Resource::Texture2D::CreateInfo & info );

		virtual void Bind( int slot ) override;
		virtual void Bind() override;
		virtual void Unbind() override;

		virtual uint32_t GetWidth() override;
		virtual uint32_t GetHeight() override;

	private:
		ID3D11DeviceContext * m_pContext;
		com_ptr<ID3D11Texture2D> m_pTexture;
		com_ptr<ID3D11ShaderResourceView> m_pShaderResourceView;
		uint32_t m_Slot;
		uint32_t m_Width, m_Height;
	};

	class BlenderDX11 : public Resource::Blender
	{
	public:
		BlenderDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const Resource::Blender::CreateInfo & info );

		virtual void Bind() override;
		virtual void Unbind() override;

	private:
		ID3D11DeviceContext * m_pContext;
		com_ptr<ID3D11BlendState> m_pBlendState;
	};

}
