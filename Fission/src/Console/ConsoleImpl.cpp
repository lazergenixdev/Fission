#include "ConsoleImpl.h"
#include <fstream>
#include <Fission/Base/Exception.h>

#define _Fission_Console_Log_Location "console.log"

namespace Fission {

	namespace helpers {
		int strlen( const char * str ) {
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
			std::string sProcessName = std::filesystem::path( buffer ).filename().string();

			// Set the title of the console window
			char title[100];
			sprintf( title, "Debug Console - %s", sProcessName.c_str() );
			SetConsoleTitleA( title );
		}
#endif /* FISSION_CREATE_CONSOLE_WINDOW */

		{ // Clear contexts of debug log for writing
			auto f = std::ofstream( _Fission_Console_Log_Location, std::ios::trunc );

			f << "===== " _Fission_Console_Log_Location " =====\n\n";
		}

		m_CharacterBuffer = (char *)_aligned_malloc( m_BufferCapacity, 16u );
		m_Lines.reserve( 200 );

		/* Default Console commands */

		RegisterCommand(
			"help",
			[] ( string s ) -> string {
				return "There shall be NO help! >:)";
			}
		);

		RegisterCommand(
			"cls",
			[] ( string ) {
				Console::Clear();
				return string();
			}
		);

		RegisterCommand(
			"msg",
			[] ( string s ) {
				Console::WriteLine( s.c_str(), Colors::White );
				return string();
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

	void ConsoleImpl::RegisterCommand( const string & _Command_Name, CommandCallback _Callback )
	{
		m_CommandMap.emplace( _Command_Name.string(), _Callback );
	}

	void ConsoleImpl::UnregisterCommand( const string & _Command_Name )
	{
		m_CommandMap.erase( _Command_Name.string() );
	}

	void ConsoleImpl::ExecuteCommand( const string & _Command )
	{
		std::string cmd = _Command.string();

		auto begin_parameters = cmd.find_first_of( ' ' );

		std::string name = cmd.substr( 0u, begin_parameters );

		auto itt = m_CommandMap.find( name );
		if( itt == m_CommandMap.end() )
		{
			std::string comment = "Unknown command: '" + name + "'";
			WriteLine( Colors::DarkGray, comment.c_str() );
			return;
		}

		string out;

		if( begin_parameters < cmd.size() )
		{
			out = itt->second( string( cmd.substr( begin_parameters + 1 ) ) );
		}
		else
			out = itt->second( "" );

		if( !out.empty() )
			WriteLine( Colors::DarkGray, out.c_str() );
	}

	void ConsoleImpl::WriteLine( color _Color, const char * _Text )
	{
		std::scoped_lock lock( m_WriteMutex );

		m_Lines.emplace_back( m_BufferCount, _Color );

		uint32_t len = (uint32_t)helpers::strlen( _Text );

		auto f = std::ofstream( _Fission_Console_Log_Location, std::ios::app );
		f.write( _Text, len );
		f.write( "\n", 1u );
		f.close();

#ifdef FISSION_CREATE_CONSOLE_WINDOW
		printf( "%s\n", string( _Text, len ).c_str() );
#endif

		reserve( m_BufferCount + len );
		uint32_t start = m_BufferCount;
		m_BufferCount += len;
		memcpy( m_CharacterBuffer + start, _Text, len * sizeof( wchar_t ) );
	}

	void ConsoleImpl::Write( color _Color, const char * _Text )
	{
		FISSION_THROW_NOT_IMPLEMENTED();
	}

	bool ConsoleImpl::GetLine( int _Line_Number, string * _Output_Text, color * _Output_Color )
	{
		if( _Line_Number >= m_Lines.size() )
			return false;

		const TextLine & Start = m_Lines[_Line_Number];

		const char * str = m_CharacterBuffer + Start.start;
		size_t length = m_BufferCount - Start.start;

		if( (size_t)_Line_Number + 1 < m_Lines.size() ) {
			length = m_Lines[(size_t)_Line_Number + 1].start - Start.start;
		}

		*_Output_Text = string( str, length );
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
			while( _New_Count > m_BufferCapacity ) // could be implemented better, but realisticly there should not be more than 2000 chars written at a time
				m_BufferCapacity += 2000u;
			char * temp = (char *)_aligned_malloc( m_BufferCapacity, 16u );

			if( temp == nullptr )
				throw 0x45;

			memcpy( temp, m_CharacterBuffer, m_BufferCount );
			_aligned_free( m_CharacterBuffer );
			m_CharacterBuffer = temp;
		}
	}
}
