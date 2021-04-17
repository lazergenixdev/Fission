#include "ConsoleLayer.h"
#include "Fission/Core/Console.h"
#include "Fission/Core/Application.h"

namespace JetBrainsMonoTTF {
#include "Static Fonts/JetBrainsMono-Regular.inl"
}

namespace Fission {

	void ConsoleLayer::OnCreate()
	{
		m_CommandText.reserve( s_MaxCommandSize );
		m_pRenderer2D = Renderer2D::Create( GetApp()->GetGraphics() );
		FontManager::SetFont( "$console", JetBrainsMonoTTF::data, JetBrainsMonoTTF::size, 8.0f );
		auto pFont = FontManager::GetFont( "$console" );
		m_pRenderer2D->SelectFont( pFont );
		m_FontSize = pFont->GetSize();
	}

	void ConsoleLayer::OnUpdate()
	{
		float dt = t.gets();
		if( m_bShow )
		{
			extend += ( 200.0f - extend ) * dt * extend_rate;

			//vec2f size = vec2f( 1280.0f, 720.0f ); // todo: this is a bug, please fix as soon as possible
			vec2f size = vec2f( 1920.0f, 720.0f ); // todo: this is a bug, please fix as soon as possible

			// Draw Background
			m_pRenderer2D->FillRectGrad( rectf( 0.0f, size.x, 0.0f, extend ), Colors::Black, Colors::Black, color( 0x0, 0.7f ), color( 0x0, 0.7f ) );
			m_pRenderer2D->FillRect( rectf( 0.0f, size.x, extend - m_FontSize - m_BottomPadding * 3.0f, extend ), color( 0x0, 0.40f ) );
			m_pRenderer2D->FillRect( rectf( 0.0f, size.x, extend - 1.0f, extend + 1.0f ), Colors::White );

			// Draw Console Buffer
			float top = extend - m_FontSize * 2.0f - m_BottomPadding * 2.0f;

			int maxIndex = Console::GetLineCount() - 1;
			lineOffset = std::min( lineOffset, (uint32_t)maxIndex );
			for( int i = maxIndex - (int)lineOffset; i >= 0; i-- )
			{
				std::wstring line;
				color col;
				Console::GetLine( i, &line, &col );

				auto LineLayout = m_pRenderer2D->CreateTextLayout( line.c_str() );
				const float new_top = top - LineLayout.height;

				m_pRenderer2D->DrawString( line.c_str(), { 5.0f, new_top }, col );

				if( new_top <= 0.0f ) // Only Render Text Visible
					break;

				top = new_top;
			}

			if( lineOffset != 0 )
			{
				auto car = m_pRenderer2D->CreateTextLayout( L"^" );

				const float width = car.width;
				const float space = size.x / 20.0f;
				float offset = space / 2.0f;

				for( int i = 0; i < 20; i++ )
				{
					m_pRenderer2D->DrawString( L"^", vec2( offset, extend - m_FontSize * 2.0f - m_BottomPadding ), Colors::White );
					offset += space;
				}
			}



			//float end = (float)m_CommandText.length();
			//m_pRenderer2D->DrawRect( rectf( end * 6.6f + 22.0f, end * 6.6f + 29.0f, bottom - 7.0f, bottom - 5.0f ), color::White );

			std::wstring consoleInput = L"] ";

			consoleInput += m_CommandText;

			m_pRenderer2D->DrawString( consoleInput.c_str(), vec2f( 8.0f, extend - m_FontSize - m_BottomPadding - 1.0f ), Colors::White );

			if( m_CommandText.empty() )
			{
				m_pRenderer2D->DrawString( L"  Enter a command . . .", vec2f( 8.0f, extend - m_FontSize - m_BottomPadding - 1.0f ), Colors::Gray );
			}

			m_pRenderer2D->Render();
		}
	}

	EventResult ConsoleLayer::OnKeyDown( KeyDownEventArgs & args )
	{
		if( Console::IsEnabled() )
		{
			switch( args.key )
			{
			case Keys::Accent:
			{
				if( m_bShow ) {
					_hide();
				}
				else {
					m_CommandText.clear();
					extend = 0.0f;
					lineOffset = 0;
					m_bShow = true;
				}
				return FISSION_EVENT_HANDLED;
			}
			case Keys::Up:
			case Keys::Mouse_WheelUp:
			{
				if( m_bShow )
					lineOffset++;
				break;
			}
			case Keys::Down:
			case Keys::Mouse_WheelDown:
			{
				if( m_bShow && ( lineOffset > 0 ) )
					lineOffset--;
				break;
			}
			case Keys::Escape:
			{
				if( m_bShow ) {
					_hide();
					return FISSION_EVENT_HANDLED;
				}
				break;
			}
			default:
				break;
			}
		}
		return ( m_bShow ) ? FISSION_EVENT_HANDLED : FISSION_EVENT_PASS;
	}

	EventResult ConsoleLayer::OnTextInput( TextInputEventArgs & args )
	{
		if( m_bShow )
		{
			switch( args.character )
			{
			case '`':
			case '~':
				return FISSION_EVENT_HANDLED;
			case '\r':
			{
				if( !m_CommandText.empty() )
				{
					Console::ExecuteCommand( m_CommandText );
					m_CommandText.clear();
					lineOffset = 0u;
				}
				break;
			}
			case '\b':
			{
				if( !m_CommandText.empty() )
					m_CommandText.pop_back();
				break;
			}
			default:
				if( m_CommandText.size() < s_MaxCommandSize )
					m_CommandText += args.character;
				break;
			}
			return FISSION_EVENT_HANDLED;
		}
		return FISSION_EVENT_PASS;
	}

	void ConsoleLayer::_hide()
	{
		m_bShow = false;
	}

}
