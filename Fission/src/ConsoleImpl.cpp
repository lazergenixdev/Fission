#include "ConsoleImpl.h"
#include <fstream>

#include "unfinished.h"

#define _Fission_Console_Log_Location L"console.log"

namespace Fission {

	namespace helpers {
		int wstrlen( const wchar_t * str ) {
			int len = 0;
		loop:
			switch( str[len++] )
			{
			case '\0':
			case '\n':
			case '\r':
				goto end;
			default:
				goto loop;
			}
		end:
			return len - 1;
		}
	}

	ConsoleImpl::ConsoleImpl()
	{
#ifdef FISSION_CREATE_CONSOLE_WINDOW
		{
			// Create Console Window
			AllocConsole();
			auto f = freopen( "CONOUT$", "w", stdout );
				 f = freopen( "CONOUT$", "w", stderr );

			// Get the name of the current process
			char buffer[MAX_PATH];
			GetModuleFileNameA( NULL, buffer, (DWORD)std::size( buffer ) );
			std::string sProcessName = file::path( buffer ).filename().string();

			// Set the title of the console window
			char title[100];
			sprintf( title, "Debug Console - %s", sProcessName.c_str() );
			SetConsoleTitleA( title );
		}
#endif /* FISSION_CREATE_CONSOLE_WINDOW */

		{ // Clear contexts of debug log for writing
			auto f = std::wofstream( _Fission_Console_Log_Location, std::ios::trunc );

			f << L"===== " _Fission_Console_Log_Location L" =====\n\n";
		}

		m_CharacterBuffer = (wchar_t *)_aligned_malloc( m_BufferCapacity * sizeof( wchar_t ), 16u );
		m_Lines.reserve( 200 );

		/* Default Console commands */

		RegisterCommand(
			L"help",
			[] ( std::wstring s ) -> std::wstring {
				return L"There shall be NO help! >:)";
			}
		);

		RegisterCommand(
			L"cls",
			[] ( std::wstring ) {
				Console::Clear();
				return std::wstring();
			}
		);

		RegisterCommand(
			L"msg",
			[] ( std::wstring s ) {
				Console::WriteLine( s.c_str(), Colors::White );
				return std::wstring();
			}
		);
	}

	ConsoleImpl::~ConsoleImpl()
	{
		{
			auto f = std::ofstream( _Fission_Console_Log_Location, std::ios::app );

			f << "Process Terminated\n";
		}

		if( m_CharacterBuffer )
			_aligned_free( m_CharacterBuffer );
	}

	void ConsoleImpl::SetEnabled( bool _Enable ) {
		m_bEnabled = _Enable;
	}

	bool ConsoleImpl::IsEnabled()
	{
		return m_bEnabled;
	}

	void ConsoleImpl::Clear()
	{
		std::scoped_lock lock( m_WriteMutex );

		memset( m_CharacterBuffer, 0, m_BufferCount * sizeof( wchar_t ) );
		m_BufferCount = 0u;
		m_Lines.clear();
	}

	void ConsoleImpl::RegisterCommand( const std::wstring & _Command_Name, CommandCallback _Callback )
	{
		m_CommandMap.emplace( _Command_Name, _Callback );
	}

	void ConsoleImpl::UnregisterCommand( const std::wstring & _Command_Name )
	{
		m_CommandMap.erase( _Command_Name );
	}

	void ConsoleImpl::ExecuteCommand( const std::wstring & _Command )
	{
		auto begin_parameters = _Command.find_first_of( ' ' );

		std::wstring name = _Command.substr( 0u, begin_parameters );

		auto itt = m_CommandMap.find( name );
		if( itt == m_CommandMap.end() )
		{
			std::wstring comment = L"Unknown command: '" + name + L"'";
			WriteLine( Colors::DarkGray, comment.c_str() );
			return;
		}

		std::wstring out;

		if( begin_parameters < _Command.size() )
			out = itt->second( _Command.substr( begin_parameters + 1 ) );
		else
			out = itt->second( L"" );

		if( !out.empty() )
			WriteLine( Colors::DarkGray, out.c_str() );
	}

	void ConsoleImpl::WriteLine( color _Color, const wchar_t * _Text )
	{
		std::scoped_lock lock( m_WriteMutex );

		m_Lines.emplace_back( m_BufferCount, _Color );

		uint32_t len = (uint32_t)helpers::wstrlen( _Text );

		auto f = std::wofstream( _Fission_Console_Log_Location, std::ios::app );
		f.write( _Text, len );
		f.write( L"\n", 1u );
		f.close();

#if LAZER_CREATE_CONSOLE_WINDOW
		printf( "%ws\n", std::wstring( _Text, len ).c_str() );
#endif

		reserve( m_BufferCount + len );
		uint32_t start = m_BufferCount;
		m_BufferCount += len;
		memcpy( m_CharacterBuffer + start, _Text, len * sizeof( wchar_t ) );
	}

	void ConsoleImpl::Write( color _Color, const wchar_t * _Text )
	{
		_lazer_throw_not_implemented;
	}

	bool ConsoleImpl::GetLine( int _Line_Number, std::wstring * _Output_Text, color * _Output_Color )
	{
		if( _Line_Number >= m_Lines.size() )
			return false;

		const TextLine & Start = m_Lines[_Line_Number];

		const wchar_t * str = m_CharacterBuffer + Start.start;
		uint32_t length = m_BufferCount - Start.start;

		if( (size_t)_Line_Number + 1 < m_Lines.size() ) {
			length = m_Lines[(size_t)_Line_Number + 1].start - Start.start;
		}

		*_Output_Text = std::wstring( str, (size_t)length );
		if( _Output_Color ) *_Output_Color = Start.color;

		return true;
	}

	int ConsoleImpl::GetLineCount()
	{
		return (int)m_Lines.size();
	}

	ConsoleImpl & ConsoleImpl::Get()
	{
		static ConsoleImpl _Console_Instance;
		return _Console_Instance;
	}

	void ConsoleImpl::reserve( uint32_t _New_Count )
	{
		if( _New_Count > m_BufferCapacity )
		{
			while( _New_Count > m_BufferCapacity )
				m_BufferCapacity += 2000u;
			wchar_t * temp = (wchar_t *)_aligned_malloc( m_BufferCapacity * sizeof( wchar_t ), 16u );

			if( temp == nullptr )
				throw 0x45;

			memcpy( temp, m_CharacterBuffer, m_BufferCount * sizeof( wchar_t ) );
			_aligned_free( m_CharacterBuffer );
			m_CharacterBuffer = temp;
		}
	}
}
