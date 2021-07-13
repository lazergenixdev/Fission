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

	void Console::RegisterCommand( const string & _Command_Name, CommandCallback _Callback ) {
		return ConsoleImpl::Get().RegisterCommand( _Command_Name, _Callback );
	}

	void Console::UnregisterCommand( const string & _Command_Name ) {
		return ConsoleImpl::Get().UnregisterCommand( _Command_Name );
	}

	void Console::ExecuteCommand( const string & _Command ) {
		return ConsoleImpl::Get().ExecuteCommand( _Command );
	}

	void Console::WriteLine( color _Color, const char * _Text ) {
		return ConsoleImpl::Get().WriteLine( _Color, _Text );
	}

	void Console::Write( color _Color, const char * _Text ) {
		return ConsoleImpl::Get().Write( _Color, _Text );
	}

	bool Console::GetLine( int _Line_Number, string * _Output_Text, color * _Output_Color ) {
		return ConsoleImpl::Get().GetLine( _Line_Number, _Output_Text, _Output_Color );
	}

	int Console::GetLineCount() {
		return ConsoleImpl::Get().GetLineCount();
	}

}

