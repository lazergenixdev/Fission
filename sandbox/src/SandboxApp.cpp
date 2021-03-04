#include <Fission/Platform/EntryPoint.h>
#include <Fission/Fission.h>
#include <Fission/Core/UI/UI.h>
#include <Fission/Core/SurfaceMap.h>

#include <numbers>
#include <iostream>

#include "lazer/unfinished.h"

using namespace Fission;

void put_border( Surface * s )
{
	uint32_t w = s->width();
	uint32_t h = s->height();
	for( int i = 0; i < w; i++ )
		s->PutPixel( i, 0, Colors::White ), s->PutPixel( i, h - 1, Colors::White );
	for( int i = 0; i < h; i++ )
		s->PutPixel( 0, i, Colors::White ), s->PutPixel( w - 1, i, Colors::White );
}

static color blueOrRed() {return ( rand() % 2 == 0 ) ? Colors::Red : Colors::Blue;}
static const char * to_string( color c ) { return ( c.r == 1.0f ) ? "Red" : "Blue"; }

static constexpr bool is_prime( int n )
{
	if( n < 1 ) return false;
	if( n == 1 ) return true;
	if( n % 2 == 0 ) return false;
	for( int i = n / 2; i > 2; --i )
		if( n % i == 0 ) return false;
	return true;
}

class SandboxLayer : public ILayer 
{
public:
	SandboxLayer()
	{
	}
	virtual void OnCreate() override
	{
		surface_map map;
		std::vector<std::unique_ptr<Surface>> arr;
		Surface::CreateInfo info;

		arr.reserve( 100 );
		for( int i = 0; i < 20; i++ )
		{
			info.FillColor = blueOrRed();
			info.Width = ( rand() % 2 == 0 ) ? ( 256 ) : ( 512 );
			info.Height = ( rand() % 2 == 0 ) ? ( 256 ) : ( 512 );
			arr.emplace_back( Surface::Create( info ) );
			put_border( arr.back().get() );

			char buf[100];
			sprintf( buf, "{region name}%i", i );

			map.emplace( buf, arr.back().get() );
			map[buf]->meta["color"]["r"] = info.FillColor->r;
			map[buf]->meta["color"]["g"] = info.FillColor->g;
			map[buf]->meta["color"]["b"] = info.FillColor->b;
			map[buf]->meta["color"]["a"] = info.FillColor->a;
			map[buf]->meta["prime"] = is_prime(i);
			map[buf]->meta["even"] = (i%2==0);
		}

		metadata md;
		md["cat"] = "meow";
		md["glow"] = 9.46;
		map.set_metadata( md );

		map.build();

		map.Save( "assets/Atlas" );

		map.Load( "assets/Atlas" );

		if( map["{region name}11"]->meta["prime"].as_boolean() ) Console::Message( L"Success!!!" );
	}
	virtual void OnUpdate() override
	{
		GetApp()->Exit();
	}
private:
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
