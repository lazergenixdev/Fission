/**
*
* @file: SurfaceMap.h
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
#include <Fission/Core/Surface.hh>

// TODO: documentation

namespace Fission {

    // very closely related to JSON
    struct metadata
	{
        enum value_t
        {
            boolean, // contains a true/false
            number, // contains a number
            integer, // contains an integer
            string, // contains a string
            array, // contains an array
            table, // contains a table

            empty, // this metadata node contains no object
        };

        using const_iterator = std::unordered_map<std::string, metadata>::const_iterator;

        FISSION_API metadata();
        FISSION_API ~metadata();
		FISSION_API metadata( const metadata & src );
		FISSION_API metadata( metadata && src );
		FISSION_API metadata & operator=( const metadata & src );

        FISSION_API static metadata from_JSON( const std::string & json_str );

        FISSION_API metadata( bool _X );
        FISSION_API metadata( int _X );
		FISSION_API metadata( long long _X );
		FISSION_API metadata( float _X );
		FISSION_API metadata( double _X );
		FISSION_API metadata( const char * _X );
		FISSION_API metadata( const std::string & _X );

		FISSION_API metadata & operator[]( const std::string & key );
		FISSION_API const metadata & operator[]( const std::string & key ) const;

		FISSION_API metadata & operator[]( size_t index );
		FISSION_API const metadata & operator[]( size_t index ) const;

		FISSION_API size_t size() const;
		FISSION_API void resize(size_t n);

		FISSION_API const_iterator begin() const;
		FISSION_API const_iterator end() const;

        FISSION_API value_t type() const;

        FISSION_API const char * as_string() const;
        FISSION_API double as_number() const;
        FISSION_API long long as_integer() const;
        FISSION_API bool as_boolean() const;

        // helpers
        inline bool is_number() const { return ( m_Type == value_t::number || m_Type == value_t::integer ); }
        inline bool is_integer() const { return ( m_Type == value_t::number || m_Type == value_t::integer ); }
        inline bool is_boolean() const { return ( m_Type == value_t::boolean || m_Type == value_t::integer ); }
        inline bool is_string() const { return ( m_Type == value_t::string ); }
        inline bool is_table() const { return ( m_Type == value_t::table ); }
        inline bool is_array() const { return ( m_Type == value_t::array ); }
        inline bool is_empty() const { return ( m_Type == value_t::empty ); }

    private:

		using table_t = std::unordered_map<std::string, metadata>;
		using array_t = std::vector<metadata>;
		using number_t = double;
		using integer_t = long long;
		using string_t = std::string;

        union data_t
        {
            table_t * m_table;
            array_t * m_array;
            string_t * m_string;
            number_t m_number;
            integer_t m_integer;
            bool m_boolean;
        };

        value_t m_Type;
        data_t m_pData;
    };


	struct sub_surface
	{
        struct region_uv
        {
            base::rectf rel; // relative rect (UV texture coordinates)
            base::recti abs; // absolute rect (pixel coordinates)
            bool flipped = false;
        };

		const Surface * source;

        region_uv region;
		metadata meta;
	};

    class surface_map : public ISerializable
    {
    public:
		static constexpr int MaxWidth = 8192 / 2;
		static constexpr int MaxHeight = 8192 / 2;

		using iterator = std::unordered_map<std::string, sub_surface>::iterator;

        FISSION_API surface_map();
        FISSION_API ~surface_map() = default;

        // pointers are not guaranteed to be valid after later calls to emplace()
        FISSION_API const sub_surface * operator[]( const std::string & key ) const;
        FISSION_API sub_surface * operator[]( const std::string & key );

		FISSION_API iterator begin();
		FISSION_API iterator end();

        FISSION_API void emplace( const std::string & key, const Surface * surface );
        FISSION_API void remove( const std::string & key );

        FISSION_API void clear();

        FISSION_API virtual bool Load( const std::filesystem::path & file ) override;
        FISSION_API virtual bool Save( const std::filesystem::path & file ) const override;

        FISSION_API void set_metadata( const metadata & meta );
        FISSION_API metadata & get_metadata();
        FISSION_API const metadata & get_metadata() const;

        //FISSION_API void SetMaxWidth( int _Width );
        //FISSION_API void SetMaxHeight( int _Height );
        //FISSION_API void SetMaxSize( vec2<int> _Size );

        enum BuildFlags
        {
            Build_OptimizeSize = 0,
            Build_OptimizeSpeed = 1,

            Build_Default = Build_OptimizeSize,
        };

        FISSION_API bool build( BuildFlags flags = Build_Default );

        // check to see if it contains any key-surface pairs
        FISSION_API bool empty() const;

        FISSION_API const Surface * get() const; // null if build failed or not called

        FISSION_API std::unique_ptr<Surface> && release();

    private:
        base::size m_MaxSize;
        metadata m_MetaData;

        std::unordered_map<std::string,sub_surface> m_Map;
        std::unique_ptr<Surface> m_Surface;

    }; // class Fission::surface_map

} // namespace Fission
