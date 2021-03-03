#pragma once
#include "Fission/config.h"

#include "Surface.h"

namespace Fission {

class surface_map : public ISerializable
{
public:
    static constexpr size_t MaxWidth = 8192;
    static constexpr size_t MaxHeight = 8192;

    struct value
    {
        const Surface * source;

        rectf uv;
    }

    FISSION_API surface_map();

    FISSION_API const value & operator[]( const std::string & key ) const;

    FISSION_API void emplace( const std::string & key, const Surface * surface );
    FISSION_API void remove( const std::string & key );

    FISSION_API void clear();

    FISSION_API virtual bool Load( const std::string & file ) override;
    FISSION_API virtual bool Save( const std::string & file ) const override;

    FISSION_API void SetMaxWidth( int _Width );
    FISSION_API void SetMaxHeight( int _Height );
    FISSION_API void SetMaxSize( vec2<int> _Size );

    enum BuildFlags
    {
        Build_OptimizeSize = 0,
        Build_OptimizeSpeed = 1,

        Build_Default = Build_OptimizeSize,
    }

    FISSION_API bool build( BuildFlags flags = Build_Default );

    // check to see if it contains any key-surface pairs
    FISSION_API bool empty() const;

    FISSION_API const Surface * get() const; // null if build failed or not called

    FISSION_API std::unique_ptr<Surface> && release();

private:
    std::unordered_map<std::string,value> map;
    
} // class Fission::surface_map

} // namespace Fission
