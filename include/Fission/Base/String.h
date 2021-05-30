#pragma once
#include "config.h"
#include <string>

/*
* 
* @note  The implementation for Unicode only guarantees correct
*			encoded output for correct encoded input, there are
*			no checks for invalid UTF encoding.
* 
* 
* 	INDEX
*
* [String Config]
* [String Types]
* [String Implementation]
* [String Helpers]
* [String Definitions]
* [String Operators]
* [String Literals]
* [String Conversions]
*
*/



/* ======================== [String Config] ======================== */

#if FISSION_STRING_ENABLE_TRACE
#define _FISSION_STRING_TRACE( fmt, ... ) printf( fmt, __VA_ARGS__ )
#else
#define _FISSION_STRING_TRACE( fmt, ... ) ((void)0)
#endif

#ifdef __cpp_aligned_new
#define FISSION_STRING_IMPL_MALLOC(X) ::operator new( X, std::align_val_t(32) )
#define FISSION_STRING_IMPL_FREE(X) ::operator delete( X, std::align_val_t(32) )
#else
#ifdef _WIN32
#define FISSION_STRING_IMPL_MALLOC(X) _aligned_malloc( X, 32 )
#define FISSION_STRING_IMPL_FREE(X) _aligned_free( X, 32 )
#else
#define FISSION_STRING_IMPL_MALLOC(X) malloc( X )
#define FISSION_STRING_IMPL_FREE(X) free( X )
#endif
#endif

_FISSION_BASE_PUBLIC_BEGIN

using codepoint = char32_t;

#ifndef __cpp_char8_t
using char8_t = char;
#endif



/* ======================== [String Types] ======================== */

/* String that uses UTF-8 Encoding Standard. */
class utf8_string;

/* String that uses UTF-16 Encoding Standard. */
class utf16_string;

/* String for representing Unicode code points */
class utf32_string;

/* String that uses single-byte US-ASCII encoding. */
class ascii_string;


/* Select default string to use. */
using string = utf8_string;



/* ==================== [String Implementation] ==================== */

//! @brief Container Type for null terminated character arrays.
template <typename _CharType>
class _string_impl
{
public:

	using char_type = _CharType;
	using iterator = char_type *;
	using const_iterator = std::add_const_t<iterator>;

	_string_impl() :
		m_pData( m_Storage ), m_Capacity( 0 ),
		m_Size( 0 ), m_Storage()
	{
		_FISSION_STRING_TRACE( "Default Constructor\n" );
	}

	_string_impl( size_t _Count, char_type _Ch ) :
		m_pData( m_Storage ), m_Capacity( 0 ),
		m_Size( _Count ), m_Storage()
	{
		_FISSION_STRING_TRACE( "Constructor: " __FUNCSIG__ "\n" );
		if( _Count > ( std::size( m_Storage ) - 1 ) )
		{
			m_Capacity = m_Size + 1;
			m_pData = (char_type *)FISSION_STRING_IMPL_MALLOC( m_Capacity * sizeof( char_type ) );

			_FISSION_STRING_TRACE( "\tallocated %zi bytes.\n", m_Capacity );
			if( !m_pData ) throw std::bad_alloc();
		}

		for( size_t i = 0; i < m_Size; ++i )
			m_pData[i] = _Ch;

		m_pData[m_Size] = static_cast<char_type>( 0 );
	}

	_string_impl( const char_type * _Str, size_t _Count ) :
		m_pData( m_Storage ), m_Capacity( 0 ),
		m_Size( _Count ), m_Storage()
	{
		_FISSION_STRING_TRACE( "Constructor: " __FUNCSIG__ "\n" );
		if( _Count > ( std::size( m_Storage ) - 1 ) )
		{
			m_Capacity = m_Size + 1;
			m_pData = (char_type *)FISSION_STRING_IMPL_MALLOC( m_Capacity * sizeof( char_type ) );

			_FISSION_STRING_TRACE( "\tallocated %zi bytes.\n", m_Capacity );
			if( !m_pData ) throw std::bad_alloc();
		}

		for( size_t i = 0; i < m_Size; ++i )
			m_pData[i] = _Str[i];

		m_pData[m_Size] = static_cast<char_type>( 0 );
	}

	_string_impl( const std::basic_string<char_type> & _Src )
		: _string_impl( _Src.data(), _Src.size() ) 
	{}

