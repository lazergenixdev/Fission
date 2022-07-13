/**
 * @file String.hpp
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
#include <Fission/Base/Types.hpp>

#include <limits>
#include <string>
#include <format>

#ifndef _FISSION_STRING_TRACE
#define _FISSION_STRING_TRACE(...) (void)0
#endif

#define _FISSION_GUARD_STRING_ITERATORS \
static_assert( !std::is_same_v<_Encoding, utf8> && !std::is_same_v<_Encoding, utf16>, \
"Think about what you're doing, does it make sense?? `chr_iterator{}` is probably what you're looking for." );

__FISSION_BEGIN__

	
namespace base
{
	// Forward Declarations

	template <util::chr_encoding, util::allocator>
	class encoded_string;

	template <util::chr_encoding>
	class encoded_string_view;
}

namespace util
{
	template <typename T> static constexpr T null_character = T{};

	template <util::chr_encoding _Encoding>
	using def_alloc_from_encoding = Fission::allocator<typename _Encoding::value_type>;
}

namespace impl
{
	template <std::unsigned_integral S, typename T>
	static constexpr S strlen( T const* const start )
	{
		T const* end = start;
		for (; *end != util::null_character<T>; ++end);
		return end - start;
	}

	template <
		typename _Alloc,
		typename _ValueTy,
		typename _CValueTy,
		typename _PtrTy,
		typename _SizeTy
	> struct string_data final : public _Alloc
	{
		using base = _Alloc;
		using _ConstPtr = std::add_const_t<std::remove_pointer_t<_PtrTy>> *;

		// With a size of 16 bytes:	 total size = 32, enough to generally fit two on a cacheline.
		static constexpr _SizeTy storage_size = 16;

		// sizeof value_type should always equal sizeof cstr_type
		static constexpr _SizeTy storage_count = storage_size / sizeof _ValueTy;

		union _Buffer
		{
			_ValueTy   storage  [storage_count];
			_CValueTy cstorage  [storage_count];
			_PtrTy      ptr;
			_CValueTy* cptr;
		} data;

		_SizeTy size;
		_SizeTy capacity;


	public:
		constexpr bool _has_memory() const noexcept { return capacity >= storage_count; }

		static constexpr _SizeTy _pick_capacity( _SizeTy minimum ) noexcept { return minimum - minimum % 64 + 64; }

		constexpr _PtrTy get() const noexcept { return const_cast<_PtrTy>( _has_memory() ? data.ptr : data.storage ); }

		constexpr void construct( _ConstPtr src ) {
			if( size < storage_count ) {
				memcpy( data.storage, src, storage_count );
				capacity = storage_count - 1;
				return;
			}

			capacity = size;
			data.ptr = this->_Alloc::allocate( capacity + 1 );

			memcpy( data.ptr, src, size * sizeof _ValueTy );
			data.ptr[size] = util::null_character<_ValueTy>;
		}

		constexpr void fill( _ValueTy c ) {
			_ValueTy * dest;

			if( size < storage_count ) {
				capacity = storage_count - 1;
				dest = data.storage;
			} else {
				capacity = size;
				dest = data.ptr = this->_Alloc::allocate(capacity + 1);
			}

			for(size_t i = 0; i < size; ++i)
				dest[i] = c;
		}

		// Constructors:
	public:
		constexpr string_data() noexcept( std::is_nothrow_default_constructible_v<base> ):
			base(), size( 0 ), capacity( 0 ) { data.storage[0] = util::null_character<_ValueTy>; }

		constexpr string_data( base const& a ):
			base(a), size( 0 ) { data.storage[0] = util::null_character<_ValueTy>; }

		constexpr string_data( _ConstPtr p, _SizeTy const s ) noexcept( std::is_nothrow_default_constructible_v<base> ):
			base(), size( s ) { construct( p ); }

		constexpr string_data( _ConstPtr p, _SizeTy const s, base const& a ):
			base(a), size( s ) { construct( p ); }

		constexpr string_data( string_data const& src ) noexcept( std::is_nothrow_default_constructible_v<base> ):
			base(), size( src.size ) { construct( src.get() ); }

		constexpr string_data( string_data const& src, base const& a ):
			base(a), size( src.size ) { construct( src.get() ); }

		constexpr string_data( _SizeTy s, _ValueTy c ) noexcept( std::is_nothrow_default_constructible_v<base> ):
			base(), size( s ) { fill( c ); }

		constexpr string_data( _SizeTy s, _ValueTy c, base const& a ):
			base(a), size( s ) { fill( c ); }

		constexpr ~string_data() noexcept
		{
			if( capacity >= storage_count )
				_Alloc::deallocate( data.ptr, capacity + 1 );
		}

		constexpr string_data( string_data&& src ) noexcept
			: base(), size( src.size ), capacity( src.capacity )
		{
			if( src._has_memory() ) {
				data.ptr = src.data.ptr;
			} else {
				memcpy( data.storage, src.data.storage, storage_count );
			}
			src.data.storage[0] = util::null_character<_ValueTy>;
			src.size = 0;
			src.capacity = storage_count - 1;
		}

		constexpr string_data& operator=( string_data&& src ) noexcept
		{
			this-> ~string_data();
			this->size = src.size;
			this->capacity = src.capacity;

			if( src._has_memory() ) {
				this->data.ptr = src.data.ptr;
			}
			else {
				memcpy( this->data.storage, src.data.storage, storage_count );
			}
			src.data.storage[0] = util::null_character<_ValueTy>;
			src.capacity = storage_count - 1;
			src.size = 0;

			return *this;
		}

		constexpr string_data& operator=( string_data const& src ) noexcept
		{
			this-> ~string_data();
			this->size = src.size;
			this->capacity = 0;

			construct( src.get() );

			return *this;
		}

	}; // struct string_data<>

	// TODO: Add default implementation that gives readable compiler error.
	template <util::chr_encoding _From, util::chr_encoding _To>
	requires std::negation_v<std::is_same<_From, _To>> // requires encodings are different.
	static base::encoded_string<_To, util::def_alloc_from_encoding<_To>> convert( base::encoded_string_view<_From> const& _Msg );

} // namespace Fission::impl

namespace base
{
	template <
		util::chr_encoding _Encoding,
		util::allocator    _Allocator = util::def_alloc_from_encoding<_Encoding>
	> class encoded_string
	{
	protected:
		using _AllocTraitsTy = std::allocator_traits<_Allocator>;

	public:
		using traits_type = _Encoding;
		using allocator_type = _Allocator;

		using cstr_type  = typename _Encoding::cstr_type;
		using value_type = typename traits_type::value_type;
		using size_type  = typename _AllocTraitsTy::size_type;

		using difference_type = typename _AllocTraitsTy::difference_type;
		using pointer         = typename _AllocTraitsTy::pointer;
		using const_pointer   = typename _AllocTraitsTy::const_pointer;
		using reference       = typename std::add_lvalue_reference_t<value_type>;
		using const_reference = typename std::add_const_t<reference>;

		using iterator       = typename pointer;
		using const_iterator = typename const_pointer;

		using reverse_iterator       = typename std::reverse_iterator<iterator>;
		using const_reverse_iterator = typename std::reverse_iterator<const_iterator>;

		static constexpr size_type npos = (size_type)( -1 );

	public:
		/* ********************************************************************** */
		/* ******************  .... Default Constructors ....  ****************** */

		//! @brief Create an empty Encoded String.
		constexpr encoded_string() :m() { _FISSION_STRING_TRACE("default ctor"); }

		//! @brief Create an empty Encoded String with provided allocator.
		//! @param _Allocator	Allocator associated with this string.
		constexpr encoded_string( allocator_type const& _Allocator ) : m( _Allocator ) { _FISSION_STRING_TRACE(__FUNCSIG__); }



		/* ********************************************************************** */
		/* ******************  ... C String Constructors ....  ****************** */

		//! @brief Create a string from a C String.
		//! @param _Pointer		C String
		constexpr explicit encoded_string( value_type const* const _Pointer )
		: m( _Pointer, impl::strlen<size_type>(_Pointer) ) { _FISSION_STRING_TRACE(__FUNCSIG__); }
		
		//! @brief Create a string from a C String.
		//! @param _Pointer		C String
		//! @param _Allocator	Allocator associated with this string.
		constexpr explicit encoded_string( value_type const* const _Pointer, allocator_type const& _Allocator )
		: m( _Pointer, impl::strlen<size_type>(_Pointer), _Allocator ) { _FISSION_STRING_TRACE(__FUNCSIG__); }

		//! @brief Create a string from a C String.
		//! @param _Pointer		C String
		constexpr explicit encoded_string( cstr_type const* const _Pointer )
		: m( reinterpret_cast<value_type const* const>(_Pointer), impl::strlen<size_type>(_Pointer) ) { _FISSION_STRING_TRACE(__FUNCSIG__); }

		//! @brief Create a string from a C String.
		//! @param _Pointer		C String
		//! @param _Allocator	Allocator associated with this string.
		constexpr explicit encoded_string( cstr_type const* const _Pointer, allocator_type const& _Allocator )
		: m( reinterpret_cast<value_type const* const>(_Pointer), impl::strlen<size_type>(_Pointer), _Allocator ) { _FISSION_STRING_TRACE(__FUNCSIG__); }



		/* ********************************************************************** */
		/* ******************  ..... Buffer Constructors ....  ****************** */

		//! @brief Create a string from a fixed buffer.
		//! @param _Buffer	Buffer of characters to create from.
		//! @param _Size	Size of the buffer. (template)
		template <size_type _Size>
		constexpr encoded_string( value_type const( &_Buffer )[_Size] )
		: m( _Buffer, _Size - 1 ) { _FISSION_STRING_TRACE(__FUNCSIG__); }

		//! @brief Create a string from a fixed buffer.
		//! @param _Buffer		Buffer of characters to create from.
		//! @param _Size		Size of the buffer. (template)
		template <size_type _Size>
		constexpr encoded_string( cstr_type const( &_Buffer )[_Size] )
		: m( reinterpret_cast<const_pointer>(_Buffer), _Size - 1 ) { _FISSION_STRING_TRACE(__FUNCSIG__); }

		//! @brief Create a string from a fixed buffer.
		//! @param _Buffer		Buffer of characters to create from.
		//! @param _Allocator	Allocator associated with this string.
		//! @param _Size		Size of the buffer. (template)
		template <size_type _Size>
		constexpr encoded_string( value_type const( &_Buffer )[_Size], allocator_type const& _Allocator )
		: m( _Buffer, _Size - 1, _Allocator ) { _FISSION_STRING_TRACE(__FUNCSIG__); }

		//! @brief Create a string from a fixed buffer.
		//! @param _Buffer		Buffer of characters to create from.
		//! @param _Allocator	Allocator associated with this string.
		//! @param _Size		Size of the buffer. (template)
		template <size_type _Size>
		constexpr encoded_string( cstr_type const( &_Buffer )[_Size], allocator_type const& _Allocator )
		: m( reinterpret_cast<const_pointer>(_Buffer), _Size - 1, _Allocator ) { _FISSION_STRING_TRACE(__FUNCSIG__); }



		/* ********************************************************************** */
		/* ******************  .. String View Constructors ..  ****************** */

		//! @brief Create a string from a C String with the string length.
		//! @param _Pointer	C String
		//! @param _Length	Length of the string (not including null terminator)
		constexpr encoded_string( value_type const* const _Pointer, size_type _Length )
		: m( _Pointer, _Length ) { _FISSION_STRING_TRACE(__FUNCSIG__); }

		//! @brief Create a string from a C String with the string length.
		//! @param _Pointer	C String
		//! @param _Length	Length of the string (not including null terminator)
		constexpr encoded_string( cstr_type const* const _Pointer, size_type _Length )
		: m( reinterpret_cast<value_type const* const>(_Pointer), _Length ) { _FISSION_STRING_TRACE(__FUNCSIG__); }

		//! @brief Create a string from a C String with the string length.
		//! @param _Pointer		C String
		//! @param _Length		Length of the string (not including null terminator)
		//! @param _Allocator	Allocator associated with this string.
		constexpr encoded_string( value_type const* const _Pointer, size_type _Length, allocator_type const& _Allocator )
		: m( _Pointer, _Length, _Allocator ) { _FISSION_STRING_TRACE(__FUNCSIG__); }

		//! @brief Create a string from a C String with the string length.
		//! @param _Pointer		C String
		//! @param _Length		Length of the string (not including null terminator)
		//! @param _Allocator	Allocator associated with this string.
		constexpr encoded_string( cstr_type const* const _Pointer, size_type _Length, allocator_type const& _Allocator )
		: m( reinterpret_cast<value_type const* const>(_Pointer), _Length, _Allocator ) { _FISSION_STRING_TRACE(__FUNCSIG__); }

		//! @brief Create a string from a string view type.
		//! @param _String_View	String view
		template <util::string_view<value_type> SV>
		constexpr encoded_string( SV const& _String_View )
		: m( _String_View.data(), static_cast<size_type>(_String_View.size()) ) { _FISSION_STRING_TRACE(__FUNCSIG__); }

		//! @brief Create a string from a string view type.
		//! @param _String_View	String view
		template <util::string_view<cstr_type> SV>
		constexpr encoded_string( SV const& _String_View )
		: m( reinterpret_cast<value_type const* const>(_String_View.data()), static_cast<size_type>(_String_View.size()) ) { _FISSION_STRING_TRACE(__FUNCSIG__); }

		//! @brief Create a string from a string view type.
		//! @param _String_View	String view
		//! @param _Allocator	Allocator associated with this string.
		template <util::string_view<value_type> SV>
		constexpr encoded_string( SV const& _String_View, allocator_type const& _Allocator )
		: m( _String_View.data(), static_cast<size_type>(_String_View.size()), _Allocator ) { _FISSION_STRING_TRACE(__FUNCSIG__); }

		//! @brief Create a string from a string view type.
		//! @param _String_View	String view
		//! @param _Allocator	Allocator associated with this string.
		template <util::string_view<cstr_type> SV>
		constexpr encoded_string( SV const& _String_View, allocator_type const& _Allocator )
		: m( reinterpret_cast<value_type const* const>(_String_View.data()), static_cast<size_type>(_String_View.size()), _Allocator) { _FISSION_STRING_TRACE(__FUNCSIG__); }



		/* ********************************************************************** */
		/* ******************  ... Character Constructors ...  ****************** */

		//! @brief Create a string from a character and a count.
		//! @param _Count	Number of characters to fill string.
		//! @param _Ch		Character to fill string.
		constexpr encoded_string( size_type _Count, value_type _Ch )
		: m( _Count, _Ch ) { _FISSION_STRING_TRACE(__FUNCSIG__); }

		//! @brief Create a string from a character and a count.
		//! @param _Count	Number of characters to fill string.
		//! @param _Ch		Character to fill string.
		constexpr encoded_string( size_type _Count, cstr_type _Ch )
		: m( _Count, static_cast<value_type>(_Ch) ) { _FISSION_STRING_TRACE(__FUNCSIG__); }
		
		//! @brief Create a string from a character and a count.
		//! @param _Count		Number of characters to fill string.
		//! @param _Ch			Character to fill string.
		//! @param _Allocator	Allocator associated with this string.
		constexpr encoded_string( size_type _Count, cstr_type _Ch, allocator_type const& _Allocator )
		: m( _Count, _Ch, _Allocator ) { _FISSION_STRING_TRACE(__FUNCSIG__); }
		
		//! @brief Create a string from a character and a count.
		//! @param _Count		Number of characters to fill string.
		//! @param _Ch			Character to fill string.
		//! @param _Allocator	Allocator associated with this string.
		constexpr encoded_string( size_type _Count, value_type _Ch, allocator_type const& _Allocator )
		: m( _Count, static_cast<value_type>(_Ch), _Allocator ) { _FISSION_STRING_TRACE(__FUNCSIG__); }



		/* ********************************************************************** */
		/* ******************  ... Copy/Move Constructors ...  ****************** */

		//! @brief Create a string from an existing string.
		//! @param _Source	String to copy.
		constexpr encoded_string( encoded_string const& _Source ) = default;
		
		//! @brief Create a string from an existing string with another allocator.
		//! @param _Source		String to copy.
		//! @param _Allocator	Allocator associated with this string.
		constexpr encoded_string( encoded_string const& _Source, allocator_type const& _Allocator );


	public:
		constexpr allocator_type get_allocator() const { return static_cast<allocator_type&>(this->m); }

		//! @brief Check if string has characters.
		constexpr explicit operator bool() const noexcept { return static_cast<bool>(m.size); }

		//! @brief Check if string is empty.
		//! @return `true` if string is empty; `false` otherwise.
		constexpr bool empty() const noexcept { return !static_cast<bool>( m.size ); }

		constexpr       pointer data()       noexcept { return m.get(); }
		constexpr const_pointer data() const noexcept { return m.get(); }

		constexpr cstr_type const* c_str() const noexcept { return reinterpret_cast<cstr_type const*>(m.get()); }

		constexpr auto str() const noexcept { return std::basic_string<cstr_type>( c_str(), m.size ); }
		constexpr auto view() const noexcept { return std::basic_string_view<cstr_type>( c_str(), m.size ); }

		constexpr size_type   size() const noexcept { return m.size; }
		constexpr size_type length() const noexcept { return m.size; }

	public:
		constexpr       iterator  begin()       noexcept { _FISSION_GUARD_STRING_ITERATORS return m.get(); }
		constexpr const_iterator  begin() const noexcept { _FISSION_GUARD_STRING_ITERATORS return m.get(); }
		constexpr const_iterator cbegin() const noexcept { _FISSION_GUARD_STRING_ITERATORS return m.get(); }

		constexpr       iterator  end()       noexcept { _FISSION_GUARD_STRING_ITERATORS return m.get() + m.size; }
		constexpr const_iterator  end() const noexcept { _FISSION_GUARD_STRING_ITERATORS return m.get() + m.size; }
		constexpr const_iterator cend() const noexcept { _FISSION_GUARD_STRING_ITERATORS return m.get() + m.size; }
	
	public:
		void clear() {
			m.size = 0;
		}

		void pop_back() {
			m.get()[--m.size] = util::null_character<value_type>;
		}

		void resize( size_type _New_Size )
		{
			if(_New_Size <= m.capacity) {
				m.size = _New_Size;
				m.get()[m.size] = util::null_character<value_type>;
			} else {
				pointer temp = m.allocate( _New_Size + 1 );

				if( m._has_memory() )
				{
					memcpy( temp, m.data.ptr, m.size );
					m.deallocate( m.data.ptr, m.capacity + 1 );
				}
				else memcpy( temp, m.data.storage, m.size );

				m.size = _New_Size;
				m.capacity = m.size + 1;
				m.data.ptr = temp;
				m.data.ptr[m.size] = util::null_character<value_type>;
			}
		}

		void fast_resize( size_type _New_Size )
		{
			m.size = _New_Size;
			m.get()[m.size] = util::null_character<value_type>;
		}

		void reserve( size_type _New_Capacity )
		{
			if( m.capacity >= _New_Capacity )
				return;

			pointer temp = m.allocate( _New_Capacity + 1 );

			if( m._has_memory() )
			{
				memcpy( temp, m.data.ptr, m.size );
				m.deallocate( m.data.ptr, m.capacity + 1 );
			}
			else memcpy( temp, m.data.storage, m.size );

			m.capacity = _New_Capacity;
			m.data.ptr = temp;
			m.data.ptr[m.size] = util::null_character<value_type>;
		}

	public:
		inline size_type find_first_of( const value_type& _Char, size_type const _Position = 0u ) const
		{
			auto const data = m.get();
			auto const end = data + m.size;
			for( auto pc = data + _Position; pc != end; ++pc )
				if( *pc == _Char ) return size_type( pc - data );
			return std::string::npos;
		}

	public:
		constexpr auto operator<=>( const encoded_string &_Other ) const
		{
			auto const _Left = this->m.get();
			auto const _Right = _Other.m.get();
			size_t min = std::min( m.size, _Other.m.size );
			
			for(size_t i = 0; i < min; ++i)
				if( _Left[i] != _Right[i] )
					return _Left[i] <=> _Right[i];

			return m.size == _Other.m.size ? std::strong_ordering::equal : m.size <=> _Other.m.size;
		}

		constexpr bool operator==( encoded_string const& _Other ) const
		{
			if( m.size != _Other.m.size )
				return false;

			auto const _Left = this->m.get();
			auto const _Right = _Other.m.get();

			for( size_type i = 0; i < m.size; ++i )
				if( _Left[i] != _Right[i] )
					return false;

			return true;
		}

	public:
		template <util::chr_encoding _To>
		constexpr auto as() const { return impl::convert<_Encoding, _To>( *this ); }

		static constexpr auto encoding() { return _Encoding::name; }

		//! @brief NOT IMPLEMENTED
		constexpr bool validate() const noexcept;

	private:
		impl::string_data<allocator_type, value_type, cstr_type, pointer, size_type> m;

	}; // class encoded_string<>

	template <util::chr_encoding _Encoding>
	class encoded_string_view
	{
		using size_type = size_t;
		using value_type = typename _Encoding::value_type;
		using cstr_type = typename _Encoding::cstr_type;
	public:
		constexpr encoded_string_view() : m_Data(""), m_Size(0) {}
		template <util::allocator _Allocator>
		constexpr encoded_string_view(encoded_string<_Encoding, _Allocator> const& _Str) : m_Data(_Str.data()), m_Size(_Str.size()) {}

		constexpr encoded_string_view(value_type const* _Ptr, size_type _Size) : m_Data(_Ptr), m_Size(_Size) {}
		constexpr encoded_string_view(cstr_type const* _Ptr, size_type _Size) : m_Data((value_type const*)_Ptr), m_Size(_Size) {}

		constexpr explicit encoded_string_view(value_type const* _Ptr) : m_Data(_Ptr), m_Size(impl::strlen<size_type>(_Ptr)) {}
		constexpr explicit encoded_string_view(cstr_type const* _Ptr) : m_Data((value_type const*)_Ptr), m_Size(impl::strlen<size_type>(_Ptr)) {}

		template <size_t _Size>
		constexpr encoded_string_view(value_type const(&_Ptr)[_Size]) : m_Data(_Ptr), m_Size(_Size-1) {}
		template <size_t _Size>
		constexpr encoded_string_view(cstr_type  const(&_Ptr)[_Size]) : m_Data((value_type const*)_Ptr), m_Size(_Size-1) {}

		//! @brief Check if string is empty.
		//! @return `true` if string is empty; `false` otherwise.
		constexpr bool empty() const noexcept { return !static_cast<bool>( m_Size ); }

		constexpr auto  data() const { return m_Data; }
		constexpr auto c_str() const { return reinterpret_cast<cstr_type const*>(m_Data);}
		constexpr size_type size() const { return m_Size; }

		template <util::chr_encoding _To>
		constexpr auto as() const { return impl::convert<_Encoding, _To>( *this ); }

	private:
		value_type const* m_Data;
		size_type m_Size;
	}; // class encoded_string_view<>

} // namespace Fission::base

