/**
 * @file Exception.hpp
 * @author lazergenixdev@gmail.com
 * 
 *	 _______   _   _____   _____   _   _____   __    _  
 *	|  _____| | | |  ___| |  ___| | | |  _  | |  \  | | 
 *	| |___    | |  \ \     \ \    | | | | | | |   \ | | 
 *	|  ___|   | |   \ \     \ \   | | | | | | | |\ \| | 
 *	| |       | |  __\ \   __\ \  | | | |_| | | | \   | 
 *	|_|       |_| |_____| |_____| |_| |_____| |_|  \__| 
 * 
 *	MIT License
 *	
 *	Copyright (c) 2021-2022 Lazergenix
 *	
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this software and associated documentation files (the "Software"), to deal
 *	in the Software without restriction, including without limitation the rights
 *	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *	copies of the Software, and to permit persons to whom the Software is
 *	furnished to do so, subject to the following conditions:
 *	
 *	The above copyright notice and this permission notice shall be included in all
 *	copies or substantial portions of the Software.
 *	
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 */
#pragma once
#include <Fission/config.hpp>
#include <exception>
#include <format>

/**
* There is a "Feature" in msc where __LINE__
* is not a constexpr, so this is the workaround.
*/
#ifdef _MSC_VER
#define __FISSION_CAT(X,Y) __FISSION_CAT2(X,Y)
#define __FISSION_CAT2(X,Y) X##Y
#define FISSION_CONSTEXPR_LINE int(__FISSION_CAT(__LINE__,U)) 
#else
#define FISSION_CONSTEXPR_LINE __LINE__
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper Macros:

#define FISSION_FILENAME() ::Fission::impl::Filename<::Fission::impl::get_filename_size(__FILE__)>( __FILE__ )
#define FISSION_THROW_LOCATION() ::Fission::base::throw_location( FISSION_CONSTEXPR_LINE, FISSION_FILENAME() )

#define FISSION_THROW(NAME, APPENDS) {\
static constexpr auto _ = FISSION_THROW_LOCATION(); \
throw ::Fission::base::generic_error( NAME, ::Fission::base::error_message(_) APPENDS ); \
} (void)0

#define FISSION_THROW_NOT_IMPLEMENTED() \
FISSION_THROW( "Not Implemented",.append( "Function '" __FUNCTION__ "' not implemented." ) )

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

__FISSION_BEGIN__

namespace impl
{
	template <size_t _Size>
	static constexpr size_t get_filename_size( const char (&_Filepath)[_Size] )
	{
		size_t ret = _Size-1;
        int n = 0;

		do {
			if( 
				_Filepath[ret] == '\\' // Windows Filesystem
			||  _Filepath[ret] == '/'  // Literally every other Filesystem
			) ++n;
                
            if( n >= 2 )
                return _Size - ret - 1;
		} 
		while( ret-- > 0u );

		return _Size;
	}

    template <size_t _Filename_Size>
    struct Filename
    {
        char value[_Filename_Size];
        
        template <size_t _Size>
        constexpr Filename( const char (&_Path)[_Size] ) : value()
        {
            for( size_t i = 0; i < _Filename_Size; ++i )
            { // Ninja Turtles
                value[i] = _Path[i+_Size-_Filename_Size];
            }
        }
    }; // struct Fission::impl::Filename
	
} // namespace Fission::impl

namespace base
{

	template <size_t _Size>
	class throw_location
	{
	public:
		constexpr throw_location( int line, const impl::Filename<_Size> & file ) noexcept
			: _Line( line ), _File( file )
		{}

		constexpr int get_line() const noexcept { return _Line; }
		constexpr impl::Filename<_Size> const& get_file() const noexcept { return _File; }

		std::string as_string() const noexcept {
			return std::format( "[{}:{}]", _File.value, _Line );
		}
	private:
		int _Line;
		impl::Filename<_Size> _File;

	}; // class Fission::throw_location


	class error_message
	{
	public:
		template <size_t _Size>
		error_message( const throw_location<_Size> & loc ) noexcept : _msg( loc.as_string() ) {}

		error_message & append( std::string const& msg ) noexcept {
			std::format_to( std::back_inserter( _msg ), "\n\n{}", msg );
			return *this;
		}

		error_message & append( std::string const& key, std::string const& msg ) noexcept {
			std::format_to( std::back_inserter( _msg ), "\n\n{}:\n{}", key, msg );
			return *this;
		}

		inline operator const std::string & ( ) const noexcept {
			return _msg;
		}

	private:
		std::string _msg;

	}; // class Fission::error_message


	class runtime_error : public std::runtime_error
	{
	public:
		const char * name() const noexcept { return _name.c_str(); }

	protected:
		runtime_error( const std::string & name, const std::string & msg ) noexcept
			: std::runtime_error( msg ), _name( name )
		{}

	private:
		std::string _name;

	}; // class Fission::runtime_error


	// `generic_error` should be used for all exceptions,
	//  but for exceptions that will explicitly be caught, inherit from `runtime_error`
	class generic_error : public runtime_error
	{
	public:
		generic_error( const std::string & name, const error_message & message ) noexcept
			: runtime_error( name, message )
		{}
	}; // class Fission::generic_error

} // namespace Fission::base

__FISSION_END__
