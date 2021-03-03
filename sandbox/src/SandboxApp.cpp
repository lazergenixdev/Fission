#include <Fission/Platform/EntryPoint.h>
#include <Fission/Fission.h>
#include <Fission/Core/UI/UI.h>

#include <numbers>
#include <iostream>

using namespace Fission;

namespace CustomFont {
#include "Static Fonts/Nunito-SemiBold.inl"
}

static color randColor() {
	return color( (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX );
}

class SandboxLayer : public ILayer 
{
public:
	static constexpr int nSides = 60;
	SandboxLayer() : mesh( 100, 200 )
	{
	}
	virtual void OnCreate() override
	{
		FontManager::SetFont( "myfont", CustomFont::data, CustomFont::size, 20.0f );
		Font * font = FontManager::GetFont( "myfont" );

		pRenderer2D = Renderer2D::Create( GetApp()->GetGraphics() );
		pRenderer2D->SelectFont( font );


		const int nVerticies = nSides + 1;

		mesh.push_color( Colors::White );
		mesh.push_vertex( {}, 0 );

		for( int i = 0; i < nSides; i++ ) {
			float factor = ( (float)i / (float)nSides );
			mesh.push_color( {} );
			mesh.push_vertex( vec2f::from_angle( std::numbers::pi_v<float> *2.0f * factor ), i + 1 );
		}
		for( int i = 0; i < nSides - 1; i++ ) {
			mesh.push_index( 0 );
			mesh.push_index( (i + 1)%nVerticies );
			mesh.push_index( (i + 2)%nVerticies );
		}
		mesh.push_index( 0 );
		mesh.push_index( nSides );
		mesh.push_index( 1 );
	}
	virtual void OnUpdate() override
	{
		pRenderer2D->FillRect( rectf::from_center( { 500.0f, 300.0f }, { 50.0f, 50.0f } ), c );
		pRenderer2D->DrawRect( rectf::from_center( { 500.0f, 300.0f }, { 50.0f, 50.0f } ), Colors::White, 2.0f );

		mesh.set_color( 0, colorf_hsv( 0.0f, 0.0f, v ) );
		for( int i = 0; i < nSides; i++ ) {
			float factor = ( (float)i / (float)nSides );
			mesh.set_color( i + 1, colorf_hsv( factor, 1.0f, v ) );
		}
		pRenderer2D->PushTransform( mat3x2f::Transform( 0.0f, 100.0f, 700.0f, 350.0f ) );
		pRenderer2D->DrawMesh( &mesh );
		pRenderer2D->PopTransform();

		pRenderer2D->FillCircle( { 200.0f, 500.0f }, 100.0f, Colors::White );


		pRenderer2D->DrawLine( { 400.5f, 50.0f }, vec2f{ 400.5f, 450.0f }, Colors::White );

		float cs = cosf( t.peeks() * 2.5f ) * 100.0f;
		float sn = sinf( t.peeks() * 2.5f ) * 100.0f;

		pRenderer2D->FillCircle( { 250.0f, 250.0f }, 100.0f, coloru(25, 25, 25) );
		pRenderer2D->FillArrow( { 200.0f, 250.0f }, vec2f{ 250.0f + cs, 250.0f + sn }, scale, Colors::White );

		pRenderer2D->DrawString( L"Sandbox app", { 450.0f, 50.0f }, Colors::White );

		DebugLayer::Push( "HECK" );
		int vmaj, vmin, vpat;
		Fission::GetVersion( &vmaj, &vmin, &vpat );
		DebugLayer::Text( "Fission Engine v%i.%i.%i", vmaj, vmin, vpat );
		DebugLayer::Text( "what in the heck %i", 5 );
		DebugLayer::Text( "what in the %i", 5 );
		DebugLayer::Text( "what %i", 5 );
		DebugLayer::Text( "%i-%i-%.1f", 5, 6, t.peeks() );
		DebugLayer::Pop();

		DebugLayer::Push( "Position" );
		DebugLayer::Text( "%.2f, %.2f", cs, sn );
		DebugLayer::Pop();

		pRenderer2D->Render();
	}
private:
	std::unique_ptr<Renderer2D> pRenderer2D;
	color c = Colors::Red;
	float scale = 16.0f;
	float v = 1.0f;

	simple_timer t;

	vec2f vector;

	Mesh mesh;
};

class SandboxApp : public Application
{
public:
	SandboxApp() : Application( { L"sandbox" } ) {}

	virtual void OnCreate() override {
		PushLayer( "sandbox", new SandboxLayer );
	}
};

Application * CreateApplication() {
	return new SandboxApp;
}
