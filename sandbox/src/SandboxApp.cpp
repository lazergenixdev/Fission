#include <Fission/Platform/EntryPoint.h>
#include <Fission/Core/Monitor.hh>
#include <Fission/Base/Time.hpp>
#include <Fission/Simple2DLayer.h>
#include <Fission/Core/Graphics/Font.hh>
#include <Fission/Core/Graphics/Renderer2D.hh>
#include <Fission/Core/Graphics/TextRenderer.hh>
#include <Fission/Core/Graphics.hh>
#include <Fission/Core/Scene.hh>

namespace NotoSans_RegularTTF {
#include "Static Fonts/NotoSans-Regular.inl"
}

template <typename T>
struct DefaultDelete : public T { virtual void Destroy() override { delete this; } };

using namespace Fission;
using namespace Fission::base;
static Fission::Renderer2D * g_r2d;

static void* twemojifile = nullptr;
static float rendertime = 0.0f;

// Testing SDF-based text rendering, will probably settle on using two text renderers:
//   1. Simple FAST text renderer that uses a simple single channel texture (For console and debug interfaces)
//	 2. Scalable text renderer that uses SDF or MSDF
struct _TextRenderer : public Fission::Renderer {

	struct vertex {
		Fission::v2f32 pos;
		Fission::v2f32 tc;
	};

	struct DrawData {
		vertex* vertex_data = nullptr;
		uint32_t* index_data = nullptr;

		Fission::u32 vcount = 0;
		Fission::u32 icount = 0;
	};

	enum State {
		Kill       = 0x1,
		WantResize = 0x2,
		Complete   = 0x4,
	};

