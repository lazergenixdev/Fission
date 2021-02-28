#include <Fission/config.h>

/*
*  WARNING:
*   This API is ABSOLUTE GARBAGE and will be removed and replaced with
*      two systems, defualt debug ui and general purpose ui
*/

namespace Fission::UI
{
    struct WindowKey;    
    struct SliderKey;
    struct ButtonKey;

    class Key 
    {
    public:
        FISSION_API Key();
        FISSION_API Key( void * );
        FISSION_API ~Key();

    protected:
        void * context;
    };


    FISSION_API WindowKey PushWindow( const char * name, int x, int y, int width, int height );

    struct WindowKey : public Key
    {
        WindowKey() = default;

        FISSION_API WindowKey( const WindowKey & );

        FISSION_API WindowKey( void * );

        // Have to put this in each Key because the compiler
        //  is a piece of garbage that refuses to inherit this 
        //  from the base class
        FISSION_API WindowKey & operator=( WindowKey && );

        FISSION_API ButtonKey PushButton( const char * name, std::function<void()> action );
        FISSION_API SliderKey PushSliderFloat( const char * name, float * var, float min, float max, const char * fmt = "%.3f" );
        FISSION_API SliderKey PushSliderInt( const char * name, int * var, int min, int max, const char * fmt = "%.3f" );

    };

    struct SliderKey : public Key
    {
        SliderKey() = default;

        FISSION_API SliderKey( const SliderKey & );

        FISSION_API SliderKey( void * );

        // Have to put this in each Key because the compiler
        //  is a piece of garbage that refuses to inherit this 
        //  from the base class
        FISSION_API SliderKey & operator=( SliderKey && );

    };

    struct ButtonKey : public Key
    {
        ButtonKey() = default;

        FISSION_API ButtonKey( const ButtonKey & );

        FISSION_API ButtonKey( void * );

        // Have to put this in each Key because the compiler
        //  is a piece of garbage that refuses to inherit this 
        //  from the base class
        FISSION_API ButtonKey & operator=( ButtonKey && );

    };

} // namespace Fission::UI

