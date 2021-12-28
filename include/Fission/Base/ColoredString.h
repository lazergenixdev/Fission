#pragma once
#include <Fission/Base/Color.h>
#include <Fission/Base/String.h>
#include <optional>

_FISSION_BASE_PUBLIC_BEGIN


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
	colored_string( const string& str ) : str( str ), col() {}
	colored_string( const string& str, const color& col ) : str( str ), col( col ) {}

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
	colored_stream(size_t capacity = 1u) { m_strings.reserve( capacity ); }

	colored_stream( const colored_string& string ) :colored_stream() { m_strings.emplace_back( string ); }
	colored_stream( const string& string ) :colored_stream() { m_strings.emplace_back( string ); }
	colored_stream( const string& str, const color& col ) : colored_stream() { m_strings.emplace_back( str, col ); }

	colored_stream( const colored_string& string0, const colored_string& string1 ) : colored_stream( 2u )
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
static colored_string operator/( const char( &_Buffer )[_Size], const Colors::KnownColor& _Color )
{
	return colored_string( string( _Buffer, _Size - 1u ), _Color );
}
static colored_string operator/( const string& _String, const Colors::KnownColor& _Color )
{
	return colored_string( _String, _Color );
}
static colored_string operator/( const string& _String, const color& _Color )
{
	return colored_string( _String, _Color );
}

#include <type_traits>

struct formatable_string
{
	template <typename...Params>
	Fission::string operator()( Params&&...params )
	{
		Fission::string out;

		auto sz = snprintf( nullptr, 0, _Buf, params... );
		out.resize( sz );

		sprintf_s( (char* const)out.data(), sz + 1, _Buf, params... );

		return out;
	}

	const char* _Buf; size_t _Size;
};

static constexpr formatable_string operator""_format( const char* str, size_t len )
{
	return { str, len };
}

_FISSION_BASE_PUBLIC_END