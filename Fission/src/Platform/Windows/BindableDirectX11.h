#pragma once
#include <Fission/Core/Graphics/Bindable.hh>
#include <d3d11.h>

namespace Fission::Platform {

	class VertexBufferDX11 : public Resource::IFVertexBuffer
	{
	public:
		VertexBufferDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const CreateInfo & info );

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void SetData( const void * pVertexData, uint32_t vtxCount ) override;

		virtual uint32_t GetCount() override;

		virtual void Destroy() override;
	private:
		ID3D11DeviceContext * m_pContext;
		com_ptr<ID3D11Buffer> m_pBuffer;

		const Type m_Type;

		uint32_t m_Stride;
		uint32_t m_Count;
	};

	class IndexBufferDX11 : public Resource::IFIndexBuffer
	{
	public:
		IndexBufferDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const CreateInfo & info );

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void SetData( const void * pIndexData, uint32_t idxCount ) override;

		virtual uint32_t GetCount() override;

		virtual void Destroy() override;
	private:
		ID3D11DeviceContext * m_pContext;
		com_ptr<ID3D11Buffer> m_pBuffer;

		const Type m_Type;

		uint32_t m_Stride;
		uint32_t m_Count;
	};

	class ConstantBufferDX11 : public Resource::IFConstantBuffer
	{
	public:
		ConstantBufferDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const CreateInfo & info );

		virtual void Bind() override;
		virtual void Unbind() override {};

		virtual void Bind( Target target, int slot ) override;
		virtual void SetData( const void * pData, uint32_t size ) override;

		virtual void Destroy() override;

	private:
		ID3D11DeviceContext * m_pContext;
		com_ptr<ID3D11Buffer> m_pBuffer;
	};

	class ShaderDX11 : public Resource::IFShader
	{
	public:
		ShaderDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const CreateInfo & info );

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void Destroy() override;
	private:

		static DXGI_FORMAT get_format( Resource::VertexLayoutTypes::Type type );
	private:

		ID3D11DeviceContext * m_pContext;
		com_ptr<ID3D11InputLayout> m_pInputLayout;
		com_ptr<ID3D11VertexShader> m_pVertexShader;
		com_ptr<ID3D11PixelShader> m_pPixelShader;
		std::wstring m_Name;
	};


	class Texture2DDX11 : public Resource::IFTexture2D
	{
	public:
		Texture2DDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const CreateInfo & info );

		virtual void Bind( int slot ) override;
		virtual void Bind() override;
		virtual void Unbind() override;

		virtual uint32_t GetWidth() override;
		virtual uint32_t GetHeight() override;

		virtual void Destroy() override;
	private:
		ID3D11DeviceContext * m_pContext;
		com_ptr<ID3D11Texture2D> m_pTexture;
		com_ptr<ID3D11ShaderResourceView> m_pShaderResourceView;
		uint32_t m_Slot;
		uint32_t m_Width, m_Height;
	};

	class BlenderDX11 : public Resource::IFBlender
	{
	public:
		BlenderDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const CreateInfo & info );

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void Destroy() override;
	private:
		ID3D11DeviceContext * m_pContext;
		com_ptr<ID3D11BlendState> m_pBlendState;
	};

	class SwapChainDX11 : public Resource::IFSwapChain
	{
	public:
		SwapChainDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const CreateInfo & info );

		virtual void Resize( base::size ) override;

		virtual base::size GetSize() override;

	//	virtual Resource::FrameBuffer * GetBackBuffer() override;

		virtual void SetFullscreen( bool fullscreen, Monitor * pMonitor ) override;

		virtual void Clear( color clear_color ) override;

		virtual void Present( vsync_ vsync ) override;

		virtual void Bind() override;

		virtual void Unbind() override {}

		virtual void Destroy() override;
	private:
		std::mutex							m_Mutex;
		uint32_t *							m_pSyncInterval = nullptr;
		base::size							m_Resolution;
		D3D11_VIEWPORT						m_ViewPort;
		ID3D11DeviceContext *				m_pContext;
		com_ptr<ID3D11RenderTargetView>		m_pRenderTargetView;
		com_ptr<IDXGISwapChain>				m_pSwapChain;
	};
	//class FrameBufferDX11 : public Resource::FrameBuffer
	//{
	//public:
	//	FrameBufferDX11( ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const Resource::FrameBuffer::CreateInfo & info );

	//	virtual vec2i GetSize() override;

	//	virtual void Clear( color clear_color ) override;

	//	ID3D11RenderTargetView * GetRenderTargetView();
	//	IDXGISwapChain * GetSwapChain();
	//	D3D11_VIEWPORT * GetViewPort();
	//private:
	//	uint32_t *							m_pSyncInterval = nullptr;
	//	vec2i								m_Resolution;
	//	D3D11_VIEWPORT						m_ViewPort;
	//	ID3D11DeviceContext *				m_pContext;
	//	com_ptr<ID3D11RenderTargetView>		m_pRenderTargetView;
	//	com_ptr<IDXGISwapChain>				m_pSwapChain;
	//	class Window *						m_pParentWindow;
	//};

} // namespace Fission::Platform