	_string_impl( const _string_impl & _Src ) :
		m_pData( m_Storage ), m_Capacity( _Src.m_Capacity ),
		m_Size( _Src.m_Size ), m_Storage()
	{
		_FISSION_STRING_TRACE( "Copy Constructor\n" );
		if( m_Capacity )
		{
			m_pData = (char_type *)FISSION_STRING_IMPL_MALLOC( m_Capacity * sizeof( char_type ) );

			_FISSION_STRING_TRACE( "\tallocated %zi bytes.\n", m_Capacity );
			if( !m_pData ) throw std::bad_alloc();
		}

		for( size_t i = 0; i < m_Size; ++i )
			m_pData[i] = _Src.m_pData[i];

		m_pData[m_Size] = static_cast<char_type>( 0 );
	}

	_string_impl( _string_impl && src ) noexcept :
		m_pData( src.m_pData ), m_Capacity( src.m_Capacity ),
		m_Size( src.m_Size ), m_Storage()
	{
		_FISSION_STRING_TRACE( "Move Constructor\n" );
		src.m_Size = 0u;
		if( !m_Capacity )
		{
			m_pData = m_Storage;
			_FISSION_STRING_TRACE( "\tCopying to automatic storage.\n" );
			for( size_t i = 0; i < m_Size; ++i )
				m_pData[i] = src.m_pData[i];
			m_pData[m_Size] = '\0';
			return;
		}
		src.m_pData = m_Storage;
		src.m_Capacity = 0u;
		src.m_Storage[0] = static_cast<char_type>( 0 );
	}

	_string_impl & operator=( _string_impl && src )
	{
		_FISSION_STRING_TRACE( "Move Operator\n" );
		m_pData = src.m_pData;
		m_Capacity = src.m_Capacity;
		m_Size = src.m_Size;

		if( !m_Capacity )
		{
			m_pData = m_Storage;
			_FISSION_STRING_TRACE( "\tCopying to automatic storage.\n" );
			for( size_t i = 0; i < m_Size; ++i )
				m_pData[i] = src.m_pData[i];
			m_pData[m_Size] = static_cast<char_type>( 0 );
			return *this;
		}

		src.m_pData = m_Storage;
		src.m_Capacity = 0u;
		src.m_Size = 0u;
		src.m_Storage[0] = static_cast<char_type>( 0 );

		return *this;
	}

	_string_impl & operator=( const _string_impl & src )
	{
		_FISSION_STRING_TRACE( "Copy operator\n" );
		m_Size = src.m_Size;
		if( m_Size > ( std::size( m_Storage ) - 1 ) )
		{
			if( m_Size + 1 > m_Capacity )
			{
				if( m_Capacity )
				{
					_FISSION_STRING_TRACE( "\tfreed %zi bytes.\n", m_Capacity );
					FISSION_STRING_IMPL_FREE( m_pData );
				}
				m_Capacity = m_Size + 1;
				m_pData = (char_type *)FISSION_STRING_IMPL_MALLOC( m_Capacity * sizeof( char_type ) );
				_FISSION_STRING_TRACE( "\tallocated %zi bytes.\n", m_Capacity );
				if( !m_pData ) throw std::bad_alloc();
			}
		}
		else
		{
			if( m_Capacity )
			{
				_FISSION_STRING_TRACE( "\tfreed %zi bytes.\n", m_Capacity );
				FISSION_STRING_IMPL_FREE( m_pData );
				m_Capacity = 0;
			}
			m_pData = m_Storage;
		}

		for( size_t i = 0; i < m_Size; ++i )
			m_pData[i] = src.m_pData[i];

		m_pData[m_Size] = static_cast<char_type>( 0 );

		return *this;
	}

	~_string_impl()
	{
		_FISSION_STRING_TRACE( "Destructor\n" );
		if( m_Capacity )
		{
			_FISSION_STRING_TRACE( "\tfreed %zi bytes.\n", m_Capacity );
			FISSION_STRING_IMPL_FREE( m_pData );
		}
	}

