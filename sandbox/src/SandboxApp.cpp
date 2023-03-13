#include <Fission/Platform/EntryPoint.h>
#include <Fission/Core/Monitor.hh>
#include <Fission/Base/Time.hpp>
#include <Fission/Simple2DLayer.h>

#include "../Fission/vendor/json/single_include/nlohmann/json.hpp"

template <typename T>
struct DefaultDelete : public T { virtual void Destroy() override { delete this; } };

using namespace Fission::base;
static Fission::IFRenderer2D * g_r2d;

// Testing SDF-based text rendering, will probably settle on using two text renderers:
//   1. Simple FAST text renderer that uses a simple single channel texture (For console and debug interfaces)
//	 2. Scalable text renderer that uses SDF or MSDF
struct TextRenderer : public Fission::IFRenderer {
	virtual void OnCreate( Fission::IFGraphics* gfx, Fission::size2 _Viewport_Size ) {
		vertex_data = (vertex*)_aligned_malloc( vertex_max_count * sizeof vertex, 32 );
		index_data = (uint32_t*)_aligned_malloc( index_max_count * sizeof uint32_t, 32 );
		_viewport_size = _Viewport_Size;
		OnRecreate( gfx );
	}
	virtual void OnRecreate( Fission::IFGraphics* gfx ) {
		m_pGraphics = gfx;
		using namespace Fission;
		using namespace Fission::Resource;
		using namespace Fission::Resource::VertexLayoutTypes;

		auto vl = VertexLayout{};
		vl.Append( Float2, "Position" );
		vl.Append( Float2, "TexCoord" );

		{ // Create Vertex Buffer
			IFVertexBuffer::CreateInfo info;
			info.vtxCount = vertex_max_count;
			info.pVertexLayout = &vl;
			info.type = IFVertexBuffer::Type::Dynamic;
			m_pVertexBuffer = gfx->CreateVertexBuffer( info );
		}
		{ // Create Index Buffer
			IFIndexBuffer::CreateInfo info;
			info.idxCount = index_max_count;
			info.size = IFIndexBuffer::Size::UInt32;
			info.type = IFIndexBuffer::Type::Dynamic;
			m_pIndexBuffer = gfx->CreateIndexBuffer( info );
		}
		{ // Create Index Buffer
			IFConstantBuffer::CreateInfo info;
			info.type = IFConstantBuffer::Type::Dynamic;
			info.max_size = 128;
			m_pTransformBuffer = gfx->CreateConstantBuffer( info );

			const auto res = v2f32{ (float)_viewport_size.w, (float)_viewport_size.h };

			const auto screen = m44(
				2.0f / res.x, 0.0f, -1.0f, 0.0f,
				0.0f, -2.0f / res.y, 1.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			).transpose();

			m_pTransformBuffer->SetData( &screen, sizeof( screen ) );
		}
		{ // Create Shaders
			IFShader::CreateInfo info;
			info.pVertexLayout = &vl;
			info.sourceCode = R"(
cbuffer Transform : register(b0)
{
	matrix screen;
}

struct VS_OUT { 
	float2 tc : TexCoord; 
	float4 pos : SV_Position; 
};
VS_OUT vs_main( float2 pos : Position, float2 tc : TexCoord ) { 
	VS_OUT vso; 
	vso.pos = mul( float4( pos, 1.0, 1.0 ), screen ); 
	vso.tc = tc;
	return vso; 
}

Texture2D tex;
SamplerState ss;

float median(float a, float b, float c) {
    return max(min(a,b), min(max(a,b), c));
}

float4 ps_main( float2 tc : TexCoord ) : SV_Target { 
    float3 dist = tex.Sample( ss, tc ).rgb;
    
    float d = median(dist.r, dist.g, dist.b) - 0.5;

    float w = clamp(d/fwidth(d) + 0.5, 0.0, 1.0);
    
    float4 outside = float4(0, 0, 0, 0);
    float4 inside = float4(1, 1, 1, 1);
    float4 color = lerp(outside, inside, w);
    
    return color;
}
			)";
			m_pShader = gfx->CreateShader( info );
		}
		{
			IFBlender::CreateInfo info;
			info.blend = IFBlender::Blend::Normal;
			m_pBlender = gfx->CreateBlender( info );
		}
		{
			IFSampler::CreateInfo info;
			info.filter = IFSampler::Linear;
			m_pSampler = gfx->CreateSampler( info );
		}
		{
			auto surface = Fission::Surface::Create();
			surface->Load( "roboto.png" );
			IFTexture2D::CreateInfo info = {};
			info.pSurface = surface.get();
			m_pTexture = gfx->CreateTexture2D( info );
		}
	}

	virtual void OnResize( Fission::IFGraphics* _Ptr_Graphics, Fission::size2 size ) {
		const auto screen = Fission::m44(
			2.0f / (float)size.w, 0.0f,                -1.0f, 0.0f,
			0.0f,                -2.0f / (float)size.h, 1.0f, 0.0f,
			0.0f,                 0.0f,                 1.0f, 0.0f,
			0.0f,                 0.0f,                 0.0f, 1.0f
		).transpose();

		m_pTransformBuffer->SetData( &screen, sizeof( screen ) );
	}

	void Destroy()
	{
		_aligned_free( vertex_data );
		_aligned_free( index_data );
		delete this;
	}

	void Render(float s) {
		index_data[icount++] = 0;
		index_data[icount++] = 1;
		index_data[icount++] = 3;
		index_data[icount++] = 0;
		index_data[icount++] = 2;
		index_data[icount++] = 3;

		float tcl =            290.5f / 820.0f;
		float tcb = ( 820.0f - 375.5f) / 820.0f;
		float tcr =            307.5f / 820.0f;
		float tct = ( 820.0f - 401.5f ) / 820.0f;

		float pl = 0.0063911593993426913f, pb = -0.049594541139240556f, pr= 0.53608930935065724f, pt= 0.76053204113924044f;

		float x = 100.0f, y = 700.0f;

		float x0 = x + pl * s;
		float y0 = y - pt * s;
		float x1 = x + pr * s;
		float y1 = y - pb * s;

		vertex_data[vcount++] = {{x0, y0}, {tcl, tct}};
		vertex_data[vcount++] = {{x1, y0}, {tcr, tct}};
		vertex_data[vcount++] = {{x0, y1}, {tcl, tcb}};
		vertex_data[vcount++] = {{x1, y1}, {tcr, tcb}};
		
		m_pVertexBuffer->SetData( vertex_data, vcount );
		m_pIndexBuffer->SetData( index_data, icount );

		m_pVertexBuffer->Bind();
		m_pIndexBuffer->Bind();
		m_pTransformBuffer->Bind( Fission::Resource::IFConstantBuffer::Target::Vertex, 0 );
		m_pShader->Bind();
		m_pBlender->Bind();
		m_pTexture->Bind(0);
		m_pSampler->Bind(Fission::Resource::IFSampler::Pixel, 0);

		m_pGraphics->DrawIndexed( icount );
		icount = vcount = 0;
	}

	struct vertex {
		Fission::v2f32 pos;
		Fission::v2f32 tc;
	};

	Fission::IFGraphics* m_pGraphics = nullptr;

	Fission::fsn_ptr<Fission::Resource::IFVertexBuffer>   m_pVertexBuffer;
	Fission::fsn_ptr<Fission::Resource::IFIndexBuffer>    m_pIndexBuffer;
	Fission::fsn_ptr<Fission::Resource::IFConstantBuffer> m_pTransformBuffer;
	Fission::fsn_ptr<Fission::Resource::IFShader>		  m_pShader;
	Fission::fsn_ptr<Fission::Resource::IFSampler>		  m_pSampler;
	Fission::fsn_ptr<Fission::Resource::IFBlender>		  m_pBlender;
	Fission::fsn_ptr<Fission::Resource::IFTexture2D>	  m_pTexture;

	vertex * vertex_data = nullptr;
	uint32_t* index_data = nullptr;

	Fission::u32 vcount = 0;
	Fission::u32 icount = 0;

	static constexpr int vertex_max_count = 1000;
	static constexpr int index_max_count  = 2000;

	Fission::size2 _viewport_size;
};