using string = base::encoded_string<default_encoding>;

using  utf8_string = base::encoded_string<utf8>;
using utf16_string = base::encoded_string<utf16>;
using utf32_string = base::encoded_string<utf32>;
using ascii_string = base::encoded_string<ascii>;

using string_view = base::encoded_string_view<default_encoding>;

using  utf8_string_view = base::encoded_string_view<utf8>;
using utf16_string_view = base::encoded_string_view<utf16>;
using utf32_string_view = base::encoded_string_view<utf32>;
using ascii_string_view = base::encoded_string_view<ascii>;


#define _FISSION_DEFINE_EXCEPTION(NAME,BASE) struct NAME:public BASE{NAME(const char * msg):BASE(msg){}}
#define _FISSION_DEFINE_LEAF_EXCEPTION(BASE,NAME,INFO) struct NAME:public BASE{NAME():BASE(INFO){}}

_FISSION_DEFINE_EXCEPTION( encoding_exception, std::exception );
_FISSION_DEFINE_EXCEPTION( utf8_decode_exception, encoding_exception );

_FISSION_DEFINE_LEAF_EXCEPTION( utf8_decode_exception, unexpected_continuation_byte, 	"unexpected continuation byte" );
_FISSION_DEFINE_LEAF_EXCEPTION( utf8_decode_exception, overlong_encoding, 				"overlong encoding" );
_FISSION_DEFINE_LEAF_EXCEPTION( utf8_decode_exception, invalid_codepoint, 				"invalid codepoint" );
_FISSION_DEFINE_LEAF_EXCEPTION( utf8_decode_exception, unexpected_end_of_string, 		"unexpected end of string" );
_FISSION_DEFINE_LEAF_EXCEPTION( utf8_decode_exception, unexpected_non_continuation_byte,"unexpected non-continuation byte before the end of the character" );

