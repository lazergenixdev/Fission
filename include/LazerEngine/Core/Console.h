#pragma once
#include "LazerEngine/config.h"
#include "Color.h"

#ifndef LAZER_CREATE_CONSOLE_WINDOW
#if LAZER_DEBUG
#define LAZER_CREATE_CONSOLE_WINDOW 1
#else
#define LAZER_CREATE_CONSOLE_WINDOW 0
#endif /* LAZER_DEBUG */
#endif /* LAZER_CREATE_CONSOLE_WINDOW */

#ifndef LAZER_ERROR_COLOR
#define LAZER_ERROR_COLOR lazer::Colors::Red
#endif

#ifndef LAZER_WARNING_COLOR
#define LAZER_WARNING_COLOR lazer::Colors::Yellow
#endif

#ifndef LAZER_MESSAGE_COLOR
#define LAZER_MESSAGE_COLOR lazer::Colors::LightGray
#endif

namespace lazer {

// \brief performs a command
// \param command_args: command input
// \returns message for incorrectly formatted commands
	using CommandCallback = std::function<std::wstring( std::wstring command_args )>;

	class Console
	{
		static constexpr size_t Default_Format_Buffer_Size = 128;
	public:

		// use this to stop console from showing when typing
		LAZER_API static void SetEnabled( bool _Enable );

		LAZER_API static bool IsEnabled();

		LAZER_API static void Clear();

		LAZER_API static void RegisterCommand( const std::wstring & _Command_Name, CommandCallback _Callback );

		LAZER_API static void UnregisterCommand( const std::wstring & _Command_Name );

		LAZER_API static void ExecuteCommand( const std::wstring & _Full_Command );

		// All console functions are thread safe
		LAZER_API static void WriteLine( color _Color, const wchar_t * _Text );

		LAZER_API static void Write( color _Color, const wchar_t * _Text );

		// \param _Output_Text - destination of where the line of the console is copied to
		// \return True - success; False - Function did not find a line at that index
		LAZER_API static bool GetLine( int _Line_Number, std::wstring * _Output_Text, color * _Output_Color = nullptr );

		LAZER_API static int GetLineCount();

		// This might be needed for safety when reading console.
	//	LAZER_API static std::lock_guard<std::mutex> Lock();

		static void WriteLine( const wchar_t * _Text ) {
			WriteLine( LAZER_MESSAGE_COLOR, _Text );
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
			WriteLine( LAZER_ERROR_COLOR, _Text, std::forward<T>( t )... );
		}

		template <size_t _Format_Buffer_Size = Default_Format_Buffer_Size, typename...T>
		static void Warning( const wchar_t * _Text, T&&...t ) {
			WriteLine( LAZER_WARNING_COLOR, _Text, std::forward<T>( t )... );
		}

		template <size_t _Format_Buffer_Size = Default_Format_Buffer_Size, typename...T>
		static void Message( const wchar_t * _Text, T&&...t ) {
			WriteLine( LAZER_MESSAGE_COLOR, _Text, std::forward<T>( t )... );
		}

	};

}