 	void shrink_to_fit()
	{
		// We already don't have memory allocated.
		if( !m_Capacity ) return;

		// Check to see if we can store the characters in local storage.
		if( m_Size < std::size( m_Storage ) )
		{
			for( size_t i = 0; i < m_Size; ++i )
				m_Storage[i] = m_pData[i];
			m_Storage[m_Size] = static_cast<char_type>( 0 );

			_FISSION_STRING_TRACE( "_string_impl::shrink_to_fit(): Moved %zi bytes to local storage.\n", m_Capacity );
			m_Capacity = 0u;
			FISSION_STRING_IMPL_FREE( m_pData );
			return;
		}

		// Check if `m_Size` is equal to `m_Capacity`,
		// but also handle the case where they are invalid.. `m_Size > m_Capacity`
		if( m_Size + 2 > m_Capacity ) return;

		m_Capacity = m_Size + 1;
		_FISSION_STRING_TRACE( "_string_impl::shrink_to_fit(): resize to %zi bytes.\n", m_Capacity );
		char_type * _New_Dest = (char_type *)FISSION_STRING_IMPL_MALLOC( m_Capacity * sizeof( char_type ) );

		// Copy characters to the new Memory.
		memcpy( _New_Dest, m_pData, m_Capacity );

		// Old Memory can now be freed.
		FISSION_STRING_IMPL_FREE( m_pData );

		m_pData = _New_Dest;
	}

	inline char_type& operator[](size_t _Index) { return m_pData[_Index]; }
	inline const char_type& operator[](size_t _Index) const { return m_pData[_Index]; }

	inline char_type * data() { return m_pData; }
	inline size_t size() const { return m_Size; }
	inline size_t capacity() const { return m_Capacity ? ( m_Capacity - 1 ) : ( std::size( m_Storage ) - 1 ); }
	inline bool empty() const { return !bool( m_Size ); }

	inline void clear() { m_Size = 0; m_pData[0] = static_cast<char_type>( 0 ); }
	inline void pop_back() { m_pData[--m_Size] = static_cast<char_type>( 0 ); }

	inline iterator begin() { return iterator( m_pData ); }
	inline iterator end() { return iterator( m_pData + m_Size ); }


#ifdef __cpp_char8_t
	auto string() const {
		if constexpr( std::is_same_v<char_type, char8_t> )
		{
			return std::string( reinterpret_cast<char *>( m_pData ), m_Size );
		}
		if constexpr( not std::is_same_v<char_type, char8_t> )
		{
			return std::basic_string( m_pData, m_Size );
		}
	}
	auto c_str() const {
		if constexpr( std::is_same_v<char_type, char8_t> )
		{
			return reinterpret_cast<const char *>( m_pData );
		}
		if constexpr( not std::is_same_v<char_type, char8_t> )
		{
			return m_pData;
		}
	}
#else
	auto string() const { return std::basic_string( m_pData, m_Size ); }
	const char_type * c_str() const { return m_pData; }
#endif

	void reserve( size_t _New_Capacity )
	{
		if( m_Capacity )
		{
			if( _New_Capacity >= m_Capacity )
			{
				// We need to resize our buffer to fit both strings.
				m_Capacity = _New_Capacity + 1;

				auto _New_Buffer = (char_type *)FISSION_STRING_IMPL_MALLOC( m_Capacity * sizeof( char_type ) );
				
				::memcpy( _New_Buffer, m_pData, m_Size );
				_New_Buffer[m_Size] = static_cast<char_type>( 0 );

				FISSION_STRING_IMPL_FREE( m_pData );
				m_pData = _New_Buffer;

				return;
			}

			// There is enough capacity in current buffer.
			return;
		}

		// We have our string in local storage, we assume `m_Size < std::size(m_Storage)`

		if( _New_Capacity >= ( std::size( m_Storage ) ) )
		{
			m_Capacity = _New_Capacity + 1;

			m_pData = (char_type *)FISSION_STRING_IMPL_MALLOC( m_Capacity * sizeof( char_type ) );
			::memcpy( m_pData, m_Storage, m_Size );

			m_Size = m_Capacity - 1;
			m_pData[m_Size] = static_cast<char_type>( 0 );

			return;
		}
	}

public:

	// This is just awful, need a better way to do this!
	template <typename _String_Type>
	inline bool __equal( const _String_Type & _Right ) const
	{
		if constexpr( std::is_base_of_v<_string_impl, _String_Type> )
		{
			static_assert(
				sizeof(char_type)==sizeof(typename _String_Type::char_type),
				"Size of char types must be equal in order to compare the strings."
			);
			for( size_t i = 0; ; ++i )
			{
				// return false if they are not equal.
				if( _Right.m_pData[i] != m_pData[i] ) return false;

				// return true if both are null
				if( !( _Right.m_pData[i] | m_pData[i] ) ) return true;
			}
		}
		if constexpr( std::is_pointer_v<_String_Type> )
		{
			static_assert( 
				sizeof(char_type)==sizeof(std::remove_pointer_t<_String_Type>),
				"Size of char types must be equal in order to compare the strings."
			);
			for( size_t i = 0; ; ++i )
			{
				// return false if they are not equal.
				if( _Right[i] != m_pData[i] ) return false;

				// return true if both are null
				if( !( _Right[i] | m_pData[i] ) ) return true;
			}
		}
	}

protected:

