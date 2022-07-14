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
#include <Fission/Base/String.hpp>
#include <Fission/Base/Color.hpp>

#include <optional>
#include <mutex>

#if defined(FISSION_DEBUG)
#define FISSION_CREATE_CONSOLE_WINDOW
#endif

#ifndef FISSION_CONSOLE_ERROR_COLOR
#define FISSION_CONSOLE_ERROR_COLOR Fission::colors::Red
#endif

#ifndef FISSION_CONSOLE_WARNING_COLOR
#define FISSION_CONSOLE_WARNING_COLOR Fission::colors::Yellow
#endif

#ifndef FISSION_CONSOLE_INFO_COLOR
#define FISSION_CONSOLE_INFO_COLOR Fission::colors::LightGray
#endif

#ifndef FISSION_CONSOLE_DEFAULT_COLOR
#define FISSION_CONSOLE_DEFAULT_COLOR Fission::colors::LightGray
#endif

namespace Fission
{

	struct colored_string;
	struct colored_stream;


	/*!
	* @brief String of characters that is colored by one color.
	*/
	struct colored_string
	{
	public:
		string str;
		std::optional<color> col;

	public:
		colored_string( const string& str ): str( str ), col() {}
		colored_string( const string& str, const color& col ): str( str ), col( col ) {}

		inline colored_stream operator+( const colored_string& _Right ) const;

	}; // Fission::colored_string


	/*!
	* @brief Stream of characters that is colored by one or more colors.
	*/
	struct colored_stream
	{
	private:
		using const_iterator = std::vector<colored_string>::const_iterator;

	public:
		colored_stream( size_t capacity = 1u ) { m_strings.reserve( capacity ); }

		colored_stream( const colored_string& string ):colored_stream() { m_strings.emplace_back( string ); }
		colored_stream( const string& string ):colored_stream() { m_strings.emplace_back( string ); }
		colored_stream( const string& str, const color& col ): colored_stream() { m_strings.emplace_back( str, col ); }

		colored_stream( const colored_string& string0, const colored_string& string1 ): colored_stream( 2u )
		{
			m_strings.emplace_back( string0 );
			m_strings.emplace_back( string1 );
		}

		//! @warning This function acts like `+=` and not like how `+` should.
		inline colored_stream& operator+( const colored_string& _Right )
		{
			m_strings.emplace_back( _Right );
			return *this;
		}

		inline const_iterator begin() const { return m_strings.begin(); }
		inline const_iterator end() const { return m_strings.end(); }

	private:
		std::vector<colored_string> m_strings;

	}; // Fission::colored_stream


	colored_stream colored_string::operator+( const colored_string& _Right ) const
	{
		return colored_stream( *this, _Right );
	}


	template <size_t _Size>
	static colored_string operator/( const char( &_Buffer )[_Size], colors::known const& _Color )
	{
		return colored_string( string( _Buffer, _Size - 1u ), _Color );
	}
	static colored_string operator/( const string& _String, colors::known const& _Color )
	{
		return colored_string( _String, _Color );
	}
	static colored_string operator/( const string& _String, const color& _Color )
	{
		return colored_string( _String, _Color );
	}

	struct formatable_string
	{
		template <typename...Params>
		Fission::string operator()( Params&&...params )
		{
			std::string out;

			auto sz = snprintf( nullptr, 0, _Buf, params... );
			out.resize( sz );

			sprintf_s( (char* const)out.data(), sz + 1, _Buf, params... );

			return Fission::string{ out };
		}

		const char* _Buf; size_t _Size;
	};

	static constexpr formatable_string operator""_format( const char* str, size_t len )
	{
		return { str, len };
	}
}

namespace Fission
{

	/*!
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
	 * @return 
	 *   Wide String message which outputs to the console if command was unsuccessful.
	 * 
	 */
	using CommandCallback = std::function<void( string command_args )>;

	
	struct FISSION_API console_iterator
	{
	private:
		struct _string
		{
			string_view str;
			color col;
			int newlines;
		};
	public:
		struct FISSION_API console_string_view
		{
			_string operator*() const;

			console_string_view& operator++(); // Go up one line.
			bool operator!=( const console_string_view& ) const;

			void* parent;
			int pos, line;
		};

	public:

		console_iterator( int _Line );

		console_string_view begin();
		console_string_view end();

	private:
		void* parent;
		int pos, line;

	}; // Fission::console_iterator


	/*!
	 * @brief 
	 *   Console, it is a console not much else to say.. 
	 */
	class FISSION_API Console
	{
	private:
		static constexpr size_t Default_Format_Buffer_Size = 128;

	public:

		//! @brief Sets whether the Console should be active to the user.
		//! @param _Enable: New state for the Console.
		//! @note Use this to stop console from showing when typing.
		FISSION_THREAD_SAFE static void SetEnabled( bool _Enable );


		//! @brief Check to see if the Console is active for the user.
		//! @return `true` => Console is enabled; `false` otherwise.
		FISSION_THREAD_SAFE static bool IsEnabled();


