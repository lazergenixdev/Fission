#include "DebugLayer.h"
#include <Fission/Core/Application.hh>
#include <Fission/Core/Console.hh>
#include <Fission/Platform/System.h>
#include <Fission/Base/Rect.h>
#include "../Version.h"

#include <dxgi.h>
#pragma comment(lib,"dxgi")

#ifdef FISSION_DIST
#define _FISSION_BUILD_STRING
#else
#define _FISSION_BUILD_STRING " (dev build)"
#endif

// todo: move system info gathering to Fission::System

namespace IBMPlexMono_MediumTTF {
#include "Static Fonts/IBMPlexMono-Medium.inl"
}

using namespace Fission;

static DebugLayerImpl * s_DebugLayer = nullptr;


DebugLayerImpl::DebugLayerImpl()
{
	s_DebugLayer = this;
	m_UserInfo.reserve(100);
}

void Fission::DebugLayerImpl::SetAppVersionString( const string & version_String )
{
	m_AppVersionString = version_String;
}

void DebugLayerImpl::RegisterDrawCallback( const char * _Key, DrawCallback _Callback, void * _UserData )
{
	m_DrawCallbacks.emplace( _Key, DrawCallbackData{ _Callback, _UserData } );
}

void DebugLayerImpl::Text( const char * what )
{
	m_UserInfo.emplace_back( what );
}

static std::string cpu_name;
static std::string memory_str;
static std::string gpu_name;
#include <intrin.h>

void DebugLayerImpl::Destroy() { FontManager::DelFont( "$debug" ); delete this; }

void DebugLayerImpl::OnCreate(class FApplication * app) {
	pRenderer2D = (IFRenderer2D*)app->pEngine->GetRenderer("$internal2D");

	m_width = (float)app->pMainWindow->GetSwapChain()->GetSize().w;

	FontManager::SetFont( "$debug", IBMPlexMono_MediumTTF::data, IBMPlexMono_MediumTTF::size, 18.0f, app->pGraphics );

	int CPUInfo[4] = { -1 };
	unsigned   nExIds, i = 0;
	char CPUBrandString[0x40] = {};
	// Get the information associated with each extended ID.
	__cpuid( CPUInfo, 0x80000000 );
	nExIds = CPUInfo[0];
	for( i = 0x80000000; i <= nExIds; ++i )
	{
		__cpuid( CPUInfo, i );
		// Interpret CPU brand string
		if( i == 0x80000002 )
			memcpy( CPUBrandString, CPUInfo, sizeof( CPUInfo ) );
		else if( i == 0x80000003 )
			memcpy( CPUBrandString + 16, CPUInfo, sizeof( CPUInfo ) );
		else if( i == 0x80000004 )
			memcpy( CPUBrandString + 32, CPUInfo, sizeof( CPUInfo ) );
	}
	//string includes manufacturer, model and clockspeed
	cpu_name = (const char *)CPUBrandString;
	for( size_t i = cpu_name.size()-1; i > 0u; --i )
		if( cpu_name[i] != ' ' )
		{ cpu_name.resize(i+1); break; }

	//SYSTEM_INFO sysInfo;
	//GetSystemInfo( &sysInfo );
	//cout << "Number of Cores: " << sysInfo.dwNumberOfProcessors << endl;

	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof( statex );
	GlobalMemoryStatusEx( &statex );
	char buf[128];
	sprintf( buf, "%.1f", float( ( statex.ullTotalPhys / 1024 ) / ( 1024 * 1000 ) ) );
	memory_str = "Total System Memory: " + std::string(buf) + "GB";

	Platform::com_ptr<IDXGIFactory> pFactory = NULL;
	CreateDXGIFactory( IID_PPV_ARGS( &pFactory ) );

	Platform::com_ptr<IDXGIAdapter> pAdapter = NULL;
	pFactory->EnumAdapters( 0, &pAdapter );

	DXGI_ADAPTER_DESC descAdapter;
	pAdapter->GetDesc( &descAdapter );

	gpu_name = utf16_string( (char16_t*)descAdapter.Description ).utf8().string();
}

#define _DEBUG_LAYER FISSION_ENGINE " " FISSION_FULL_VERSION_STRING _FISSION_BUILD_STRING