#undef _FISSION_DEFINE_EXCEPTION
#undef _FISSION_DEFINE_LEAF_EXCEPTION

namespace impl
{
	template <typename _Exceptions, util::chr_encoding _Encoding>
	class chr_iterator
	{
		using value_type = typename _Encoding::value_type;
	public:
		static_assert( !std::is_same_v<_Encoding, utf32>, "UTF-32 has constant length character, use begin() and end() instead." );
		static_assert( !std::is_same_v<_Encoding, ascii>, "ASCII has constant length character, use begin() and end() instead." );

		chr_iterator( const value_type *p, const value_type *end ) :p( p ), end( end ) { conv_char(); }

		template <typename T>
		bool operator!= ( T ) const { return p <= end; }


		chr_iterator &operator++()
		{
			if( p == end )
			{
				++p;
				return *this;
			}

			conv_char();

			return *this;
		}


		constexpr chr operator*() const { return stored; }

#define START_UTF_8        if constexpr( std::is_same_v<_Encoding, utf8> ){
#define START_UTF_16 }else if constexpr( std::is_same_v<_Encoding, utf16> ){
#define END }

#define _utfdecode_ThrowException(NAME) if constexpr ( _Exceptions::value ) throw NAME()
#define _utfdecode_ThrowExceptionIf(EXPRESSION,NAME) {if constexpr( _Exceptions::value ){if(EXPRESSION)throw NAME();}}
#define _utfdecode_IsAtOrPastEndOfString(P) P >= end
#define _utfdecode_CurrentLocation p
#define _utfdecode_ReturnCodepoint(X) stored = X; return;
#define _utfdecode_Advance(N) p += N
	private:
		constexpr void conv_char()
		{
			START_UTF_8
#				include "impl/utf8-decode.inl"
			START_UTF_16
#				include "impl/utf16-decode.inl"
			END
		}

#undef _utfdecode_ReturnCodepoint 
#undef _utfdecode_CurrentLocation
#undef _utfdecode_IsAtOrPastEndOfString
#undef _utfdecode_ThrowExceptionIf
#undef _utfdecode_ThrowException

#undef START_UTF_8
#undef START_UTF_16
#undef END

