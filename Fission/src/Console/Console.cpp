#include "ConsoleImpl.h"

namespace Fission {

	void Console::SetEnabled( bool _Enable ) {
		return ConsoleImpl::Get().SetEnabled( _Enable );
	}

	bool Console::IsEnabled() {
		return ConsoleImpl::Get().IsEnabled();
	}

	void Console::Clear() {
		return ConsoleImpl::Get().Clear();
	}

	void Console::RegisterCommand( const string& _Command_Name, CommandCallback _Callback ) {
		return ConsoleImpl::Get().RegisterCommand( _Command_Name, _Callback );
	}

	void Console::UnregisterCommand( const string & _Command_Name ) {
		return ConsoleImpl::Get().UnregisterCommand( _Command_Name );
	}

	void Console::ExecuteCommand( const string & _Command ) {
		return ConsoleImpl::Get().ExecuteCommand( _Command );
	}

	void Console::WriteLine( const colored_stream& _Text ) {
		return ConsoleImpl::Get().WriteLine( _Text );
	}

	void Console::Write( const colored_stream& _Text ) {
		return ConsoleImpl::Get().Write( _Text );
	}

	//bool Console::GetLine( int _Line_Number, string * _Output_Text, color * _Output_Color ) {
	//	return ConsoleImpl::Get().GetLine( _Line_Number, _Output_Text, _Output_Color );
	//}

	int Console::GetLineCount() {
		return ConsoleImpl::Get().GetLineCount();
	}

	console_iterator::console_iterator( int ln ): parent(&ConsoleImpl::Get())
	{
		auto loc = static_cast<ConsoleImpl*>(parent)->_GetSegmentFromLine( ln );

		pos = loc.startSegment;
		line = loc.startNewLine;
	}

	console_iterator::console_string_view console_iterator::begin()
	{
		return console_string_view{ parent, pos, line };
	}

	console_iterator::console_string_view console_iterator::end()
	{
		auto console = static_cast<ConsoleImpl*>( parent );
		return console_string_view{ nullptr, (int)console->m_Segments.size(), 0 };
	}

	console_iterator::_string console_iterator::console_string_view::operator*() const
	{
		auto console = static_cast<ConsoleImpl*>( parent );

		switch( line )
		{
		[[unlikely]] default:
		{
			return { string_view{ "",0u }, { 0.0f }, (int)line };
		}
		[[likely]] case 0:
		{
			auto [start,newlines,color,p] = console->m_Segments[pos];
			const char * str = console->m_CharacterBuffer + start;
			size_t length = console->m_BufferCount - start;

			if( (size_t)pos + 1u < console->m_Segments.size() ) {
				length = console->m_Segments[pos + 1].StartOffset - start;
			}

			return { string_view{ str, length }, color, (int)newlines };
		}
		}
	}

	console_iterator::console_string_view& console_iterator::console_string_view::operator++()
	{
		//if( line ) [[unlikely]]
		//{
		//	line = 0;
		//	return *this;
		//}

		//auto console = static_cast<ConsoleImpl*>( parent );
		//const auto& seg = console->m_Segments[pos];

		//if( seg.NewLines )
		//	pos = seg.PrevSegment;
		//else
		//	if( ++pos == (int)console->m_Segments.size() ) pos = seg.PrevSegment;

		++pos;
		line = 0;

		return *this;
	}

	bool console_iterator::console_string_view::operator!=( const console_string_view& _Right ) const
	{
		return pos != _Right.pos;
	}

}

