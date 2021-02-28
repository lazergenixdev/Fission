#include "DebugLayer.h"
#include "Fission/Core/Application.h"
#include "../Version.h"

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

void DebugLayerImpl::OnCreate() {
	m_pRenderer2D = Renderer2D::Create( GetApp()->GetGraphics() );

	FontManager::SetFont( "$debug", RobotoRegularTTF::data, RobotoRegularTTF::size, 8.0f );
}

void DebugLayerImpl::OnUpdate() {

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



		auto tl = m_pRenderer2D->CreateTextLayout( L"Fission-v" FISSION_VERSION_STRING " - Debug Layer" );

		m_pRenderer2D->FillRect( rectf( 0.0f, tl.width, 0.0f, 2.0f * diff ), color( Colors::Black, 0.5f ) );

		m_pRenderer2D->DrawString( L"Fission-v" FISSION_VERSION_STRING " - Debug Layer", { 0.0f, 0.0f }, Colors::White );

		m_pRenderer2D->DrawString( L"(F3)", { tl.width + 4.0f, 0.0f }, color( 0xFFFFFF, 0.5f ) );

		m_pRenderer2D->DrawString( buf, { 0.0f, diff }, Colors::White );

		float start = 50.0f;

		for( auto && [key,text] : m_InfoMap )
		{
			for( auto && s : text )
			{
				auto tl = m_pRenderer2D->CreateTextLayout( s.c_str() );

				m_pRenderer2D->FillRect( rectf( 0.0f, tl.width, start, start + tl.height ), color( Colors::Black, 0.5f ) );
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
