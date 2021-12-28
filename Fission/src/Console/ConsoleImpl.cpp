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

		void OpenConsoleWindow()
		{
			// Create Console Window
			AllocConsole();
			auto f = freopen("CONOUT$", "w", stdout);
				 f = freopen("CONOUT$", "w", stderr);

			// Get the name of the current process
			char buffer[MAX_PATH];
			GetModuleFileNameA(NULL, buffer, (DWORD)std::size(buffer));
			std::string sProcessName = std::filesystem::path(buffer).filename().string();

			// Set the title of the console window
			char title[100];
			sprintf(title, "Debug Console - %s", sProcessName.c_str());
			SetConsoleTitleA(title);
		}
	}

	ConsoleImpl::ConsoleImpl()
	{
#ifdef FISSION_CREATE_CONSOLE_WINDOW
		helpers::OpenConsoleWindow();
#endif /* FISSION_CREATE_CONSOLE_WINDOW */

		{ // Clear contexts of debug log for writing
			auto f = std::ofstream( _Fission_Console_Log_Location, std::ios::trunc );

			f << "===== " _Fission_Console_Log_Location " =====\n\n";
		}

		m_CharacterBuffer = (char *)_aligned_malloc( m_BufferCapacity, 16u );
		m_Segments.reserve( 1000 );
		m_Lines.reserve( 200 );

		/* Default Console commands */

		RegisterCommand( "help",
			[]( const string & s ) {
				Console::WriteLine( "There shall be NO help! >:)" / Colors::White );
			}
		);

		RegisterCommand( "cls",
			[]( const string& ) {
				Console::Clear();
			}
		);

		RegisterCommand( "msg",
			[]( const string& s ) {
				Console::WriteLine( s / Colors::White );
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

	//	memset( m_CharacterBuffer, 0, m_BufferCount );
		m_BufferCount = 0u;

		m_Segments.clear();
		m_Lines.clear();
	}

	void ConsoleImpl::RegisterCommand( const string& _Command_Name, CommandCallback _Callback )
	{
		m_CommandMap.emplace( _Command_Name.string(), _Callback);
	}

	void ConsoleImpl::UnregisterCommand( const string & _Command_Name )
	{
		m_CommandMap.erase( _Command_Name.string() );
	}

	void ConsoleImpl::ExecuteCommand( const string & _Command )
	{
		std::string cmd = _Command.string();

		size_t begin_parameters = cmd.find_first_of( ' ' );

		std::string name = cmd.substr( 0u, begin_parameters );

		auto itt = m_CommandMap.find( name );
		if( itt == m_CommandMap.end() )
		{
			WriteLine( string("Unknown command: '" + name + "'") / Colors::DarkGray );
			return;
		}

		if( begin_parameters < cmd.size() )
			itt->second( string( cmd.substr(begin_parameters+1u) ) );
		else
			itt->second( "" );
	}

//	void ConsoleImpl::WriteLine( color _Color, const char * _Text )
//	{
//		std::scoped_lock lock( m_WriteMutex );
//
//		m_Lines.emplace_back( m_BufferCount, _Color );
//
//		uint32_t len = (uint32_t)helpers::strlen( _Text );
//
//		auto f = std::ofstream( _Fission_Console_Log_Location, std::ios::app );
//		f.write( _Text, len );
//		f.write( "\n", 1u );
//		f.close();
//
//#ifdef FISSION_CREATE_CONSOLE_WINDOW
//		printf( "%s\n", string( _Text, len ).c_str() );
//#endif
//
//		reserve( m_BufferCount + len );
//		uint32_t start = m_BufferCount;
//		m_BufferCount += len;
//		memcpy( m_CharacterBuffer + start, _Text, len );
//	}

	void ConsoleImpl::WriteLine( const colored_stream& _Text )
	{
		std::scoped_lock lock( m_WriteMutex );

		auto f = std::ofstream( _Fission_Console_Log_Location, std::ios::app );

		for( const auto& [str, col] : _Text )
		{
			size_t start = 0u;
#ifdef FISSION_CREATE_CONSOLE_WINDOW
			printf( "%s", str.c_str() );
#endif
			f << str.string();

		find_end:
			size_t end = str.find_first_of( '\n', start );

			switch( end )
			{
			case 0u: // newline at the beginning
			{
				add_newline();
				goto find_end;
			}
			case string::npos: // no newline from `start` to end
			{
				add_line( str.c_str() + start, uint32_t(str.size() - start), 0u, col.value_or(FISSION_CONSOLE_DEFAULT_COLOR));
				break; 
			}
			default:
			{
				add_line( str.c_str() + start, uint32_t(end - start), 1u, col.value_or(FISSION_CONSOLE_DEFAULT_COLOR) );
				start = end + 1;
				goto find_end;
			}
			}
		}

#ifdef FISSION_CREATE_CONSOLE_WINDOW
		printf( "\n" );
#endif
		f << "\n";

		add_newline();
	}

	void ConsoleImpl::Write( const colored_stream& _Text )
	{
		std::scoped_lock lock( m_WriteMutex );

		for( const auto&[str,col] : _Text )
		{
			size_t start = 0u;
	#ifdef FISSION_CREATE_CONSOLE_WINDOW
			printf( "%s", str.c_str() );
	#endif
			if( auto f = std::ofstream( _Fission_Console_Log_Location, std::ios::app ) )
				f << str.string();

		find_end:
			size_t end = str.find_first_of( '\n', start );

			if( start == end ) // newline at the beginning
			{
				add_newline();
				++start;
				goto find_end;
			}

			if( end == string::npos ) // no newline from `start` to end
			{ 
				if( start == str.size() )
					break;

				add_line( str.c_str() + start, uint32_t(end - start), 0u, col.value_or( FISSION_CONSOLE_DEFAULT_COLOR ) );
				break; 
			}
			else
			{
				add_line( str.c_str() + start, uint32_t(end - start), 1u, col.value_or(FISSION_CONSOLE_DEFAULT_COLOR) );
				start = end + 1;
				goto find_end;
			}
		}
	}

	//bool ConsoleImpl::GetLine( int _Line_Number, string * _Output_Text, color * _Output_Color )
	//{
	//	if( _Line_Number >= m_Lines.size() )
	//		return false;

	//	const TextLine & Start = m_Lines[_Line_Number];

	//	const char * str = m_CharacterBuffer + Start.start;
	//	size_t length = m_BufferCount - Start.start;

	//	if( (size_t)_Line_Number + 1 < m_Lines.size() ) {
	//		length = m_Lines[(size_t)_Line_Number + 1].start - Start.start;
	//	}

	//	*_Output_Text = string( str, length );
	//	if( _Output_Color ) *_Output_Color = Start.color;

	//	return true;
	//}

	int ConsoleImpl::GetLineCount()
	{
		return (int)m_Lines.size();
	}

	ConsoleImpl::LineInfo ConsoleImpl::_GetSegmentFromLine( uint32_t _Line )
	{
#ifdef FISSION_DEBUG
		if( _Line + 1 > m_Lines.size() )
			FISSION_THROW("Out of Bounds Exception", .append("Line number greater than the Console line count."))
#endif
		return m_Lines[_Line];
	}

	ConsoleImpl & ConsoleImpl::Get()
	{
		static ConsoleImpl _Console_Instance;
		return _Console_Instance;
	}

	void ConsoleImpl::add_newline()
	{
		// Special case where there are no characters in the buffer.
		if( m_Segments.empty() ) [[unlikely]]
			m_Segments.emplace_back( 0u, 1u, FISSION_CONSOLE_DEFAULT_COLOR );
		else [[likely]]
			++m_Segments.back().NewLines; // add newline to the last segment.

		if( m_Segments.back().NewLines > 1u )
			m_Lines.emplace_back( m_Lines.back().startSegment, m_Segments.back().NewLines - 1u);
	}

	void ConsoleImpl::add_line( const char* _Buffer, uint32_t _Length, uint32_t numNewLines, const color& _Color )
	{
		StringSegment seg{ m_BufferCount, numNewLines, _Color };

		// Make sure there is enough space in the buffer.
		reserve( m_BufferCount + _Length );

		// Copy the character to the console buffer.
		memcpy( m_CharacterBuffer + m_BufferCount, _Buffer, _Length );
		m_BufferCount += _Length;

		if( m_Segments.size() )
		{
			// Previous segment had a newline, we are on a new line => add a line.
			if( m_Segments.back().NewLines )
				m_Lines.emplace_back( (uint32_t)m_Segments.size(), 0u );
		}
		else m_Lines.emplace_back( 0u, 0u );

		if( m_Lines.size() > 1u )
		{
			seg.PrevSegment = m_Lines[m_Lines.size() - 2u].startSegment;
		}

		m_Segments.emplace_back( seg );
	}

	void ConsoleImpl::reserve( uint32_t _New_Count )
	{
		if( _New_Count > m_BufferCapacity )
		{
			// could be implemented better, but realisticly there should not be more than 2000 chars written at a time
			while( _New_Count > m_BufferCapacity )
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
