#include "Fission/core.hpp"

//struct Scene: public fs::Scene
//{
//};

//App_Info::App_Info():
//    version      ( fs::make_compressed_version<0,1,7> ),
//    version_info ( "vanilla/dev" ),
//    name         ( "line-effect" )
//{}

auto on_create () -> fission::Defaults
{
    return {
        .window_title = "네, 한국어 정말 재미있어요! ❤🎄",
        .window_width  = 1280,
        .window_height =  720,
    };
}

void on_update (f64 dt, fission::array<fission::Event> events, fission::Render_Context const&)
{
	using namespace fission;

	local_persist rgba8 color = rgba8(255, 0, 0);

	for (auto const& e: events)
	{
		switch (e.type)
		{
		case Event_Key_Down: {
			log::info("key down: ", e.key_down.key_id);
			if (e.key_down.key_id == 'A')
				color.b += 1;
			else
				color.b -= 1;
		} break;
		}
	}

	engine.draw_data.add_triangle({-1.0f, -1.0f}, {0.0f, 0.0f}, {-1.0f, 0.0f}, color);

	local_persist f32 t = 0.0f;
	t += dt;

	engine.draw_data.push_index(engine.draw_data.current.vertex_count);
	engine.draw_data.push_index(engine.draw_data.current.vertex_count+1);
	engine.draw_data.push_index(engine.draw_data.current.vertex_count+2);
	engine.draw_data.push_vertex({{sinf(t), cosf(t)}, {}, rgba8(255, 0, 0)});
	engine.draw_data.push_vertex({{sinf(t + f32(TAU/3)), cosf(t + f32(TAU/3))}, {}, rgba8(0, 255, 0)});
	engine.draw_data.push_vertex({{sinf(t + f32(2*TAU/3)), cosf(t + f32(2*TAU/3))}, {}, rgba8(0, 0, 255)});
}

//auto on_create_scene (fs::Scene_Key const& key) -> fs::Scene *
//{
//    (void)key;
//    return new Scene;
//}