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
		goto yes;

		arr.reserve( 100 );
		for( int i = 0; i < 20; i++ )
		{
			info.FillColor = ( rand() % 2 == 0 ) ? Colors::Red : Colors::Blue;
			info.Width = ( rand() % 2 == 0 ) ? ( 256 ) : ( 512 );
			info.Height = ( rand() % 2 == 0 ) ? ( 256 ) : ( 512 );
			arr.emplace_back( Surface::Create( info ) );
			put_border( arr.back().get() );

			char buf[100];
			sprintf( buf, "text%i", i );

			map.emplace( buf, arr.back().get() );
		}

		map.build();

		map.get()->Save( "Atlas.png" );

	yes:
		{
			metadata md;
			md["what"] = "nice";
			md["array thing"].resize(10);
			auto & arr = md["array thing"];
			for( int i = 0; i < (int)arr.size(); i++ )
				arr[i] = i;

			auto wstr = utf8_to_wstring( md["what"].as_string() );
			Console::Warning( L"%s", wstr.c_str() );
			Console::Warning( L"%i", md["array thing"][3].as_integer() );
			Console::Warning( L"%i", md["array thing"][4].as_integer() );
			Console::Warning( L"%i", md["array thing"][5].as_integer() );
		}

		metadata md;
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
