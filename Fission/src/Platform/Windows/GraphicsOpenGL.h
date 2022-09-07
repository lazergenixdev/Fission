#include <Fission/Core/Graphics.hh>

namespace Fission
{
	class GraphicsOpenGL : public IFGraphics
	{
	public:
		GraphicsOpenGL();

		virtual API GetAPI() override;

		virtual void Draw( uint32_t vertexCount, uint32_t vertexOffset ) override;

		virtual void DrawIndexed( uint32_t indexCount, uint32_t indexOffset, uint32_t vertexOffset ) override;

		virtual void SetClipRect( rf32 rect ) override;

		virtual FrameBuffer* CreateFrameBuffer( const FrameBuffer::CreateInfo& info ) override;
		virtual VertexBuffer* CreateVertexBuffer( const VertexBuffer::CreateInfo& info ) override;
		virtual ConstantBuffer* CreateConstantBuffer( const ConstantBuffer::CreateInfo& info ) override;
		virtual IndexBuffer* CreateIndexBuffer( const IndexBuffer::CreateInfo& info ) override;
		virtual Shader* CreateShader( const Shader::CreateInfo& info ) override;
		virtual Texture2D* CreateTexture2D( const Texture2D::CreateInfo& info ) override;
		virtual Blender* CreateBlender( const Blender::CreateInfo& info ) override;
		virtual SwapChain* CreateSwapChain( const SwapChain::CreateInfo& info ) override;

		virtual native_handle_type native_handle() override;

		virtual void Destroy() override;
	private:
		native_type_opengl m_NativeHandle;
	};
}