void DebugLayerImpl::OnUpdate(timestep dt) {
	
	static constexpr auto c = color( 0.0f, 0.0f, 0.0f, 0.7f );

	m_LastFrameTimes[m_FrameCount % std::size( m_LastFrameTimes )] = dt.milliseconds();

	float msAvgFrameTime = [&, sum = 0.0f]()mutable{ for( auto && f : m_LastFrameTimes ) sum += f; return sum; }( ) / (float)std::size( m_LastFrameTimes );

	if( m_bShow )
	{
		char sFPS[32];
		memset( sFPS, 0, sizeof sFPS );
		sprintf_s( sFPS, "%.1f FPS (%.2fms)", 1000.0f / msAvgFrameTime, msAvgFrameTime );

		auto pFont = FontManager::GetFont( "$debug" );
		pRenderer2D->SelectFont( pFont );

		float offsety = 0.0f;

		auto tl = pRenderer2D->CreateTextLayout( _DEBUG_LAYER );
		pRenderer2D->FillRect( { m_width - tl.width, m_width, 0.0f, tl.height }, c );
		pRenderer2D->DrawString( _DEBUG_LAYER, { m_width - tl.width, 0.0f }, Colors::White );
	//	pRenderer2D->DrawString( L"(F3)", { tl.width + 4.0f, offsety }, color( Colors::White, 0.5f ) );
		offsety += tl.height;

		{ // Platform
			tl = pRenderer2D->CreateTextLayout( System::GetVersionString() );
			base::vector2f pos = { m_width - tl.width, offsety };
			pRenderer2D->FillRect( { m_width - tl.width, m_width, offsety, offsety + tl.height }, c );
			pRenderer2D->DrawString( System::GetVersionString(), pos, Colors::White );
			offsety += tl.height;
		}

		offsety += tl.height * 1.5f;
		
		{ // CPU
			tl = pRenderer2D->CreateTextLayout( cpu_name.c_str() );
			base::vector2f pos = { m_width - tl.width, offsety };
			pRenderer2D->FillRect( { m_width - tl.width, m_width, offsety, offsety + tl.height }, c );
			pRenderer2D->DrawString( cpu_name.c_str(), pos, Colors::White );
			offsety += tl.height;
		}

		//{ // Memory
		//	tl = pRenderer2D->CreateTextLayout( memory_str.c_str() );
		//	base::vector2f pos = { m_width - tl.width, offsety };
		//	pRenderer2D->FillRect( base::rectf::from_topleft( pos, tl.width, tl.height ), c );
		//	pRenderer2D->DrawString( memory_str.c_str(), pos, Colors::White );
		//	offsety += tl.height;
		//}
		 
		{ // GPU
			tl = pRenderer2D->CreateTextLayout( gpu_name.c_str() );
			base::vector2f pos = { m_width - tl.width, offsety };
			pRenderer2D->FillRect( { m_width - tl.width, m_width, offsety, offsety + tl.height }, c );
			pRenderer2D->DrawString( gpu_name.c_str(), pos, Colors::White );
			offsety += tl.height;
		}

		offsety = 0.0f;

		// App Version
		tl = pRenderer2D->CreateTextLayout( m_AppVersionString.c_str() );
		pRenderer2D->FillRect( base::rectf( 0.0f, tl.width, offsety, offsety + tl.height ), c );
		pRenderer2D->DrawString( m_AppVersionString.c_str(), { 0.0f, offsety }, Colors::White );
		offsety += tl.height;

		// FPS
		tl = pRenderer2D->CreateTextLayout( sFPS );
		pRenderer2D->FillRect( base::rectf( 0.0f, tl.width, offsety, offsety + tl.height ), c );
		pRenderer2D->DrawString( sFPS, { 0.0f, offsety }, Colors::White );
		offsety += tl.height;

		for( auto && text : m_UserInfo )
		{
			auto tl = pRenderer2D->CreateTextLayout( text.c_str() );

			pRenderer2D->FillRect( base::rectf( 0.0f, tl.width, offsety, offsety + tl.height ), c );
			pRenderer2D->DrawString( text.c_str(), { 0.0f, offsety }, Colors::White );

			offsety += tl.height;
		}

		pRenderer2D->Render();
	}

	m_UserInfo.clear();
	m_FrameCount++;
}

EventResult DebugLayerImpl::OnKeyDown( KeyDownEventArgs & args )
{
	if( args.key == Keys::Key::F3 )
	{
		m_bShow = !m_bShow;
		return FISSION_EVENT_HANDLED;
	}
	if( args.key == Keys::Key::F9 )
	{
		float m_AvgFrameTime = [&, sum = 0.0f]()mutable{ for( auto && f : m_LastFrameTimes ) sum += f; return sum; }() / (float)std::size( m_LastFrameTimes );
		wchar_t buf[30];
		memset( buf, 0, sizeof buf );
		swprintf( buf, std::size( buf ), L"%.1f FPS\n", 1000.0f / m_AvgFrameTime );
		OutputDebugStringW( buf );
	}
	return FISSION_EVENT_PASS;
}

EventResult DebugLayerImpl::OnResize( ResizeEventArgs & args )
{
	m_width = (float)args.size.w;
	return FISSION_EVENT_PASS;
}
