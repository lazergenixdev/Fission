/**
*
* @file: Exception.h
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
#include "config.h"
#include <exception>
#include <filesystem>
#include <sstream>

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

#define FISSION_FILENAME() ::Fission::base::Filename<::Fission::base::__get_filename_size( __FILE__ )>( __FILE__ )
#define FISSION_THROW_LOCATION_HERE() ::Fission::base::throw_location( FISSION_CONSTEXPR_LINE, FISSION_FILENAME() )

#define FISSION_THROW(NAME, APPENDS) {\
static constexpr auto _ = ::Fission::base::throw_location( FISSION_CONSTEXPR_LINE, FISSION_FILENAME() ); \
throw ::Fission::base::generic_error( NAME, ::Fission::base::error_message(_) APPENDS ); \
}

#define FISSION_THROW_NOT_IMPLEMENTED() \
FISSION_THROW( "'I'm Lazy Exception'",.append( "'" __FUNCTION__ "' not implemented." ) )

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

_FISSION_BASE_BEGIN

template <size_t _Size>
static constexpr size_t __get_filename_size( const char (&_Filepath)[_Size] )
{
    size_t ret = _Size-1;

    do {
        if( 
            _Filepath[ret] == '\\' // Windows Filesystem
        ||  _Filepath[ret] == '/'  // Literally every other Filesystem
        ) return _Size - ret - 1;
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
}; // struct Fission::base::Filename



template <size_t _Size>
class throw_location
{
public:
    constexpr throw_location( int line, const Filename<_Size> & file ) noexcept
        : _Line( line ), _File( file )
    {}

    constexpr int get_line() const noexcept { return _Line; }
    constexpr Filename<_Size> & get_file() const noexcept { return _File; }

    std::string as_string() const noexcept {
        std::stringstream ss;
        ss << "File: " << _File.value << " @Line " << _Line;
        return ss.str();
    }
private:
    int _Line;
    Filename<_Size> _File;
}; // class Fission::base::throw_location



template <size_t _Size>
class error_message
{
public:
    error_message( const throw_location<_Size> & loc ) noexcept : _msg( loc.as_string() ) {}

    error_message & append( const std::string & msg ) noexcept {
        _msg += "\n\n";
        _msg += msg;
        return *this;
    }

    error_message & append( const std::string & key, const std::string & msg ) noexcept {
        _msg += "\n\n";
        _msg += key;
        _msg += ":\n";
        _msg += msg;
        return *this;
    }

    inline operator const std::string & ( ) const noexcept {
        return _msg;
    }

private:
    std::string _msg;

}; // class Fission::base::exception_message



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

}; // class Fission::base::runtime_error



class generic_error : public runtime_error
{
public:
    template <size_t _Size>
    generic_error( const std::string & name, const error_message<_Size> & message ) noexcept
        : runtime_error( name, message )
    {}
}; // class Fission::base::generic_error


_FISSION_BASE_END