	template <typename _String_Type>
	inline _String_Type __add( const _String_Type & _Right ) const
	{
		_String_Type _out = {};
		auto _size = m_Size + _Right.m_Size;
		_out.reserve( _size );
		::memcpy( _out.m_pData, m_pData, m_Size );
		::memcpy( _out.m_pData + m_Size, _Right.m_pData, _Right.m_Size );
		_out.m_Size = _size;
		_out.m_pData[_out.m_Size] = static_cast<char_type>( 0 );
		return _out;
	}

	template <typename _String_Type>
	inline _String_Type & __add_eq( const _String_Type & _Right )
	{
		auto _size = m_Size + _Right.m_Size;
		reserve( _size );
		::memcpy( m_pData + m_Size, _Right.m_pData, _Right.m_Size );
		m_Size = _size;
		m_pData[m_Size] = static_cast<char_type>( 0 );
		return *reinterpret_cast<_String_Type*>( this );
	}


private:
	static constexpr size_t s_Capacity = 24 / sizeof( char_type ); //!< Capacity of storage in characters.

protected:
	char_type * m_pData;
	size_t m_Capacity;
	size_t m_Size;
	char_type m_Storage[s_Capacity];

}; // class Fission::base::_string_impl



/* ======================= [String Helpers] ======================= */

template <typename _T>
static constexpr size_t strlen( const _T * const _Buf )
{
	const _T * _pLast = _Buf;
	while( *_pLast ) { ++_pLast; }
	return size_t( _pLast - _Buf );
}



/* ===================== [String Definitions] ===================== */

// Having each string type inherit from the base string class
// allows for specialization between the types without having
// the implementation copied into each class.
// 
// But this is still not perfect, as we need to repeat 
// all the constructors and operators.

class utf8_string : public _string_impl<char8_t>
{
	friend utf16_string;
	friend utf32_string;
public:
	utf8_string() :_string_impl() {}
	utf8_string( const _string_impl & _Parent ) :_string_impl( _Parent ) {}
	utf8_string( size_t _Count, char_type _Ch ) :_string_impl( _Count, _Ch ) {}
	utf8_string( const char_type * _Str, size_t _Count ) :_string_impl( _Str, _Count ) {}
	utf8_string( const char_type * const _Str ) :_string_impl( _Str, strlen( _Str ) ) {}

	// Since char8_t is not within the C++ Standard until C++20,
	//    we have to also support 'char' so that this class is
	//    consistant with lower C++ standards.
#ifdef __cpp_char8_t
	utf8_string( size_t _Count, char _Ch ) : _string_impl( _Count, static_cast<char8_t>( _Ch ) ) {}
	utf8_string( const char * _Str, size_t _Count ) : _string_impl( reinterpret_cast<const char8_t *>( _Str ), _Count ) {}
	utf8_string( const char * const _Str ) : _string_impl( reinterpret_cast<const char8_t *>( _Str ), strlen( _Str ) ) {}
	utf8_string( const std::string & _Src ) : _string_impl( reinterpret_cast<const char8_t *>( _Src.data() ), _Src.size() ) {}

	auto u8string() const { return std::u8string( m_pData, m_Size ); }
	const char_type * c_u8str() const { return m_pData; }
#endif

	utf8_string operator+( const utf8_string & _Right ) const { return _string_impl::__add( _Right ); }
	utf8_string & operator+=( const utf8_string & _Right ) { return _string_impl::__add_eq( _Right ); }

	utf16_string utf16() const;
	utf32_string utf32() const;

}; // class Fission::base::utf8_string

class utf16_string : public _string_impl<char16_t>
{
	friend utf8_string;
	friend utf32_string;
public:
	utf16_string() :_string_impl() {}
	utf16_string( const _string_impl & _Parent ) :_string_impl( _Parent ) {}
	utf16_string( size_t _Count, char_type _Ch ) :_string_impl( _Count, _Ch ) {}
	utf16_string( const char_type * _Str, size_t _Count ) :_string_impl( _Str, _Count ) {}
	utf16_string( const char_type * const _Str ) :_string_impl( _Str, strlen( _Str ) ) {}

