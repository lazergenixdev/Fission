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

struct FrameBuffer		: public Noop<Fission::gfx::FrameBuffer> {
	virtual Fission::size2 GetSize() override { return{}; }
	virtual void Clear( Fission::color clear_color ) override {}
	virtual Fission::gfx::Texture2D* GetTexture2D() override {return nullptr;}
};

struct VertexBuffer		: public Noop<Fission::gfx::VertexBuffer> {
	virtual void SetData( const void* pVertexData, uint32_t vtxCount ) override {}
	virtual uint32_t GetCount() override { return 0; }
};

struct ConstantBuffer	: public Noop<Fission::gfx::ConstantBuffer> {
	virtual void SetData( const void* pVertexData, uint32_t vtxCount ) override {}
	virtual void Bind( Target target, int slot ) override {}
};

struct IndexBuffer		: public Noop<Fission::gfx::IndexBuffer> {
	virtual void SetData( const void* pVertexData, uint32_t vtxCount ) override {}
	virtual uint32_t GetCount() override { return 0; }
};

struct Shader			: public Noop<Fission::gfx::Shader> {};

struct Sampler			: public Noop<Fission::gfx::Sampler> {
	virtual void Bind( Target target, int slot ) override {}
};

struct Texture2D		: public Noop<Fission::gfx::Texture2D> {
	virtual uint32_t GetWidth() override { return 0; }
	virtual uint32_t GetHeight() override { return 0; }
	virtual void Bind( int slot ) override {}
	virtual Fission::Surface* GetSurface() override { return nullptr; }
};

struct Blender			: public Noop<Fission::gfx::Blender> {};

struct SwapChain		: public Noop<Fission::gfx::SwapChain> {
	virtual Fission::size2 GetSize() override { return{}; }
	virtual void Resize( Fission::size2 ) override {}
	virtual void SetFullscreen( bool fullscreen, Fission::Monitor* pMonitor ) override {}
	virtual void Clear( Fission::color clear_color ) override {}
	virtual void Present( Fission::vsync_ vsync ) override { std::this_thread::sleep_for(16ms); /* default to 60 FPS */ }
};


Fission::gfx::FrameBuffer*    Fission::Noop::Graphics::CreateFrameBuffer    ( const gfx::FrameBuffer::CreateInfo & info    ) {return new ::FrameBuffer;}
Fission::gfx::VertexBuffer*   Fission::Noop::Graphics::CreateVertexBuffer   ( const gfx::VertexBuffer::CreateInfo & info   ) {return new ::VertexBuffer;}
Fission::gfx::ConstantBuffer* Fission::Noop::Graphics::CreateConstantBuffer ( const gfx::ConstantBuffer::CreateInfo & info ) {return new ::ConstantBuffer;}
Fission::gfx::IndexBuffer*    Fission::Noop::Graphics::CreateIndexBuffer    ( const gfx::IndexBuffer::CreateInfo & info    ) {return new ::IndexBuffer;}
Fission::gfx::Shader*         Fission::Noop::Graphics::CreateShader         ( const gfx::Shader::CreateInfo & info         ) {return new ::Shader;}
Fission::gfx::Sampler*        Fission::Noop::Graphics::CreateSampler        ( const gfx::Sampler::CreateInfo & info        ) {return new ::Sampler;}
Fission::gfx::Texture2D*      Fission::Noop::Graphics::CreateTexture2D      ( const gfx::Texture2D::CreateInfo & info      ) {return new ::Texture2D;}
Fission::gfx::Blender*        Fission::Noop::Graphics::CreateBlender        ( const gfx::Blender::CreateInfo & info        ) {return new ::Blender;}
Fission::gfx::SwapChain*      Fission::Noop::Graphics::CreateSwapChain      ( const gfx::SwapChain::CreateInfo & info      ) {return new ::SwapChain;}