	private:
		chr stored;
		const value_type *p;
		const value_type *end;
	};

} // namespace Fission::impl


// `chr_iterator` is only really designed to be used in 'for each' loops.
template <util::chr_encoding _Encoding = default_encoding>
struct chr_iterator
{
public:
	struct chr_iterator_w_exceptions
	{
		using value_type = typename _Encoding::value_type;
		using iterator = impl::chr_iterator<std::true_type, _Encoding>;

		chr_iterator_w_exceptions( chr_iterator &src )
			: m_pBegin( src.m_pBegin ), m_pEnd( src.m_pEnd )
		{}

		iterator begin() noexcept { return iterator{ m_pBegin,m_pEnd }; }
		util::empty_t end() noexcept { return {}; }
	private:
		const value_type *m_pBegin, *m_pEnd;
	};

public:
	using value_type = typename _Encoding::value_type;
	using iterator = impl::chr_iterator<std::false_type, _Encoding>;

	chr_iterator( const base::encoded_string_view<_Encoding> &s )
		: m_pBegin( s.data() ), m_pEnd( m_pBegin + s.size() )
	{}

	template <util::allocator _Allocator>
	chr_iterator( const base::encoded_string<_Encoding, _Allocator> &s )
		: m_pBegin( s.data() ), m_pEnd( m_pBegin + s.size() )
	{}