	utf16_string operator+( const utf16_string & _Right ) const { return _string_impl::__add( _Right ); }
	utf16_string & operator+=( const utf16_string & _Right ) { return _string_impl::__add_eq( _Right ); }

	utf8_string utf8() const;
	utf16_string utf32() const;

}; // class Fission::base::utf16_string

class utf32_string : public _string_impl<char32_t>
{
	friend utf8_string;
	friend utf16_string;
public:
	utf32_string() :_string_impl() {}
	utf32_string( const _string_impl & _Parent ) :_string_impl( _Parent ) {}
	utf32_string( size_t _Count, char_type _Ch ) :_string_impl( _Count, _Ch ) {}
	utf32_string( const char_type * _Str, size_t _Count ) :_string_impl( _Str, _Count ) {}
	utf32_string( const char_type * const _Str ) :_string_impl( _Str, strlen( _Str ) ) {}

	utf32_string operator+( const utf32_string & _Right ) const { return _string_impl::__add( _Right ); }
	utf32_string & operator+=( const utf32_string & _Right ) { return _string_impl::__add_eq( _Right ); }

	utf8_string utf8() const;
	utf16_string utf16() const;

}; // class Fission::base::utf32_string

class ascii_string : public _string_impl<char>
{
public:
	ascii_string() :_string_impl() {}
	ascii_string( const _string_impl & _Parent ) :_string_impl( _Parent ) {}
	ascii_string( size_t _Count, char_type _Ch ) :_string_impl( _Count, _Ch ) {}
	ascii_string( const char_type * _Str, size_t _Count ) :_string_impl( _Str, _Count ) {}
	ascii_string( const char_type * const _Str ) :_string_impl( _Str, strlen( _Str ) ) {}

	ascii_string operator+( const ascii_string & _Right ) const { return _string_impl::__add( _Right ); }
	ascii_string & operator+=( const ascii_string & _Right ) { return _string_impl::__add_eq( _Right ); }

	utf8_string utf8() const;
	utf16_string utf16() const;
	utf32_string utf32() const;

}; // class Fission::base::ascii_string



/* ====================== [String Operators] ====================== */

static bool operator==( const utf8_string & _Left, const utf8_string & _Right ) noexcept { return _Left.__equal( _Right ); }
static bool operator==( const utf16_string & _Left, const utf16_string & _Right ) noexcept { return _Left.__equal( _Right ); }
static bool operator==( const utf32_string & _Left, const utf32_string & _Right ) noexcept { return _Left.__equal( _Right ); }
static bool operator==( const ascii_string & _Left, const ascii_string & _Right ) noexcept { return _Left.__equal( _Right ); }

static bool operator==( const char8_t * const _Left, const utf8_string & _Right ) { return _Right.__equal( _Left ); }
static bool operator==( const char16_t * const _Left, const utf16_string & _Right ) { return _Right.__equal( _Left ); }
static bool operator==( const char32_t * const _Left, const utf32_string & _Right ) { return _Right.__equal( _Left ); }
static bool operator==( const char * const _Left, const ascii_string & _Right ) { return _Right.__equal( _Left ); }

static bool operator==( const utf8_string & _Left, const char8_t * const _Right ) { return _Left.__equal( _Right ); }
static bool operator==( const utf16_string & _Left, const char16_t * const _Right ) { return _Left.__equal( _Right ); }
static bool operator==( const utf32_string & _Left, const char32_t * const _Right ) { return _Left.__equal( _Right ); }
static bool operator==( const ascii_string & _Left, const char * const _Right ) { return _Left.__equal( _Right ); }

#ifdef __cpp_char8_t
static bool operator==( const char * const _Left, const utf8_string & _Right ) { return _Right.__equal( _Left ); }
static bool operator==( const utf8_string & _Left, const char * const _Right ) { return _Left.__equal( _Right ); }
#endif



/* ======================= [String Literals] ======================= */

namespace string_literals
{
#ifdef __cpp_char8_t
	static utf8_string operator"" _utf8(const char * const _Str, size_t _Size)
	{
		return utf8_string( _Str, _Size );
	}
#endif
	static utf8_string operator"" _utf8(const char8_t * const _Str, size_t _Size)
	{
		return utf8_string( _Str, _Size );
	}
	static utf16_string operator"" _utf16(const char16_t * const _Str, size_t _Size)
	{
		return utf16_string( _Str, _Size );
	}
	static utf32_string operator"" _utf32(const char32_t * const _Str, size_t _Size)
	{
		return utf32_string( _Str, _Size );
	}
}



