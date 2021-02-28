#include <Fission/config.h>
#include <Fission/Core/Window.h>

namespace Fission {

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

        FISSION_API static void Load() noexcept;

        FISSION_API static void Save() noexcept;


        FISSION_API static void SetWindowConfig( const Window::Properties & ) noexcept;

        FISSION_API static Window::Properties GetWindowConfig( const Window::Properties & fallback ) noexcept;

        FISSION_API static void SetGraphicsConfig( const GraphicsConfig & ) noexcept;

        FISSION_API static GraphicsConfig GetGraphicsConfig() noexcept;


    };

}