	chr_iterator_w_exceptions exceptions() { return *this; }

	iterator begin() noexcept { return iterator{ m_pBegin,m_pEnd }; }
	util::empty_t end() noexcept { return {}; }

private:
	const value_type *m_pBegin, *m_pEnd;
};

namespace impl
{
	template <>
	static inline base::encoded_string<utf16> convert( base::encoded_string_view<utf8> const& _Msg )
	{
		base::encoded_string<utf16> out;
		out.reserve( _Msg.size() );
		
		char16_t * dst = out.data();

		const char8_t * src = _Msg.data();
		const char8_t * const end = src + _Msg.size();

		uint32_t _ax, _bx, _cx, _dx;

		// perfection.
		while( src != end )

		if( *src & 0b10000000 )
		{
			if( ( *src & 0b11100000 ) == 0b11100000 )
			{
				if( ( *src & 0b11111000 ) == 0b11110000 )
				{
					if( end - src < 4 ) { ++src; continue; }
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

				if( end - src < 3 ) { ++src; continue; }
				// 3 bytes
				_ax = *src++ & 0b00001111;
				_bx = *src++ & 0b00111111;
				_cx = *src++ & 0b00111111;

				*dst++ = char16_t( ( _ax << 12 ) | ( _bx << 6 ) | _cx );

				continue;
			}

			if( end - src < 2 ) { ++src; continue; }
			// 2 bytes
			_ax = *src++ & 0b00011111;
			_bx = *src++ & 0b00111111;

			*dst++ = char16_t( ( _ax << 6 ) | _bx );

			continue;
		}
		else

		*dst++ = char16_t( *src++ );

		out.fast_resize( dst - out.data() );

		return out;
	}
	template <>
	static inline base::encoded_string<utf8> convert( base::encoded_string_view<utf16> const& _Msg )
	{
		base::encoded_string<utf8> out;
		out.reserve( _Msg.size() * 3u );

		char8_t* dst = out.data();

		const char16_t* src = _Msg.data();
		const char16_t* const end = src + _Msg.size();

		uint32_t _ax, _bx, _cx, _dx;

		// ghetto switch statement.
		while( src != end )
		{
			if( *src > 0xD7FF )
			{
				if( end - src < 2 ) { ++src; continue; }

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
				_ax = ( *src & 0b1111'0000'0000'0000 ) >> 12;
				_bx = ( *src & 0b0000'1111'1100'0000 ) >> 6;
				_cx = *src++ & 0b0000'0000'0011'1111;

				*dst++ = char8_t( 0b1110'0000 | _ax );
				*dst++ = char8_t( 0b1000'0000 | _bx );
				*dst++ = char8_t( 0b1000'0000 | _cx );

				continue;
			}
			if( *src > 0b0000'0000'0111'1111 )
			{
				// 2 bytes
				_ax = ( *src & 0b0000'0111'1100'0000 ) >> 6;
				_bx = *src++ & 0b0000'0000'0011'1111;

				*dst++ = char8_t( 0b1100'0000 | _ax );
				*dst++ = char8_t( 0b1000'0000 | _bx );

				continue;
			}
			/* Default: */
			*dst++ = char8_t( *src++ );
		}

		out.fast_resize( dst - out.data() );

		return out;
	}
	

	template<class T, class CharT>
	struct formatter
	{
		template <typename FormatParseContext>
		auto parse( FormatParseContext& pc )
		{
			// parse formatter args like padding, precision if you support it
			return pc.end(); // returns the iterator to the last parsed character in the format string, in this case we just swallow everything
		}

		template<typename FormatContext>
		auto format( T const& str, FormatContext& fc )
		{
			return std::format_to( fc.out(), "{}", std::basic_string_view<CharT>{str.c_str(), str.size()} );
		}
	};

} // namespace Fission::impl

namespace impl
{
	namespace string
	{
		template <typename T>
		static constexpr size_t size( std::basic_string<T> const& str ) {
			return str.size();
		}
		template <typename T>
		static constexpr size_t size( base::encoded_string<T> const& str ) {
			return str.size();
		}
		template <typename T, size_t S>
		static constexpr size_t size( T( &str )[S] ) {
			return S - 1;
		}

		template <typename T>
		static constexpr auto data( std::basic_string<T> const& str ) {
			return str.data();
		}
		template <typename T>
		static constexpr auto data( base::encoded_string<T> const& str ) {
			return (typename T::cstr_type*)str.data();
		}
		template <typename T, size_t S>
		static constexpr auto data( T( &str )[S] ) {
			return str;
		}

		template <typename...Params>
		static constexpr size_t accumulated_size( Params&&...params )
		{
			return ( ( size( std::forward<Params>(params) ) ) + ... );
		}

		template <typename O, typename T>
		static constexpr void insert( O*& dest, T const& s )
		{
			auto src = data( s );
			for( size_t i = 0; i < size(s); ++i )
				*dest++ = src[i];
		}
	} // namespace Fission::impl::string

	template <typename First, typename...Rest>
	struct first_type { using type = First; };
	template <typename...Params>
	using first_t = first_type<Params...>::type;

	template <typename T> struct char_type_of { using type = void; };
	template <typename T> struct char_type_of<T *> { using type = T; };
	template <typename T> struct char_type_of<T const*> { using type = T; };
	template <typename T> struct char_type_of<T const* const> { using type = T; };
	template <typename T, size_t S> struct char_type_of<T const(&)[S]> { using type = T; };
	template <typename T, size_t S> struct char_type_of<T (&)[S]> { using type = T; };
	template <typename T> struct char_type_of<std::basic_string<T>> { using type = T; };
	template <typename T> struct char_type_of<base::encoded_string<T>> { using type = typename T::cstr_type; };

} // namespace Fission::impl

template <typename...Params>
static string cat( Params&&...params )
{
	size_t const s = impl::string::accumulated_size( std::forward<Params>(params)... );
	string out;
	out.resize( s );
	using CHAR = impl::char_type_of<impl::first_t<Params...>>::type;
	CHAR* offset = reinterpret_cast<CHAR*>( out.data() );
	( ( impl::string::insert(offset, params) ), ... );
	return out;
}

__FISSION_END__


namespace std
{
	template<class Encoding>
	struct formatter<Fission::base::encoded_string<Encoding>, typename Encoding::cstr_type>
		: public Fission::impl::formatter<Fission::base::encoded_string<Encoding>, typename Encoding::cstr_type>
	{};
}