#include <Fission/Core/Graphics.hh>

namespace Fission::Noop
{
	struct Graphics : public Fission::Graphics
	{
		virtual API GetAPI() override {return API::Noop;}

		virtual void Draw( uint32_t vertexCount, uint32_t vertexOffset ) override {}

		virtual void DrawIndexed( uint32_t indexCount, uint32_t indexOffset, uint32_t vertexOffset ) override {}

		virtual void SetClipRect( rf32 rect ) override {}
		
		virtual gfx::FrameBuffer*    CreateFrameBuffer   ( const gfx::FrameBuffer::CreateInfo & info  ) override;
		virtual gfx::VertexBuffer*   CreateVertexBuffer  ( const gfx::VertexBuffer::CreateInfo & info ) override;
		virtual gfx::ConstantBuffer* CreateConstantBuffer( const gfx::ConstantBuffer::CreateInfo & info ) override;
		virtual gfx::IndexBuffer*    CreateIndexBuffer   ( const gfx::IndexBuffer::CreateInfo & info  ) override;
		virtual gfx::Shader*         CreateShader        ( const gfx::Shader::CreateInfo & info       ) override;
		virtual gfx::Sampler*        CreateSampler       ( const gfx::Sampler::CreateInfo & info        ) override;
		virtual gfx::Texture2D*      CreateTexture2D     ( const gfx::Texture2D::CreateInfo & info    ) override;
		virtual gfx::Blender*        CreateBlender       ( const gfx::Blender::CreateInfo & info      ) override;
		virtual gfx::SwapChain*      CreateSwapChain     ( const gfx::SwapChain::CreateInfo & info    ) override;

		virtual native_handle_type native_handle() override {return nullptr;}

		virtual void Destroy() override {delete this;}
	};
}