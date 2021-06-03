#include "DebugLayer.h"
#include "Fission/Core/Application.h"
#include "../Version.h"
#include "Fission/Core/Console.h"
#include "Fission/Base/Rect.h"

// todo: move system info gathering to Fission::System

namespace RobotoRegularTTF {
#include "Static Fonts/Roboto-Regular.inl"
}

using namespace Fission;

static DebugLayerImpl * s_DebugLayer = nullptr;


void DebugLayer::Push( const char * name )
{
	s_DebugLayer->Push( name );
}

void DebugLayer::Pop()
{
	s_DebugLayer->Pop();
}

void DebugLayer::Text( const char * what )
{
	s_DebugLayer->Text( what );
}


DebugLayerImpl::DebugLayerImpl()
{
	s_DebugLayer = this;
}

void DebugLayerImpl::RegisterDrawCallback( const char * _Key, DebugDrawCallback _Callback )
{
	m_DrawCallbacks.emplace( _Key, _Callback );
}

void DebugLayerImpl::Push( const char * name )
{
	std::string key = name;

	auto it = m_InfoMap.find( key );
	if( it == m_InfoMap.end() )
	{
		auto it = m_InfoMap.emplace( key, std::vector<std::string>() );

		m_CurrentInfo = &it.first->second;
		m_CurrentInfo->emplace_back( "[" + key + "]" );
		return;
	}

	m_CurrentInfo = &it->second;
	m_CurrentInfo->erase( m_CurrentInfo->begin() + 1, m_CurrentInfo->end() );
}

void DebugLayerImpl::Pop()
{
	m_CurrentInfo = nullptr;
}

void DebugLayerImpl::Text( const char * what )
{
	FISSION_ASSERT( m_CurrentInfo, "You must Push before Text() friendo" );

	m_CurrentInfo->emplace_back( what );
}

static std::string cpu_name;
static std::string memory_str;
static std::string platform_str;
#include <intrin.h>

typedef LONG NTSTATUS;

typedef NTSTATUS( WINAPI * RtlGetVersionPtr )( PRTL_OSVERSIONINFOW );

RTL_OSVERSIONINFOW GetRealOSVersion() {
	HMODULE hMod = ::GetModuleHandleW( L"ntdll.dll" );
	if( hMod ) {
		RtlGetVersionPtr fxPtr = ( RtlGetVersionPtr )::GetProcAddress( hMod, "RtlGetVersion" );
		if( fxPtr != nullptr ) {
			RTL_OSVERSIONINFOW rovi = { 0 };
			rovi.dwOSVersionInfoSize = sizeof( rovi );
			if( 0 == fxPtr( &rovi ) ) {
				return rovi;
			}
		}
	}
	RTL_OSVERSIONINFOW rovi = { 0 };
	return rovi;
}

void DebugLayerImpl::OnCreate() {
	m_pRenderer2D = Renderer2D::Create( GetApp()->GetGraphics() );

	FontManager::SetFont( "$debug", RobotoRegularTTF::data, RobotoRegularTTF::size, 12.0f );

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

	auto ver = GetRealOSVersion();

	sprintf( buf, "Windows 10 64-bit (%u.%u.%u)", ver.dwMajorVersion, ver.dwMinorVersion, ver.dwBuildNumber );

	platform_str = buf;
}

void DebugLayerImpl::OnUpdate() {
	
	static constexpr auto c = color( 0.0f, 0.0f, 0.0f, 0.7f );
	base::vector2f size = { 1280.0f, 720.0f }; // todo: this is a bug, please fix as soon as possible

	auto pFont = FontManager::GetFont( "$debug" );
	float diff = pFont->GetSize();
	m_pRenderer2D->SelectFont( pFont );
	m_LastFrameTimes[m_FrameCount % std::size( m_LastFrameTimes )] = t.getms();

	float msAvgFrameTime = [&, sum = 0.0f]()mutable{ for( auto && f : m_LastFrameTimes ) sum += f; return sum; }( ) / (float)std::size( m_LastFrameTimes );

	if( m_bShow )
	{
		wchar_t buf[32];
		memset( buf, 0, sizeof buf );
		swprintf( buf, std::size( buf ), L"%.1f FPS (%.2fms)", 1000.0f / msAvgFrameTime, msAvgFrameTime );

		auto tl = m_pRenderer2D->CreateTextLayout( L"Fission v" FISSION_VERSION_STRING " - Debug Layer" );

		m_pRenderer2D->FillRect( base::rectf( 0.0f, tl.width, 0.0f, diff ), c );

		m_pRenderer2D->DrawString( L"Fission v" FISSION_VERSION_STRING " - Debug Layer", { 0.0f, 0.0f }, Colors::White );

		m_pRenderer2D->DrawString( L"(F3)", { tl.width + 4.0f, 0.0f }, color( Colors::White, 0.5f ) );

		{ // FPS
			tl = m_pRenderer2D->CreateTextLayout( buf );
			base::vector2f pos = { 0.0f, diff };
			m_pRenderer2D->FillRect( base::rectf::from_topleft( pos, tl.width, diff ), c );			m_pRenderer2D->DrawString( buf, pos, Colors::White );
		}
		
		{ // CPU
			tl = m_pRenderer2D->CreateTextLayout( cpu_name.c_str() );
			base::vector2f pos = { size.x - tl.width, 0.0f };
			m_pRenderer2D->FillRect( base::rectf::from_topleft( pos, tl.width, diff ), c );
			m_pRenderer2D->DrawString( cpu_name.c_str(), pos, Colors::White );
		}

		{ // Memory
			tl = m_pRenderer2D->CreateTextLayout( memory_str.c_str() );
			base::vector2f pos = { size.x - tl.width, diff };
			m_pRenderer2D->FillRect( base::rectf::from_topleft( pos, tl.width, diff ), c );
			m_pRenderer2D->DrawString( memory_str.c_str(), pos, Colors::White );
		}

		{ // Platform
			tl = m_pRenderer2D->CreateTextLayout( platform_str.c_str() );
			base::vector2f pos = { size.x - tl.width, diff * 2.0f };
			m_pRenderer2D->FillRect( base::rectf::from_topleft( pos, tl.width, diff ), c );
			m_pRenderer2D->DrawString( platform_str.c_str(), pos, Colors::White );
		}

		float start = 80.0f;

		for( auto && [key,text] : m_InfoMap )
		{
			for( auto && s : text )
			{
				auto tl = m_pRenderer2D->CreateTextLayout( s.c_str() );

				m_pRenderer2D->FillRect( base::rectf( 0.0f, tl.width, start, start + tl.height ), c );
				m_pRenderer2D->DrawString( s.c_str(), { 0.0f, start }, Colors::White );

				start += tl.height;
			}
			start += 30.0f;
		}

		m_pRenderer2D->Render();
	}

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
