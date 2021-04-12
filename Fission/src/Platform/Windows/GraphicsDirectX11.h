#pragma once
#include "Fission/Core/Graphics/Graphics.h"
#include <d3d11.h>

namespace Fission::Platform
{

	class GraphicsDirectX11 : public Graphics
	{
	public:
		GraphicsDirectX11();

		virtual ~GraphicsDirectX11() override;


		virtual API GetAPI() override;

		virtual void SetVSync( bool vsync ) override;

		virtual bool GetVSync() override;


		virtual void SetFrameBuffer( Resource::FrameBuffer * buffer ) override;


		virtual void Draw( uint32_t vertexCount, uint32_t vertexOffset ) override;

		virtual void DrawIndexed( uint32_t indexCount, uint32_t indexOffset, uint32_t vertexOffset ) override;

		virtual scoped<FrameBuffer>				CreateFrameBuffer	( const FrameBuffer::CreateInfo & info		) override;
		virtual std::unique_ptr<VertexBuffer>	CreateVertexBuffer	( const VertexBuffer::CreateInfo & info		) override;
		virtual std::unique_ptr<IndexBuffer>	CreateIndexBuffer	( const IndexBuffer::CreateInfo & info		) override;
		virtual std::unique_ptr<Shader>			CreateShader		( const Shader::CreateInfo & info			) override;
		virtual std::unique_ptr<Texture2D>		CreateTexture2D		( const Texture2D::CreateInfo & info		) override;
		virtual std::unique_ptr<Blender>		CreateBlender		( const Blender::CreateInfo & info			) override;

		virtual native_handle_type native_handle() override;

	public:

		static bool CheckSupport();

	private:
		uint32_t							m_SyncInterval = 1u;

		com_ptr<ID3D11Device>				m_pDevice;
		com_ptr<ID3D11DeviceContext>		m_pImmediateContext;

		native_type_dx11					m_NativeHandle;
	};

}