	virtual void OnCreate( Fission::Graphics* gfx, Fission::size2 _Viewport_Size ) {
		master_vertex_data = (vertex*)_aligned_malloc( vertex_max_count * sizeof vertex, 32 );
		master_index_data = (uint32_t*)_aligned_malloc( index_max_count * sizeof uint32_t, 32 );
		_viewport_size = _Viewport_Size;

		fntDraw.vertex_data = master_vertex_data;
		fntDraw.index_data  = master_index_data;

		emoDraw.vertex_data = master_vertex_data + vertex_max_count / 2;
		emoDraw.index_data  = master_index_data  + index_max_count / 2;

		OnRecreate( gfx );

		worker = std::thread( ThreadDriver, this );
	}
	virtual void OnRecreate( Fission::Graphics* gfx ) override {
		m_pGraphics = gfx;
		using namespace Fission;
		using namespace Fission::gfx;
		using namespace Fission::gfx::VertexLayoutTypes;

		auto vl = VertexLayout{};
		vl.Append( Float2, "Position" );
		vl.Append( Float2, "TexCoord" );

		{ // Create Vertex Buffer
			VertexBuffer::CreateInfo info;
			info.vtxCount = vertex_max_count;
			info.pVertexLayout = &vl;
			info.type = VertexBuffer::Type::Dynamic;
			m_pVertexBuffer = gfx->CreateVertexBuffer( info );
		}
		{ // Create Index Buffer
			IndexBuffer::CreateInfo info;
			info.idxCount = index_max_count;
			info.size = IndexBuffer::Size::UInt32;
			info.type = IndexBuffer::Type::Dynamic;
			m_pIndexBuffer = gfx->CreateIndexBuffer( info );
		}
		{ // Create Index Buffer
			ConstantBuffer::CreateInfo info;
			info.type = ConstantBuffer::Type::Dynamic;
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
			Shader::CreateInfo info;
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
SamplerState ss : register(s0);

float4 ps_main( float2 tc : TexCoord ) : SV_Target { 
    return tex.Sample( ss, tc );
}
			)";
			m_pShader = gfx->CreateShader( info );
		}
		{
			Blender::CreateInfo info;
			info.blend = Blender::Blend::Normal;
			m_pBlender = gfx->CreateBlender( info );
		}
		{
			Sampler::CreateInfo info;
			info.filter = Sampler::Linear;
			m_pSampler = gfx->CreateSampler( info );
		}
	}

	virtual void OnResize( Fission::Graphics* _Ptr_Graphics, Fission::size2 size ) {
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
		state |= Kill; // rip
		cv.notify_one();
		_aligned_free( master_vertex_data );
		_aligned_free( master_index_data );
		worker.join();
		delete this;
	}

	void Render() {
		m_pVertexBuffer->Bind();
		m_pIndexBuffer->Bind();
		m_pTransformBuffer->Bind( Fission::gfx::ConstantBuffer::Target::Vertex, 0 );
		m_pBlender->Bind();
		m_pShader->Bind();

		font->get_atlas()->Bind(0);
		m_pSampler->Bind(Fission::gfx::Sampler::Pixel, 0);
		Draw( fntDraw );

		font->get_emoji_atlas()->Bind(0);
		Draw( emoDraw );

		if( state & Complete ) {
			state = state & (~Complete);
			font.swap(resizefont);
		}

		if( font->size() != m_TargetSize ) {
			state |= WantResize;
			cv.notify_one();
		}
	}

	void Draw(DrawData& d) {
		m_pVertexBuffer->SetData( d.vertex_data, d.vcount );
		m_pIndexBuffer->SetData( d.index_data, d.icount );
		m_pGraphics->DrawIndexed( d.icount );
		d.icount = d.vcount = 0;
	}

	void SetFont( UIFont* fnt ) {
		font = fnt;
		m_pFontTexture  = font->get_atlas();
		m_pEmojiTexture = font->get_emoji_atlas();
		m_TargetSize = font->size();

		font_info.emojifile    = twemojifile;
		font_info.fontfile     = NotoSans_RegularTTF::data;
		font_info.fontfilesize = NotoSans_RegularTTF::size;
	}

	void _add_glyph( const v2f32& origin, const float& scale, DrawData& d, const Font::Glyph* g ) {
		const auto rect = rf32{
			origin.x + scale * g->rc.x.low,
			origin.x + scale * g->rc.x.high,
			origin.y + scale * g->rc.y.low,
			origin.y + scale * g->rc.y.high,
		};

		d.index_data[d.icount++] = d.vcount;
		d.index_data[d.icount++] = d.vcount + 1u;
		d.index_data[d.icount++] = d.vcount + 2u;
		d.index_data[d.icount++] = d.vcount + 3u;
		d.index_data[d.icount++] = d.vcount;
		d.index_data[d.icount++] = d.vcount + 2u;

		d.vertex_data[d.vcount++] = vertex( v2f32( rect.x.low,  rect.y.high ), v2f32( g->uv.x.low,  g->uv.y.high ) );
		d.vertex_data[d.vcount++] = vertex( v2f32( rect.x.low,  rect.y.low  ), v2f32( g->uv.x.low,  g->uv.y.low  ) );
		d.vertex_data[d.vcount++] = vertex( v2f32( rect.x.high, rect.y.low  ), v2f32( g->uv.x.high, g->uv.y.low  ) );
		d.vertex_data[d.vcount++] = vertex( v2f32( rect.x.high, rect.y.high ), v2f32( g->uv.x.high, g->uv.y.high ) );
	}

	void AddText( utf32_string_view sv, v2f32 pos ) {
		const float start = pos.x;
		const Font::Glyph* glyph;
		float scale = m_TargetSize / font->size();

		GetEngine()->GetDebug()->Text( "height = %.2f", font->height() );
		GetEngine()->GetDebug()->Text( "rendertime %.1f ms", rendertime );

		for( int i = 0; i < sv.size(); ++i ) {
			const chr& cp = sv.data()[i];
			if( cp == U'\r' || cp == U'\n' ) { pos.y += font->height() * scale; pos.x = start; continue; }
			glyph = font->lookup( cp );

			if( glyph == font->fallback() ) {
				int char_advance;
				if( auto g = font->lookup_emoji( &cp, char_advance ) )
				{
					_add_glyph( pos, scale, emoDraw, g );
					pos.x += g->advance * scale;
					i += char_advance - 1;
					continue;
				}
			}

			if( cp != U' ' ) {
				_add_glyph( pos, scale, fntDraw, glyph );
			}

			pos.x += glyph->advance * scale;
		}
	}

	static void __cdecl ThreadDriver(_TextRenderer* pthis) {
		pthis->ThreadMain();
	}

	void ThreadMain() {
		Fission::simple_timer t;
		while( (state & Kill) == 0 ) {
			std::unique_lock lock{worker_mutex};
			cv.wait( lock );

			t.reset();
			if( state & WantResize ) {
				font_info.size = m_TargetSize;
				resizefont = UIFont::Create( font_info );
				state |= Complete;
				state = state & (~WantResize);
			}

			rendertime = t.getms();
		}
	}

	void resize( float ns ) {
		m_TargetSize = std::clamp(ns, 6.0f, 64.0f);
	}

	int heikjdfsgh() { return (int)font->height(); }

	Fission::Graphics* m_pGraphics = nullptr;

	Fission::fsn_ptr<Fission::gfx::VertexBuffer>   m_pVertexBuffer;
	Fission::fsn_ptr<Fission::gfx::IndexBuffer>    m_pIndexBuffer;
	Fission::fsn_ptr<Fission::gfx::ConstantBuffer> m_pTransformBuffer;
	Fission::fsn_ptr<Fission::gfx::Shader>		 m_pShader;
	Fission::fsn_ptr<Fission::gfx::Sampler>		 m_pSampler;
	Fission::fsn_ptr<Fission::gfx::Blender>		 m_pBlender;

	Fission::gfx::Texture2D*	  m_pFontTexture;
	Fission::gfx::Texture2D*	  m_pEmojiTexture;

	DrawData fntDraw;
	DrawData emoDraw;

	float m_TargetSize;
	UIFont::CreateInfo font_info;

	fsn_ptr<UIFont> font;
	fsn_ptr<UIFont> resizefont;

	std::thread	worker;
	std::mutex  worker_mutex;
	std::condition_variable cv;
	int state = 0;

	vertex*   master_vertex_data = nullptr;
	uint32_t* master_index_data = nullptr;

	static constexpr int vertex_max_count = 4000;
	static constexpr int index_max_count  = 6000;

	Fission::size2 _viewport_size;
};

