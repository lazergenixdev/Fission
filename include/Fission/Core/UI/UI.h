#include <LazerEngine/config.h>

/*
*  WARNING:
*   This API is ABSOLUTE GARBAGE and will be removed and replaced with
*      two systems, defualt debug ui and general purpose ui
*/

namespace lazer::UI
{
    struct WindowKey;    
    struct SliderKey;
    struct ButtonKey;

    class Key 
    {
    public:
        LAZER_API Key();
        LAZER_API Key( void * );
        LAZER_API ~Key();

    protected:
        void * context;
    };


    LAZER_API WindowKey PushWindow( const char * name, int x, int y, int width, int height );

    struct WindowKey : public Key
    {
        WindowKey() = default;

        LAZER_API WindowKey( const WindowKey & );

        LAZER_API WindowKey( void * );

        // Have to put this in each Key because the compiler
        //  is a piece of garbage that refuses to inherit this 
        //  from the base class
        LAZER_API WindowKey & operator=( WindowKey && );

        LAZER_API ButtonKey PushButton( const char * name, std::function<void()> action );
        LAZER_API SliderKey PushSliderFloat( const char * name, float * var, float min, float max, const char * fmt = "%.3f" );
        LAZER_API SliderKey PushSliderInt( const char * name, int * var, int min, int max, const char * fmt = "%.3f" );

    };

    struct SliderKey : public Key
    {
        SliderKey() = default;

        LAZER_API SliderKey( const SliderKey & );

        LAZER_API SliderKey( void * );

        // Have to put this in each Key because the compiler
        //  is a piece of garbage that refuses to inherit this 
        //  from the base class
        LAZER_API SliderKey & operator=( SliderKey && );

    };

    struct ButtonKey : public Key
    {
        ButtonKey() = default;

        LAZER_API ButtonKey( const ButtonKey & );

        LAZER_API ButtonKey( void * );

        // Have to put this in each Key because the compiler
        //  is a piece of garbage that refuses to inherit this 
        //  from the base class
        LAZER_API ButtonKey & operator=( ButtonKey && );

    };

} // namespace lazer::UI

