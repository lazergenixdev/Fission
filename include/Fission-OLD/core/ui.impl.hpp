#include <clay.h>
#include <Fission/core/engine.hpp>
#include <fmt/format.h>

namespace fs {
void add_ui_layout (
    Render_Context const&   render_context,
    Clay_RenderCommandArray render_commands,
    ::Render_Custom_Procedure render_custom,
    ::Get_Font_Procedure      get_font,
    void*                   user
) {
    bool first = true;
    for (int i = 0; i < render_commands.length; i++) {
        Clay_RenderCommand *renderCommand = &render_commands.internalArray[i];
        
        switch (renderCommand->commandType) {
            case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
                auto& b = renderCommand->boundingBox;
                auto rect = fs::rf32::from_topleft(b.x, b.y, b.width, b.height);
                auto& c = renderCommand->renderData.rectangle.backgroundColor;

                // renderCommand->config.rectangleElementConfig->cornerRadius.topLeft

                engine.renderer_2d.add_rect(rect, fs::color{c.r, c.g, c.b, c.a} / 255.0f);
            }

            break; case CLAY_RENDER_COMMAND_TYPE_BORDER: {
                auto& b = renderCommand->boundingBox;
                auto r = fs::rf32::from_topleft(b.x, b.y, b.width, b.height);

                auto const& border = renderCommand->renderData.border;
                auto& c = border.color;

                {
                    auto w = border.width.left;
                    engine.renderer_2d.add_rect({{b.x, b.x + w}, r.y}, fs::color{c.r, c.g, c.b, c.a} / 255.0f);
                }
                {
                    auto w = border.width.right;
                    engine.renderer_2d.add_rect({{r.x.high - w, r.x.high}, r.y}, fs::color{c.r, c.g, c.b, c.a} / 255.0f);
                }
                {
                    auto w = border.width.top;
                    engine.renderer_2d.add_rect({r.x, {b.y, b.y + w}}, fs::color{c.r, c.g, c.b, c.a} / 255.0f);
                }
                {
                    auto w = border.width.bottom;
                    engine.renderer_2d.add_rect({r.x, {r.y.high - w, r.y.high}}, fs::color{c.r, c.g, c.b, c.a} / 255.0f);
                }

            }
            
            break; case CLAY_RENDER_COMMAND_TYPE_TEXT: {
                auto& s = renderCommand->renderData.text.stringContents;
                auto text = fs::string(s.chars, s.length);
                auto& c = renderCommand->renderData.text.textColor;
                auto font = get_font(&renderCommand->renderData.text, user);
                
                // TODO: try and do some batching that isn't this shitty
                if (engine.textured_renderer_2d.current_font != font) {
                    //if (engine.textured_renderer_2d.current_font != nullptr)
                    //engine.textured_renderer_2d.draw(render_context);
                }
                auto& b = renderCommand->boundingBox;

                engine.textured_renderer_2d.set_font(font);
                engine.textured_renderer_2d.add_string(text, {b.x, b.y}, fs::color{c.r, c.g, c.b, c.a} / 255.0f);
                
                engine.bind_font(render_context.command_buffer, (Font_Static*)font);
                engine.renderer_2d.draw(render_context);
                engine.textured_renderer_2d.draw(render_context);
            }

            break; case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START: {
                if (first) {
                    engine.renderer_2d.draw(render_context);
                    engine.textured_renderer_2d.draw(render_context);
                    first = false;
                }

                VkRect2D scissor = {
                    .offset = { (fs::u32)renderCommand->boundingBox.x,     (fs::u32)renderCommand->boundingBox.y },
                    .extent = { (fs::u32)renderCommand->boundingBox.width, (fs::u32)renderCommand->boundingBox.height },
                };
                vkCmdSetScissor(render_context.command_buffer, 0, 1, &scissor);
            }
            
            break; case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END: {
                engine.renderer_2d.draw(render_context);
                engine.textured_renderer_2d.draw(render_context);
                engine.graphics.set_default_scissor(render_context.command_buffer);
            }

            break; case CLAY_RENDER_COMMAND_TYPE_IMAGE: {
                auto& b = renderCommand->boundingBox;
                auto rect = fs::rf32::from_topleft(b.x, b.y, b.width, b.height);
                auto& c = renderCommand->renderData.image.backgroundColor;

                engine.renderer_2d.add_rect(rect, fs::colors::black);
            }

            break; case CLAY_RENDER_COMMAND_TYPE_CUSTOM: {
                render_custom(renderCommand, user);
            }
        }
    }
}
}