using namespace Fission;
struct SettingsScene : public DefaultDelete<Fission::IFScene>
{
	virtual void OnCreate(FApplication* app) override {
		r2d = app->f_pEngine->GetRenderer<IFRenderer2D>( "$internal2D" );
		tr  = app->f_pEngine->GetRenderer<TextRenderer>( "test" );
	}
	virtual void OnUpdate(Fission::timestep dt) override {

		if( hitbox[mouse] ) {
			hitbox.x.low  -= 20.0f * dt * ( hitbox.x.low  - (def_size.x.low  - 30.0f) );
			hitbox.x.high -= 20.0f * dt * ( hitbox.x.high - (def_size.x.high + 30.0f) );
			col -= 20.0f * dt * (col - colors::White);
			h -= 20.0f * dt * ( h - 32.0f );
		}
		else {
			hitbox.x.low  -= 20.0f * dt * ( hitbox.x.low  - def_size.x.low  );
			hitbox.x.high -= 20.0f * dt * ( hitbox.x.high - def_size.x.high );
			col -= 20.0f * dt * (col - rest_col);
			h -= 20.0f * dt * ( h - 28.0f );
		}

		r2d->SelectFont( FontManager::GetFont("$debug") );
	//	r2d->DrawRect( hitbox, colors::AliceBlue, 1.0f );
		r2d->FillRect( rf32{ hitbox.left(), hitbox.right(), hitbox.y.average() - 1.0f, hitbox.y.average() + 1.0f}, col);
		auto tl = r2d->CreateTextLayout( "Performance" );
		r2d->DrawString( "Performance", { 0.5f * (hitbox.width() - tl.width) + hitbox.left(), hitbox.y.average() - h}, col);
		r2d->Render();

		tr->Render(mouse.x / 3.0f);
	}
	virtual Fission::EventResult OnMouseMove(Fission::MouseMoveEventArgs& args) override {
		mouse = (v2f32)args.position;
		return EventResult::Handled;
	}
	virtual Fission::SceneKey GetKey() override { return {}; }

	v2f32 mouse = {};
	IFRenderer2D* r2d;
	TextRenderer* tr;

	static constexpr rf32 def_size = { 300.0f, 440.0f, 300.0f, 350.0f };
	rf32 hitbox = { 300.0f, 440.0f, 300.0f, 350.0f };
	static constexpr color rest_col = colors::DimGray;
	color col = rest_col;
	float h = 30.0f;
};

class MyApp : public Fission::FApplication
{
public:
	MyApp() : FApplication( "Sandbox", {2,2,8} ) {}

	virtual void OnStartUp( CreateInfo * info ) override
	{
		info->window.title = u8"🔥 Sandbox 🔥  👌👌👌👌👌";
		f_pEngine->RegisterRenderer( "test", new TextRenderer );
	}
	virtual Fission::IFScene * OnCreateScene( const Fission::SceneKey& key ) override
	{
		// ignore scene key and just create the main scene
		return new SettingsScene;
	}
	virtual void Destroy() override { delete this; }
};

Fission::FApplication * CreateApplication() {
	return new MyApp;
}