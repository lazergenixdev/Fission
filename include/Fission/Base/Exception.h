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

#include <exception>
#include <filesystem>
#include <sstream>

#define FISSION_FILENAME() ::Fission::base::Filename<::Fission::base::detail::get_filename_size( __FILE__ )>( __FILE__ )
#define FISSION_THROW_LOCATION_HERE() ::Fission::base::throw_location( __LINE__, FISSION_FILENAME() )

#define FISSION_EXPAND(X) X
#define FISSION_UNIQUE_NAME(LINE,N) FISSION_EXPAND(N)##LINE

#define FISSION_THROW(NAME, ...) \
static constexpr auto FISSION_UNIQUE_NAME(__LINE__,__loc) = FISSION_THROW_LOCATION_HERE(); \
throw ::Fission::base::Exception( NAME, ::Fission::base::exception_message(FISSION_UNIQUE_NAME(__LINE__,__loc)) __VA_ARGS__ )

#define FISSION_THROW_NOT_IMPLEMENTED() \
FISSION_THROW( "'I'm Lazy Exception'",.append( "'" __FUNCTION__ "' not implemented." ) )

namespace Fission
{
  namespace base
  {

    namespace detail
    {

      template <size_t _Size>
      static constexpr size_t get_filename_size( const char (&_Filepath)[_Size] )
      {
          size_t ret = _Size-1;

          do {
              if( 
                  _Filepath[ret] == '\\' // Windows Filesystem
               || _Filepath[ret] == '/'  // Literally every other Filesystem
             ) return _Size - ret - 1;
          } 
          while( ret-- > 0u );

          return _Size;
      }

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



    template <typename _FilenameTy>
    class throw_location
    {
    public:
      constexpr throw_location( int line, const _FilenameTy & file ) noexcept
        : _Line( line ), _File( file )
      {}

      constexpr int get_line() const noexcept { return _Line; }
      constexpr _FilenameTy & get_file() const noexcept { return _File; }

      std::string as_string() const noexcept {
        std::stringstream ss;
        ss << "File: " << _File.value << " @Line " << _Line;
        return ss.str();
      }
    private:
      int _Line;
      _FilenameTy _File;
    }; // class Fission::base::throw_location



    template <typename _ThrowLocationTy>
    class exception_message
    {
    public:
      exception_message( const _ThrowLocationTy & loc ) noexcept : _msg( loc.as_string() ) {}

      exception_message & append( const std::string & msg ) noexcept {
        _msg += "\n\n";
        _msg += msg;
        return *this;
      }

      exception_message & append( const std::string & key, const std::string & msg ) noexcept {
        _msg += "\n\n";
        _msg += key;
        _msg += ":\n";
        _msg += msg;
        return *this;
      }

      operator const std::string & ( ) const noexcept {
        return _msg;
      }

    private:
      std::string _msg;

    }; // class Fission::base::exception_message



    class base_exception : public std::runtime_error
    {
    public:
        base_exception( const std::string & name, const std::string & msg ) noexcept
            : std::runtime_error( msg ), _name( name )
        {}

        const char * name() const noexcept { return _name.c_str(); }

    private:
        std::string _name;
    }; // class Fission::base::base_exception



    template <typename _MessageTy>
    class Exception : public base_exception
    {
    public:
      Exception( const std::string & name, const _MessageTy & message ) noexcept
        : base_exception( name, message )
      {}
    }; // class Fission::base::Exception


  } // namespace Fission::base
} // namespace Fission
