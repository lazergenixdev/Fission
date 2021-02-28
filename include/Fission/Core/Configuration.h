#include <LazerEngine/config.h>
#include <LazerEngine/Core/Window.h>

namespace lazer {

    class Configuration
    {
    public:
        struct WindowConfig
        {
            std::optional<std::string>      Style;
            std::optional<vec2i>            Position;
            std::optional<vec2i>            Size;
            std::optional<bool>             SavePosition;
            std::optional<bool>             SaveSize;
            std::optional<bool>             Fullscreen;
        };

        struct GraphicsConfig
        {
            std::optional<vec2i>            Resolution;
            std::optional<std::string>      API;
            std::optional<std::string>      FrameRate;
            std::optional<std::string>      FSAA;
        };

        LAZER_API static void Load() noexcept;

        LAZER_API static void Save() noexcept;


        LAZER_API static void SetWindowConfig( const Window::Properties & ) noexcept;

        LAZER_API static Window::Properties GetWindowConfig( const Window::Properties & fallback ) noexcept;

        LAZER_API static void SetGraphicsConfig( const GraphicsConfig & ) noexcept;

        LAZER_API static GraphicsConfig GetGraphicsConfig() noexcept;


    };

}