/* ===================== [String Conversions] ===================== */

inline utf16_string utf8_string::utf16() const
{
	utf16_string out;
	out.reserve( m_Size );
	char16_t * dst = out.data();

	const char8_t * src = m_pData;
	const char8_t * const end = m_pData + m_Size;

	uint32_t _ax, _bx, _cx, _dx;

	// perfection.
	while( src != end )

	if( *src & 0b10000000 )
	{
		if( ( *src & 0b11100000 ) == 0b11100000 )
		{
			if( ( *src & 0b11111000 ) == 0b11110000 )
			{
				if( end - src < 4 ) { ++dst; continue; }
				// 4 bytes
				_ax = *src++ & 0b00000111;
				_bx = *src++ & 0b00111111;
				_cx = *src++ & 0b00111111;
				_dx = *src++ & 0b00111111;

				_dx = ( _ax << 18 ) | ( _bx << 12 ) | ( _cx << 6 ) | _dx;
				_dx -= 0x10000;

				*dst++ = char16_t( ( _dx >> 10 ) + 0xD800 );
				*dst++ = char16_t( ( _dx & 0b11'1111'1111 ) + 0xDC00 );

				continue;
			}

			if( end - src < 3 ) { ++dst; continue; }
			// 3 bytes
			_ax = *src++ & 0b00001111;
			_bx = *src++ & 0b00111111;
			_cx = *src++ & 0b00111111;

			*dst++ = char16_t( ( _ax << 12 ) | ( _bx << 6 ) | _cx );

			continue;
		}

		if( end - src < 2 ) { ++dst; continue; }
		// 2 bytes
		_ax = *src++ & 0b00011111;
		_bx = *src++ & 0b00111111;

		*dst++ = char16_t( ( _ax << 6 ) | _bx );

		continue;
	}
	else

	*dst++ = char16_t( *src++ );

	*dst = char16_t( 0 );
	out.m_Size = dst - out.m_pData;
	return out;
}

inline utf8_string utf16_string::utf8() const
{
	utf8_string out;
	out.reserve( m_Size * 3u );
	char8_t * dst = out.data();

	const char16_t * src = m_pData;
	const char16_t * const end = m_pData + m_Size;

	uint32_t _ax, _bx, _cx, _dx;

	// ghetto switch statement.
	while( src != end )
	{
	if( *src > 0xD7FF )
	{
		if( end - src < 2 ) { ++dst; continue; }

		// 4 bytes

		// Store the codepoint in `_dx`
		_dx = ( ( *src++ - 0xD800 ) * 0x400 );
		_dx = ( ( *src++ - 0xDC00 ) + _dx + 0x10000 );

		// Break the codepoint down into parts to be put into utf8
		_ax = ( _dx & 0b11100'0000'0000'0000'0000 ) >> 18;
		_bx = ( _dx & 0b00011'1111'0000'0000'0000 ) >> 12;
		_cx = ( _dx & 0b00000'0000'1111'1100'0000 ) >> 6;
		_dx &= 0b111'111;

		// Set all the "non-codepoint" bits
		*dst++ = char8_t( 0b1111'0000 | _ax );
		*dst++ = char8_t( 0b1000'0000 | _bx );
		*dst++ = char8_t( 0b1000'0000 | _cx );
		*dst++ = char8_t( 0b1000'0000 | _dx );

		continue;
	}
	if( *src > 0b0000'0111'1111'1111 )
	{
		// 3 bytes
		_ax = (*src   & 0b1111'0000'0000'0000) >> 12;
		_bx = (*src   & 0b0000'1111'1100'0000) >> 6;
		_cx =  *src++ & 0b0000'0000'0011'1111;

		*dst++ = char8_t( 0b1110'0000 | _ax );
		*dst++ = char8_t( 0b1000'0000 | _bx );
		*dst++ = char8_t( 0b1000'0000 | _cx );

		continue;
	}
	if( *src > 0b0000'0000'0111'1111 )
	{
		// 2 bytes
		_ax = ( *src   & 0b0000'0111'1100'0000 ) >> 6;
		_bx =   *src++ & 0b0000'0000'0011'1111;

		*dst++ = char8_t( 0b1100'0000 | _ax );
		*dst++ = char8_t( 0b1000'0000 | _bx );

		continue;
	}
	/* Default: */
	*dst++ = char8_t( *src++ );
	}

	*dst = char8_t( 0 );
	out.m_Size = dst - out.m_pData;
	return out;
}


_FISSION_BASE_PUBLIC_END