struct SettingsScene : public DefaultDelete<Fission::Scene>
{
	virtual void OnCreate(Application* app) override {
		r2d = app->f_pEngine->GetRenderer<Renderer2D>( "$internal2D" );
		tr  = app->f_pEngine->GetRenderer<_TextRenderer>( "test" );
		other= app->f_pEngine->GetRenderer<TextRenderer>( "$Text" );

#define FILE_ "../resources/Fonts/Noto Sans/atlas"

		const void* csv = nullptr;
		auto path = std::filesystem::absolute( FILE_ ".csv" ).make_preferred();
		{
			FILE* f = fopen( path.string().c_str(), "r");
			fseek( f, 0, SEEK_END );
			long fsize = ftell( f );
			fseek( f, 0, SEEK_SET );  /* same as rewind(f); */
			csv = malloc( fsize );
			fread( (char*)csv, fsize, 1, f );
			fclose( f );
		}
		fnt = SDFFont::Create({ csv, 0, path.replace_extension("png").string().c_str()});
		
		{
			FILE* f = fopen( std::filesystem::absolute("../resources/twemoji.bin").make_preferred().string().c_str(), "rb" );
			fseek( f, 0, SEEK_END );
			long fsize = ftell( f );
			fseek( f, 0, SEEK_SET );  /* same as rewind(f); */
			twemojifile = malloc( fsize );
			fread( twemojifile, fsize, 1, f );
			fclose( f );
		}

		UIFont::CreateInfo font_info;
		font_info.size         = 8.0f;
		font_info.emojifile    = twemojifile;
		font_info.fontfile     = NotoSans_RegularTTF::data;
		font_info.fontfilesize = NotoSans_RegularTTF::size;
		tr->SetFont( UIFont::Create( font_info ) );

		wnd = app->f_pMainWindow;
	}
	virtual void OnUpdate(Fission::timestep dt) override {


		tr->AddText( U"The😵five❤👨🏽‍✈️👩🏿‍❤️‍💋‍👩🏻👩🏻‍🚒🤏🏾🤞🏿boxing🙀wizards🍖jump👇quickly⛲", { 100.0f, 100.0f } );
	//	tr->AddText( U"🀄❤❤⛲❤🦷🥽🥤❤⛲", { 100.0f, 100.0f } );
	//	tr->AddText( utf32_string_view{text, (unsigned)size}, { 50.0f, 200.0f } );

		other->add_text_sdf( fnt.get(), "aBcDeFgHiJkLmNoPqRsTuVwXyZ", { 0.0f, 300.0f }, mouse.x*0.3f );
		other->render();

		r2d->DrawRect( {0.0f, 300.0f, 300.0f, 300.0f+mouse.x*0.3f}, Fission::colors::Green, 2.0f );

		srand( 2314 );
		for( int i = 0; i < 1000; ++i ) {
			r2d->FillRect( rf32::from_center(float(rand() % 1280), float(rand() % 720), 100.0f, 100.0f), color{colors::LightGreen, 0.1f});
		}

		wnd->_debug_set_position( { 50,200 + tr->heikjdfsgh() } );

		if( size ) {
			std::stringstream ss;
			ss << std::hex << (unsigned int)text[size - 1];
			r2d->SelectFont( GetEngine()->GetFont("$debug") );
			r2d->DrawString( ss.str().c_str(), {100.0f, 500.0f}, col);
		}
		r2d->Render();
		tr->Render();
	}
	virtual Fission::EventResult OnMouseMove(Fission::MouseMoveEventArgs& args) override {
		mouse = (v2f32)args.position;
		if(tr) tr->resize( std::floor(mouse.x / 10.0f) );
		return EventResult::Handled;
	}
	virtual Fission::EventResult OnTextInput(Fission::TextInputEventArgs& args) override {
		switch( args.codepoint )
		{
		case U'\b': { size = std::max( size - 1, 0 ); break; }
		default: {
			text[size++] = args.codepoint;
			text[size] = {};
			break;
		}
		}
		return EventResult::Handled;
	}
	virtual Fission::SceneKey GetKey() override { return {}; }