		//! @brief Clears all text from the Console.
		FISSION_THREAD_SAFE static void Clear();


		//! @brief  Registers a command name to a callback.
		//! @param  _Command_Name: Name the command will be referred as.
		//! @param  _Callback: Command callback function that will be called when the command is executed.
		//! @note This function cannot fail, and will simply override commands that already exist.
		FISSION_THREAD_SAFE static void RegisterCommand( const string& _Command_Name, CommandCallback _Callback );


		//! @brief Removes a command from the Console.
		//! @param _Command_Name: Name of the command to remove.
		FISSION_THREAD_SAFE static void UnregisterCommand( const string & _Command_Name );


		//! @brief Executes a full command string.
		//! @param _Full_Command: Wide string of the command to be executed.
		//! @note Syntax of commands should be: <command name> [command arguments]
		FISSION_THREAD_SAFE static void ExecuteCommand( const string & _Full_Command );


		//! @brief Writes a line to the Console, then advances to the next line.
		//! @param _Text: [in] Text to be put onto the Console.
		FISSION_THREAD_SAFE static void WriteLine( const colored_stream& _Text );


		//! @brief Writes a string to the Console.
		//! @since v0.7.1
		FISSION_THREAD_SAFE static void Write( const colored_stream& _Text );


		//! @warning THIS FUNCTION IS NOT IMPLEMENTED AS OF v0.1.0
	//	FISSION_THREAD_SAFE static void Log(color _Color, const char* _Text);


		////! @brief Gets a line of text and the text color from the Console.
		//FISSION_THREAD_SAFE static console_buffer_iterator GetLine( int _Line_Number,  );


		//! @brief Gets Console line count.
		//! @return Number of lines of text in the Console.
		FISSION_THREAD_SAFE static int GetLineCount();

		//! @warning THIS FUNCTION IS NOT IMPLEMENTED AS OF v0.1.0
		//! @note: This might be needed for thread safety when reading console.
		static std::lock_guard<std::mutex> Lock();



/* ------------------------------------------ Begin Console Helpers ----------------------------------------------- */

		//static inline void WriteLine( const char * _Text ) { WriteLine( FISSION_MESSAGE_COLOR, _Text ); }

		//template <size_t _Format_Buffer_Size = Default_Format_Buffer_Size, typename...T>
		//static inline void WriteLine( color _Color, const char * const _Text, T&&...t )
		//{
		//	char buffer[_Format_Buffer_Size] = {};
		//	sprintf_s( buffer, _Text, std::forward<T>( t )... );
		//	WriteLine( _Color, (const char *)buffer );
		//}

		//template <size_t _Format_Buffer_Size = Default_Format_Buffer_Size, typename...T>
		//static inline void WriteLine( const char * const _Text, T&&...t )
		//{
		//	char buffer[_Format_Buffer_Size];
		//	sprintf_s( buffer, _Text, std::forward<T>( t )... );
		//	WriteLine( (const char *)buffer );
		//}

		//template <size_t _Format_Buffer_Size = Default_Format_Buffer_Size, typename...T>
		//static inline void Error( const char * _Text, T&&...t ) {
		//	WriteLine( FISSION_ERROR_COLOR, _Text, std::forward<T>( t )... );
		//}

		//template <size_t _Format_Buffer_Size = Default_Format_Buffer_Size, typename...T>
		//static inline void Warning( const char * _Text, T&&...t ) {
		//	WriteLine( FISSION_WARNING_COLOR, _Text, std::forward<T>( t )... );
		//}

		//template <size_t _Format_Buffer_Size = Default_Format_Buffer_Size, typename...T>
		//static inline void Info( const char * _Text, T&&...t ) {
		//	WriteLine( FISSION_INFO_COLOR, _Text, std::forward<T>( t )... );
		//}

/* ------------------------------------------- End Console Helpers ------------------------------------------------ */

	}; // class Fission::Console

	/*
	* This code will be required for making the Console able to help try auto-complete
	* what the user writes to the console.
	
	* I choose not to implement this for two reasons:
	
	*   - It will make simple commands more complicated to write out.
	*   - It is not helpful to me personally.

	* I do plan on implementing this in the future, but probably after v1.0.0 gets released.

	

  struct CommandArgument
  {
    const char * name;

    bool is_number();
    bool is_string();
    bool is_integer();

    void * user;

  };

  using CommandCallback = std::function<string(CommandArgument* args, int count)>;

  using _ParseArgument = bool(const char * str, int count, CommandArgument * out);

  struct CommandInfo
  {
    const char * name;

    struct ArgumentInfo
    {
      const char * name;

      const char * responses = nullptr;

      _ParseArgument parse = nullptr;

      bool is_integer = false;
      bool is_floating_point = false;

      bool required = true;
    };

    ArgumentInfo * arguments;
    int argument_count = 0;
  };

	*/

} // namespace Fission
