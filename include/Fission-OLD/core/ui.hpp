#pragma once
#include <clay.h>
#include <Fission/core/engine.hpp>

using Render_Custom_Procedure = void (Clay_RenderCommand *render_command, void* user);
using Get_Font_Procedure      = auto (Clay_TextRenderData* text, void* user) -> fs::Font*;

namespace fs {
    extern void add_ui_layout (
        Render_Context const&   render_context,
        Clay_RenderCommandArray render_commands,
        Render_Custom_Procedure render_custom,
        Get_Font_Procedure      get_font,
        void*                   user
    );
}