	v2f32 mouse = {};
	Renderer2D* r2d;
	_TextRenderer* tr = nullptr;
	TextRenderer* other = nullptr;
	fsn_ptr<SDFFont> fnt = nullptr;
	Window* wnd = nullptr;

	chr text[32] = U"🧜🏻‍♀️";
	int size = 5;

	static constexpr rf32 def_size = { 300.0f, 440.0f, 300.0f, 350.0f };
	rf32 hitbox = { 300.0f, 440.0f, 300.0f, 350.0f };
	static constexpr color rest_col = colors::DimGray;
	color col = rest_col;
	float h = 30.0f;
};

class MyApp : public Fission::Application
{
public:
	MyApp() : Application( "Sandbox", {2,2,8} ) {}

	virtual void OnStartUp( CreateInfo * info ) override
	{
		info->window.title = u8"emoji modifiers are a nightmare to code, just fucking kill me, like who TF thought of this??!!??";
		f_pEngine->RegisterRenderer( "test", new _TextRenderer );
		Fission::TextRenderer* tr;
		Fission::CreateTextRenderer( &tr );
		f_pEngine->RegisterRenderer( "$Text", tr );
	}
	virtual Fission::Scene * OnCreateScene( const Fission::SceneKey& key ) override
	{
		// ignore scene key and just create the main scene
		return new SettingsScene;
	}
	virtual void Destroy() override { delete this; }
};

Fission::Application * CreateApplication() {
	return new MyApp;
}