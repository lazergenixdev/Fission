#pragma once
#include "LazerEngine/config.h"
#include "Color.h"

#ifndef FISSION_CREATE_CONSOLE_WINDOW
#ifdef FISSION_DEBUG
#define FISSION_CREATE_CONSOLE_WINDOW
#endif // FISSION_DEBUG
#endif // FISSION_CREATE_CONSOLE_WINDOW

#ifndef FISSION_ERROR_COLOR
#define FISSION_ERROR_COLOR Fission::Colors::Red
#endif

#ifndef FISSION_WARNING_COLOR
#define FISSION_WARNING_COLOR Fission::Colors::Yellow
#endif

#ifndef FISSION_MESSAGE_COLOR
#define FISSION_MESSAGE_COLOR Fission::Colors::LightGray
#endif

namespace Fission {

// \brief performs a command
// \param command_args: command input
// \returns message for incorrectly formatted commands
	using CommandCallback = std::function<std::wstring( std::wstring command_args )>;

	class Console
	{
		static constexpr size_t Default_Format_Buffer_Size = 128;
	public:

		// use this to stop console from showing when typing
		FISSION_API static void SetEnabled( bool _Enable );

		FISSION_API static bool IsEnabled();

		FISSION_API static void Clear();

		FISSION_API static void RegisterCommand( const std::wstring & _Command_Name, CommandCallback _Callback );

		FISSION_API static void UnregisterCommand( const std::wstring & _Command_Name );

		FISSION_API static void ExecuteCommand( const std::wstring & _Full_Command );

		// All console functions are thread safe
		FISSION_API static void WriteLine( color _Color, const wchar_t * _Text );

		FISSION_API static void Write( color _Color, const wchar_t * _Text );

		// \param _Output_Text - destination of where the line of the console is copied to
		// \return True - success; False - Function did not find a line at that index
		FISSION_API static bool GetLine( int _Line_Number, std::wstring * _Output_Text, color * _Output_Color = nullptr );

		FISSION_API static int GetLineCount();

		// This might be needed for safety when reading console.
	//	FISSION_API static std::lock_guard<std::mutex> Lock();

		static void WriteLine( const wchar_t * _Text ) {
			WriteLine( FISSION_MESSAGE_COLOR, _Text );
		}

		template <size_t _Format_Buffer_Size = Default_Format_Buffer_Size, typename...T>
		static void WriteLine( color _Color, const wchar_t * const _Text, T&&...t )
		{
			wchar_t buffer[_Format_Buffer_Size] = {};
			swprintf_s( (wchar_t *)buffer, _Format_Buffer_Size, _Text, std::forward<T>( t )... );
			WriteLine( _Color, (const wchar_t *)buffer );
		}

		template <size_t _Format_Buffer_Size = Default_Format_Buffer_Size, typename...T>
		static void WriteLine( const wchar_t * const _Text, T&&...t )
		{
			wchar_t buffer[_Format_Buffer_Size];
			swprintf_s( (wchar_t *)buffer, _Format_Buffer_Size, _Text, std::forward<T>( t )... );
			WriteLine( (const wchar_t *)buffer );
		}

		template <size_t _Format_Buffer_Size = Default_Format_Buffer_Size, typename...T>
		static void Error( const wchar_t * _Text, T&&...t ) {
			WriteLine( FISSION_ERROR_COLOR, _Text, std::forward<T>( t )... );
		}

		template <size_t _Format_Buffer_Size = Default_Format_Buffer_Size, typename...T>
		static void Warning( const wchar_t * _Text, T&&...t ) {
			WriteLine( FISSION_WARNING_COLOR, _Text, std::forward<T>( t )... );
		}

		template <size_t _Format_Buffer_Size = Default_Format_Buffer_Size, typename...T>
		static void Message( const wchar_t * _Text, T&&...t ) {
			WriteLine( FISSION_MESSAGE_COLOR, _Text, std::forward<T>( t )... );
		}

	}; // class Fission::Console

} // namespace Fission
