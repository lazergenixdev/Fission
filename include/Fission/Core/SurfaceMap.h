#pragma once
#include "Fission/config.h"

#include "Surface.h"

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
            null, // contains null

            empty, // this metadata node contains no object
        };

        using const_iterator = std::unordered_map<std::string, metadata>::const_iterator;

        FISSION_API metadata();
        FISSION_API ~metadata();
		FISSION_API metadata( const metadata & src );
		FISSION_API metadata( metadata && src );
		FISSION_API metadata & operator=( const metadata & src );

        FISSION_API metadata & from_JSON( const std::string & json_str );

        FISSION_API metadata( bool _X );
        FISSION_API metadata( int _X );
		FISSION_API metadata( long long _X );
		FISSION_API metadata( float _X );
		FISSION_API metadata( double _X );
		FISSION_API metadata( const char * _X );
		FISSION_API metadata( const std::string & _X );
		FISSION_API metadata( nullptr_t _X );

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
        inline bool is_number() const { return ( m_Type == number || m_Type == integer ); }
        inline bool is_integer() const { return ( m_Type == number || m_Type == integer ); }
        inline bool is_boolean() const { return ( m_Type == boolean || m_Type == integer ); }
        inline bool is_table() const { return ( m_Type == table ); }
        inline bool is_array() const { return ( m_Type == array ); }
        inline bool is_null() const { return ( m_Type == null ); }
        inline bool empty() const { return ( m_Type == value_t::empty ); }

    private:

        value_t m_Type;
        void * m_pData;
    };


	struct sub_surface
	{
        struct region_uv
        {
            rectf rel; // relative rect (UV texture coordinates)
            recti abs; // absolute rect (pixel coordinates)
            bool flipped = false;
        };

		const Surface * source;

        region_uv region;
		metadata meta;
	};

    class surface_map : public ISerializable
    {
    public:
		static constexpr int MaxWidth = 8192/2;
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

        FISSION_API virtual bool Load( const file::path & file ) override;
        FISSION_API virtual bool Save( const file::path & file ) const override;

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
        vec2i m_MaxSize;
        metadata m_MetaData;

        std::unordered_map<std::string,sub_surface> m_Map;
        std::unique_ptr<Surface> m_Surface;

    }; // class Fission::surface_map

} // namespace Fission
