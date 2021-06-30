#include "ConsoleLayer.h"
#include <Fission/Core/Application.hh>
#include <Fission/Core/Console.hh>

namespace JetBrainsMonoTTF {
#include "Static Fonts/JetBrainsMono-Regular.inl"
}

namespace Fission {

	void ConsoleLayerImpl::OnCreate( FApplication * app )
	{
		m_CommandText.reserve( s_MaxCommandSize );
		m_pRenderer2D = (IFRenderer2D*)app->pEngine->GetRenderer("$internal2D");
		FontManager::SetFont( "$console", JetBrainsMonoTTF::data, JetBrainsMonoTTF::size, 14.0f, app->pGraphics );
		m_pFont = FontManager::GetFont( "$console" );
		m_FontSize = m_pFont->GetSize();

		m_width = (float)app->pMainWindow->GetSwapChain()->GetSize().w;
	}

	void ConsoleLayerImpl::OnUpdate(timestep dt)
	{
		if( m_bShow )
		{
			m_pRenderer2D->SelectFont( m_pFont );

			extend += ( 200.0f - extend ) * dt * extend_rate;

			// Draw Background
			m_pRenderer2D->FillRectGrad( base::rectf( 0.0f, m_width, 0.0f, extend ), Colors::Black, Colors::Black, color( 0.7f ), color( 0.7f ) );
			m_pRenderer2D->FillRect( base::rectf( 0.0f, m_width, extend - m_FontSize - m_BottomPadding * 3.0f, extend ), Colors::make_gray( 0.2f, 0.3f ) );
			m_pRenderer2D->FillRect( base::rectf( 0.0f, m_width, extend - 1.0f, extend + 1.0f ), Colors::White );

			// Draw Console Buffer
			float top = extend - m_FontSize * 2.0f - m_BottomPadding * 2.0f;

			int maxIndex = Console::GetLineCount() - 1;
			lineOffset = std::min( lineOffset, (uint32_t)maxIndex );
			for( int i = maxIndex - (int)lineOffset; i >= 0; i-- )
			{
				string line;
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
				const float space = m_width / 20.0f;
				float offset = space / 2.0f;

				for( int i = 0; i < 20; i++ )
				{
					m_pRenderer2D->DrawString( L"^", base::vector2f( offset, extend - m_FontSize * 2.0f - m_BottomPadding ), Colors::White );
					offset += space;
				}
			}


			constexpr const char * s_DefText = "Enter a command . . .";

			auto tl = m_pRenderer2D->DrawString( "] ", base::vector2f(8.0f, extend - m_FontSize - m_BottomPadding - 1.0f), Colors::White );

			if( m_CommandText.empty() ) {
				m_pRenderer2D->DrawString( s_DefText, base::vector2f( 8.0f + tl.width, extend - m_FontSize - m_BottomPadding - 1.0f ), Colors::Gray );
			} else {
				m_BlinkPosition += 1.5f * dt;
				if( m_BlinkPosition >= 2.0f )
					m_BlinkPosition = 0.0f;

				if( m_BlinkPosition < 1.0f )
				{
					auto cmd_tl = m_pRenderer2D->CreateTextLayout( m_CommandText.c_str() );
					auto left = 9.0f + tl.width + cmd_tl.width;
					m_pRenderer2D->FillRect( base::rectf( left, 6.0f + left, extend - 7.0f, extend - 5.0f ), Colors::White );
				}

				m_pRenderer2D->DrawString( m_CommandText.c_str(), base::vector2f( 8.0f + tl.width, extend - m_FontSize - m_BottomPadding - 1.0f ), Colors::White );
			}

			m_pRenderer2D->Render();
		}
	}

	EventResult ConsoleLayerImpl::OnKeyDown( KeyDownEventArgs & args )
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

	EventResult ConsoleLayerImpl::OnTextInput( TextInputEventArgs & args )
	{
		if( m_bShow )
		{
			switch( args.codepoint )
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
				{
					m_BlinkPosition = 0.0f;
					m_CommandText.pop_back();
				}
				break;
			}
			default:
				if( m_CommandText.size() < s_MaxCommandSize )
				{
					m_BlinkPosition = 0.0f;
					m_CommandText += string( 1, (char)args.codepoint );
				}
				break;
			}
			return FISSION_EVENT_HANDLED;
		}
		return FISSION_EVENT_PASS;
	}

	EventResult ConsoleLayerImpl::OnResize( ResizeEventArgs & args )
	{
		m_width = (float)args.size.w;
		return FISSION_EVENT_PASS;
	}

	void ConsoleLayerImpl::_hide()
	{
		m_bShow = false;
	}

	void ConsoleLayerImpl::Destroy()
	{
		delete this;
	}

}
