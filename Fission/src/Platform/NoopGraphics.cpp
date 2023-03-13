#include "NoopGraphics.h"
#include <thread>

using namespace Fission::Noop;
using namespace std::chrono_literals;

template <typename T>
struct Noop : public T {
	virtual void Bind() override {}
	virtual void Unbind() override {}
	virtual void Destroy() override { delete this; }
};

struct FrameBuffer		: public Noop<Fission::Resource::IFFrameBuffer> {
	virtual Fission::size2 GetSize() override { return{}; }
	virtual void Clear( Fission::color clear_color ) override {}
	virtual Graphics::Texture2D* GetTexture2D() override {return nullptr;}
};

struct VertexBuffer		: public Noop<Fission::Resource::IFVertexBuffer> {
	virtual void SetData( const void* pVertexData, uint32_t vtxCount ) override {}
	virtual uint32_t GetCount() override { return 0; }
};

struct ConstantBuffer	: public Noop<Fission::Resource::IFConstantBuffer> {
	virtual void SetData( const void* pVertexData, uint32_t vtxCount ) override {}
	virtual void Bind( Target target, int slot ) override {}
};

struct IndexBuffer		: public Noop<Fission::Resource::IFIndexBuffer> {
	virtual void SetData( const void* pVertexData, uint32_t vtxCount ) override {}
	virtual uint32_t GetCount() override { return 0; }
};

struct Shader			: public Noop<Fission::Resource::IFShader> {};

struct Sampler			: public Noop<Fission::Resource::IFSampler> {
	virtual void Bind( Target target, int slot ) override {}
};

struct Texture2D		: public Noop<Fission::Resource::IFTexture2D> {
	virtual uint32_t GetWidth() override { return 0; }
	virtual uint32_t GetHeight() override { return 0; }
	virtual void Bind( int slot ) override {}
	virtual Fission::Surface* GetSurface() override { return nullptr; }
};

struct Blender			: public Noop<Fission::Resource::IFBlender> {};

struct SwapChain		: public Noop<Fission::Resource::IFSwapChain> {
	virtual Fission::size2 GetSize() override { return{}; }
	virtual void Resize( Fission::size2 ) override {}
	virtual void SetFullscreen( bool fullscreen, Fission::Monitor* pMonitor ) override {}
	virtual void Clear( Fission::color clear_color ) override {}
	virtual void Present( Fission::vsync_ vsync ) override { std::this_thread::sleep_for(16ms); /* default to 60 FPS */ }
};


Graphics::FrameBuffer*    Graphics::CreateFrameBuffer    ( const Graphics::FrameBuffer::CreateInfo & info    ) {return new ::FrameBuffer;}
Graphics::VertexBuffer*   Graphics::CreateVertexBuffer   ( const Graphics::VertexBuffer::CreateInfo & info   ) {return new ::VertexBuffer;}
Graphics::ConstantBuffer* Graphics::CreateConstantBuffer ( const Graphics::ConstantBuffer::CreateInfo & info ) {return new ::ConstantBuffer;}
Graphics::IndexBuffer*    Graphics::CreateIndexBuffer    ( const Graphics::IndexBuffer::CreateInfo & info    ) {return new ::IndexBuffer;}
Graphics::Shader*         Graphics::CreateShader         ( const Graphics::Shader::CreateInfo & info         ) {return new ::Shader;}
Graphics::Sampler*         Graphics::CreateSampler       ( const Graphics::Sampler::CreateInfo & info        ) {return new ::Sampler;}
Graphics::Texture2D*      Graphics::CreateTexture2D      ( const Graphics::Texture2D::CreateInfo & info      ) {return new ::Texture2D;}
Graphics::Blender*        Graphics::CreateBlender        ( const Graphics::Blender::CreateInfo & info        ) {return new ::Blender;}
Graphics::SwapChain*      Graphics::CreateSwapChain      ( const Graphics::SwapChain::CreateInfo & info      ) {return new ::SwapChain;}
