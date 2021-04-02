/**
* 
* @file: Console.h
* @author: lazergenixdev@gmail.com
* 
* 
* This file is provided under the MIT License:
* 
* Copyright (c) 2021 Lazergenix Software
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
* 
*/

#pragma once
#include "Fission/config.h"
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

	/***************************************************************************************
	 * @brief 
	 *   Command Callback Function
	 * 
	 * @description: 
	 *   Callback called once every time the corresponding command is executed.
	 * 
	 * @param 
	 *  command_args ::
	 *     Wide String containing everything after the command name.
	 *   
	 * 
	 * @return 
	 *   Wide String message which outputs to the console if command was unsuccessful.
	 * 
	 */
	using CommandCallback = std::function<std::wstring( std::wstring command_args )>;


	/***************************************************************************************
	 * @brief 
	 *   Console, it is a console not much else to say.. 
	 */
	class Console
	{
		static constexpr size_t Default_Format_Buffer_Size = 128;
	public:

		//! @brief Sets whether the Console should be active to the user.
		//! @param _Enable: New state for the Console.
		//! @note Use this to stop console from showing when typing.
		FISSION_THREAD_SAFE FISSION_API static void SetEnabled( bool _Enable );


		//! @brief Check to see if the Console is active for the user.
		//! @return `true` => Console is enabled; `false` otherwise.
		FISSION_THREAD_SAFE FISSION_API static bool IsEnabled();


		//! @brief Clears all text from the Console.
		FISSION_THREAD_SAFE FISSION_API static void Clear();


	   /**
		* @brief  Registers a command name to a callback.
		* @param  _Command_Name: Name the command will be referred as.
		* @param  _Callback: Command callback function that will be called when the command is executed.
		* 
		* @note This function cannot fail, and will simply override commands that already exist.
		*/
		FISSION_THREAD_SAFE FISSION_API static void RegisterCommand( const std::wstring & _Command_Name, CommandCallback _Callback );


		//! @brief Removes a command from the Console.
		//! @param _Command_Name: Name of the command to remove.
		FISSION_THREAD_SAFE FISSION_API static void UnregisterCommand( const std::wstring & _Command_Name );


		//! @brief Executes a full command string.
		//! @param _Full_Command: Wide string of the command to be executed.
		//! @note Syntax of commands should be: <command name> [command arguments]
		FISSION_THREAD_SAFE FISSION_API static void ExecuteCommand( const std::wstring & _Full_Command );


		//! @brief Writes a line to the Console.
		//! @param _Color: Color of the line of text outputted.
		//! @param _Text: [in] Text to be put onto the Console.
		FISSION_THREAD_SAFE FISSION_API static void WriteLine( color _Color, const wchar_t * _Text );


		//! @brief Writes a string to the Console.
		//! @warning THIS FUNCTION IS NOT IMPLEMENTED AS OF v0.1.0
		FISSION_THREAD_SAFE FISSION_API static void Write( color _Color, const wchar_t * _Text );


		//! @brief Gets a line of text and the text color from the Console.
		//! @param _Line_Number Line number of the line retrieved.
		//! @param _Output_Text [out] Destination of where the line of the console is copied to.
		//! @param _Output_Color [out] Destination of where the color of the text is copied to. This may be omitted.
		//! @return `true` => Success; `False` => Function did not find a line at that index.
		FISSION_THREAD_SAFE FISSION_API static bool GetLine( int _Line_Number, std::wstring * _Output_Text, color * _Output_Color = nullptr );


		//! @brief Gets Console line count.
		//! @return Number of lines of text in the Console.
		FISSION_THREAD_SAFE FISSION_API static int GetLineCount();

	// todo: This might be needed for thread safety when reading console.
	//	FISSION_API static std::lock_guard<std::mutex> Lock();



/* ------------------------------------------ Begin Console Helpers ----------------------------------------------- */

		static inline void WriteLine( const wchar_t * _Text ) { WriteLine( FISSION_MESSAGE_COLOR, _Text ); }

		template <size_t _Format_Buffer_Size = Default_Format_Buffer_Size, typename...T>
		static inline void WriteLine( color _Color, const wchar_t * const _Text, T&&...t )
		{
			wchar_t buffer[_Format_Buffer_Size] = {};
			swprintf_s( (wchar_t *)buffer, _Format_Buffer_Size, _Text, std::forward<T>( t )... );
			WriteLine( _Color, (const wchar_t *)buffer );
		}

		template <size_t _Format_Buffer_Size = Default_Format_Buffer_Size, typename...T>
		static inline void WriteLine( const wchar_t * const _Text, T&&...t )
		{
			wchar_t buffer[_Format_Buffer_Size];
			swprintf_s( (wchar_t *)buffer, _Format_Buffer_Size, _Text, std::forward<T>( t )... );
			WriteLine( (const wchar_t *)buffer );
		}

		template <size_t _Format_Buffer_Size = Default_Format_Buffer_Size, typename...T>
		static inline void Error( const wchar_t * _Text, T&&...t ) {
			WriteLine( FISSION_ERROR_COLOR, _Text, std::forward<T>( t )... );
		}

		template <size_t _Format_Buffer_Size = Default_Format_Buffer_Size, typename...T>
		static inline void Warning( const wchar_t * _Text, T&&...t ) {
			WriteLine( FISSION_WARNING_COLOR, _Text, std::forward<T>( t )... );
		}

		template <size_t _Format_Buffer_Size = Default_Format_Buffer_Size, typename...T>
		static inline void Message( const wchar_t * _Text, T&&...t ) {
			WriteLine( FISSION_MESSAGE_COLOR, _Text, std::forward<T>( t )... );
		}

/* ------------------------------------------- End Console Helpers ------------------------------------------------ */

	}; // class Fission::Console

} // namespace Fission
