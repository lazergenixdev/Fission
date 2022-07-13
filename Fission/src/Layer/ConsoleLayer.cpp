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
		m_pRenderer2D = (IFRenderer2D*)app->f_pEngine->GetRenderer("$internal2D");
		FontManager::SetFont( "$console", JetBrainsMonoTTF::data, JetBrainsMonoTTF::size, 14.0f, app->f_pGraphics );
		m_pFont = FontManager::GetFont( "$console" );
		m_FontSize = m_pFont->GetSize();

		m_width = (float)app->f_pMainWindow->GetSwapChain()->GetSize().w;
	}

	void ConsoleLayerImpl::OnUpdate(timestep dt)
	{
		if( m_bShow )
		{
			if( !Console::IsEnabled() )
			{
				_hide();
				return;
			}

			m_pRenderer2D->SelectFont( m_pFont );

			extend += ( 200.0f - extend ) * dt * extend_rate;

			// Draw Background
			m_pRenderer2D->FillRectGrad( rf32( 0.0f, m_width, 0.0f, extend ), Colors::Black, Colors::Black, color( 0.7f ), color( 0.7f ) );
			m_pRenderer2D->FillRect( rf32( 0.0f, m_width, extend - m_FontSize - m_BottomPadding * 3.0f, extend ), Colors::make_gray( 0.11f, 0.5f ) );
			m_pRenderer2D->FillRect( rf32( 0.0f, m_width, extend - 1.0f, extend + 1.0f ), Colors::White );

			// Draw Console Buffer
			float top = extend - m_FontSize * 2.0f - m_BottomPadding;


			int maxIndex = Console::GetLineCount() - 1;
			lineOffset = std::min( lineOffset, (uint32_t)maxIndex );

		//	m_pRenderer2D->FillRect( base::rectf( 0.0f, m_width, top, top + 1.0f ), Colors::White ); //debug

			int numLinesVisible = 0;
			float start = top;
			for( ; start > 0.0f ; start -= m_FontSize, ++numLinesVisible );
			if( numLinesVisible > 0 )
			{
				int bottomLine = Console::GetLineCount() - lineOffset;
				int startLine = bottomLine - numLinesVisible;

				auto cursor = v2f32{ m_LeftPadding, start };
				if( startLine < 0 )
				{
					cursor.y -= (float)startLine * m_FontSize;
					startLine = 0;
				}

				numLinesVisible = std::min( bottomLine, numLinesVisible );

				for( auto [view, color, newlines] : console_iterator{ startLine } )
				{
					if( view.empty() ) [[unlikely]]
						cursor.y -= m_FontSize * (float)newlines;

					else [[likely]]
					{
						auto tl = m_pRenderer2D->DrawString( view, cursor, color );

						if( newlines )
						{
							cursor.y += m_FontSize * (float)newlines;
							cursor.x = m_LeftPadding;
						}
						else
							cursor.x += tl.width;
					}

					numLinesVisible -= newlines;

				//	if( numLinesVisible <= 0 ) break;
					if( cursor.y > top - m_FontSize ) break; // Only Render Text Visible
				}
			}


			if( lineOffset != 0 )
			{
				auto car = m_pRenderer2D->CreateTextLayout( "^" );

				const float width = car.width;
				const float space = m_width / 20.0f;
				float offset = space / 2.0f;

				for( int i = 0; i < 20; i++ )
				{
					m_pRenderer2D->DrawString( "^", v2f32( offset, extend - m_FontSize * 2.0f - m_BottomPadding ), Colors::White );
					offset += space;
				}
			}


			constexpr const char * s_DefText = "Enter a command . . .";

			auto tl = m_pRenderer2D->DrawString( "$ ", v2f32(m_LeftPadding, extend - m_FontSize - m_BottomPadding - 1.0f), Colors::White );

			if( m_CommandText.empty() ) {
				m_pRenderer2D->DrawString( s_DefText, v2f32( 8.0f + tl.width, extend - m_FontSize - m_BottomPadding - 1.0f ), Colors::Gray );
			} else {
				m_BlinkPosition += 1.5f * dt;
				if( m_BlinkPosition >= 2.0f )
					m_BlinkPosition = 0.0f;

				if( m_BlinkPosition < 1.0f )
				{
					auto cmd_tl = m_pRenderer2D->CreateTextLayout( m_CommandText.c_str(), m_CursorPosition );
					auto left = 9.0f + tl.width + cmd_tl.width;
					m_pRenderer2D->FillRect( rf32( left, 1.0f + left, extend - m_FontSize - m_BottomPadding - 1.0f, extend - m_BottomPadding - 1.0f ), Colors::White );
				}

				m_pRenderer2D->DrawString( m_CommandText.c_str(), v2f32( 8.0f + tl.width, extend - m_FontSize - m_BottomPadding - 1.0f ), Colors::White );
			}

			m_pRenderer2D->Render();
		}
	}

	EventResult ConsoleLayerImpl::OnKeyDown( KeyDownEventArgs & args )
	{
		if( m_bShow )
		{
			switch( args.key )
			{
			case Keys::Accent: return FISSION_EVENT_HANDLED;
			case Keys::Mouse_WheelUp:
			{
				if( m_bShow )
					lineOffset++;
				break;
			}
			case Keys::Mouse_WheelDown:
			{
				if( m_bShow && ( lineOffset > 0 ) )
					lineOffset--;
				break;
			}
			case Keys::Up:
			{
				if( !m_History.empty() )
				{
					++m_LastHistoryIndex;
					if( m_LastHistoryIndex > (int)m_History.size() ) m_LastHistoryIndex = (int)m_History.size();

					auto & str = m_History[m_History.size() - size_t(m_LastHistoryIndex)];
					m_CommandText = str;
					m_CursorPosition = (int)m_CommandText.length();
				}
				break;
			}
			case Keys::Down:
			{
				if( !m_History.empty() )
				{
					--m_LastHistoryIndex;
					if( m_LastHistoryIndex < 1 ) m_LastHistoryIndex = 1;

					auto & str = m_History[m_History.size() - size_t(m_LastHistoryIndex)];
					m_CommandText = str;
					m_CursorPosition = (int)m_CommandText.length();
				}
				break;
			}
			case Keys::Left:
			{
				m_BlinkPosition = 0.0f;
				m_CursorPosition = std::max( m_CursorPosition - 1, 0 );
				break;
			}
			case Keys::Right:
			{
				m_BlinkPosition = 0.0f;
				m_CursorPosition = std::min( m_CursorPosition + 1, (int)m_CommandText.length() );
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
			return FISSION_EVENT_HANDLED;
		}
		return FISSION_EVENT_PASS;
	}

	EventResult ConsoleLayerImpl::OnTextInput( TextInputEventArgs & args )
	{
		if( m_bShow )
		{
			switch( args.codepoint )
			{
			case '\r':
			{
				if( !m_CommandText.empty() )
				{
					m_History.emplace_back( m_CommandText );
					Console::WriteLine( string{"$ " + m_CommandText.str()} );
					Console::ExecuteCommand( m_CommandText );
					m_CommandText.clear();
					m_CursorPosition = 0;
					m_LastHistoryIndex = 0;
					lineOffset = 0u;
				}
				break;
			}
			case '\b':
			{
				if( m_CursorPosition > 0 )
				{
					m_BlinkPosition = 0.0f;
					--m_CursorPosition;
					m_CommandText = m_CommandText.str().erase( m_CursorPosition, 1 );
				}
				break;
			}
			default:
				if( m_CommandText.size() < s_MaxCommandSize )
				{
					m_BlinkPosition = 0.0f;
					m_CommandText = m_CommandText.str().insert( m_CursorPosition, 1, (char)args.codepoint );
					++m_CursorPosition;
				}
				break;
			}
			return FISSION_EVENT_HANDLED;
		}
		else if( args.codepoint == U'`' && Console::IsEnabled() )
		{
			m_CommandText.clear();
			extend = 0.0f;
			lineOffset = 0;
			m_CursorPosition = 0;
			m_LastHistoryIndex = 0;
			m_bShow = true;
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
