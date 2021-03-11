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

	class ConstantBufferDX11 : Resource::IBindable
	{
	public:
		struct Variable
		{
			D3D_SHADER_VARIABLE_TYPE m_type;
			D3D_SHADER_VARIABLE_CLASS m_class;
			uint32_t m_columns;
			uint32_t m_rows;
			uint32_t m_elements;
			uint32_t m_offset;

			template <typename T>
			bool IsSame() {
				return ( m_type == T::hlsltype
					&& m_class == T::hlslclass
					&& m_columns == T::columns
					&& m_rows == T::rows );
			}
		};

		ConstantBufferDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, uint32_t slot, uint32_t size );

		virtual void Bind() override;
		virtual void Unbind() override {};

		// used for setting scalar/vector/matrix values by name
		template <typename T>
		bool Set( const std::string & name, const typename T::type & value )
		{
			using val_type = typename T::type;
			auto it = m_VariableMap.find( name );
			if( it == m_VariableMap.end() ) return false; // variable not found
			auto & var = it->second;
			if( !var.IsSame<T>() ) return false; // not the same type
			val_type * pVal = reinterpret_cast<val_type *>( m_pData.get() + it->second.m_offset );
			*pVal = value;
			m_bDirty = true;
			return true;
		}

		void AddVariable( const std::string & name, const Variable & var )
		{
			m_VariableMap.emplace( name, var );
		}


	protected:
		ID3D11DeviceContext * m_pContext;
		com_ptr<ID3D11Buffer> m_pBuffer;
		uint32_t m_BindSlot;

	private:
		uint32_t m_ByteSize;
		std::unique_ptr<char[]> m_pData;
		std::unordered_map<std::string, Variable> m_VariableMap;
		bool m_bDirty = false;
	};

	class PixelConstantBufferDX11 : public ConstantBufferDX11
	{
	public:
		PixelConstantBufferDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, uint32_t slot, uint32_t size );
		virtual void Bind() override;
	};
	class VertexConstantBufferDX11 : public ConstantBufferDX11
	{
	public:
		VertexConstantBufferDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, uint32_t slot, uint32_t size );
		virtual void Bind() override;
	};

	class ShaderDX11 : public Resource::Shader
	{
	public:
		ShaderDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const Resource::Shader::CreateInfo & info );

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual bool SetVariable( const char * name, float val ) override;
		virtual bool SetVariable( const char * name, vec2f val ) override;
		virtual bool SetVariable( const char * name, vec3f val ) override;
		virtual bool SetVariable( const char * name, vec4f val ) override;

		virtual bool SetVariable( const char * name, int val ) override;
		virtual bool SetVariable( const char * name, vec2i val ) override;
		virtual bool SetVariable( const char * name, vec3i val ) override;
		virtual bool SetVariable( const char * name, vec4i val ) override;

		virtual bool SetVariable( const char * name, mat2x2f val ) override;
		virtual bool SetVariable( const char * name, mat3x2f val ) override;
		virtual bool SetVariable( const char * name, mat3x3f val ) override;
		virtual bool SetVariable( const char * name, mat4x4f val ) override;

	private:
		template <typename T>
		inline bool _Set( const char * name, const typename T::type & val )
		{
			for( auto && cb : m_VertexCBuffers )
				if( cb.Set<T>( name, val ) ) return true;
			for( auto && cb : m_PixelCBuffers )
				if( cb.Set<T>( name, val ) ) return true;
			return false;
		}

		static DXGI_FORMAT get_format( Resource::VertexLayoutTypes::Type type );
	private:

		ID3D11DeviceContext * m_pContext;
		com_ptr<ID3D11InputLayout> m_pInputLayout;
		com_ptr<ID3D11VertexShader> m_pVertexShader;
		com_ptr<ID3D11PixelShader> m_pPixelShader;
		std::vector<PixelConstantBufferDX11> m_PixelCBuffers;
		std::vector<VertexConstantBufferDX11> m_VertexCBuffers;
		std::wstring m_Name;
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
