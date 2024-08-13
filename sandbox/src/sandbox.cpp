//#include <Fission/Platform.hpp>
#include <Fission/Core/Engine.hh>
//#include <Fission/Core/Window.hh>
//#include <Fission/Core/Graphics.hh>
//#include <Fission/Core/Renderer_2D.hh>
//#include <Fission/Base/Time.hpp>
//#include <Fission/Base/Math/Matrix.hpp>
//#include <Fission/Core/Scene.hh>
#include <Fission/Core/Input/Keys.hh>
//#include <Fission/Core/Font.hh>
#include <Fission/Core/Console.hh>
//#include <Fission/Platform/utils.h>
//#include "miniaudio.h"
//#include <cmath>
//#include <algorithm>
//#include <numeric>
#include "Path.hpp"

extern fs::Engine engine;
using namespace fs;

inline Path p;

struct Tetris_Scene : public fs::Scene {
	void on_update(double dt, std::vector<fs::Event> const& events, fs::Render_Context* ctx) override {
		auto const pos = v2f32(engine.window.mouse_position);

		for (auto const& event : events) {
			switch (event.type)
			{
			default:
			break; case fs::Event_Key_Down:
				if (event.key_down.key_id == keys::F11) {
					engine.window.set_mode((fs::Window_Mode)!(bool)engine.window.mode);
					engine.flags |= engine.fGraphics_Recreate_Swap_Chain;
				}
				else if (event.key_down.key_id == keys::Mouse_Left) {
					if (dp) {
						dragging = true;
					}
					else {
						p.add_point(pos);
						p.calc_total_length();
					}
				}
				else if (event.key_down.key_id == keys::Delete) {
					p.points.clear();
					p.points.emplace_back(100, 100);
					p.points.emplace_back(200, 200);
					p.points.emplace_back(300, 300);
					p.calc_total_length();
				}
				else if (event.key_down.key_id == keys::A) {
					show_control = !show_control;
				}
				else if (event.key_down.key_id == keys::S) {
					show_points = !show_points;
				}
				else if (event.key_down.key_id == keys::R) {
					p.resolve(rf32::from_topleft(engine.graphics.sc_extent.width, engine.graphics.sc_extent.height));
				}
			break; case fs::Event_Key_Up:
				if (event.key_up.key_id == keys::Mouse_Left) {
					dragging = false;
				}
			break;
			}
		}

		bp += 100.0f * float(dt);
		
		if (dragging) {
			*dp = pos;
			if (engine.modifier_keys & keys::Mod_Shift) {
				p.resolve(rf32::from_topleft(engine.graphics.sc_extent.width, engine.graphics.sc_extent.height));
			}
			p.calc_total_length();
		}
		else {
			auto d = p.data();
			float m = 1e20f;
			int u = 0;
			FS_FOR(p.count()) {
				auto const dif = pos - d[i];
				auto const dsq = dif.lensq();
				if (dsq < m) { m = dsq; u = i; }
			}
			dp = m < 70.0f*70.0f? d+u : nullptr;
		}

		rp.begin(ctx, colors::Black);

		VkDescriptorSet sets[] = { engine.transform_2d.set, engine.fonts.debug.texture };
		FS_VK_BIND_DESCRIPTOR_SETS(ctx->command_buffer, engine.textured_renderer_2d.pipeline_layout, (uint32_t)std::size(sets), sets);
		engine.textured_renderer_2d.set_font(&engine.fonts.debug);

		if (show_points) {
			if (dp)
				engine.renderer_2d.add_circle(*dp, 8.0f, colors::White);
			else
				engine.renderer_2d.add_circle(pos, 8.0f, colors::White);
		}

		p.add(engine.renderer_2d, show_control, show_points);

		if (p.total_length > 0.0f) {
			while (bp > p.total_length)
				bp -= p.total_length;

			engine.renderer_2d.add_circle(p.position(bp), 10.0f, colors::Gray);
			engine.renderer_2d.add_circle(p.position(bp), 8.0f, colors::DodgerBlue);

			float n = bp + 20.0f;
			if (n > p.total_length)
				n -= p.total_length;

			engine.renderer_2d.add_circle(p.position(n), 10.0f, colors::Gray);
			engine.renderer_2d.add_circle(p.position(n), 8.0f, colors::BlueViolet);
			
			FS_FOR(10) {
				n = n + 20.0f;
				if (n > p.total_length)
					n -= p.total_length;

				engine.renderer_2d.add_circle(p.position(n), 10.0f, colors::Gray);
				engine.renderer_2d.add_circle(p.position(n), 8.0f, colors::BlueViolet);
			}
		}

		engine.renderer_2d.draw(*ctx);

		rp.end(ctx);
	}
	virtual void on_resize() override {}

	Tetris_Scene() {
		auto& gfx = engine.graphics;
		auto samples = VK_SAMPLE_COUNT_1_BIT;

		rp.create(samples, true);

		console::register_command(FS_str("exit"), [](fs::string) { engine.flags &=~ engine.fRunning; });
		console::register_command(FS_str("save"), [](fs::string filename) {
			auto path = platform::path("paths")/filename.str();
			auto f = std::ofstream(path, std::ios::binary);
			f.exceptions(std::ios::badbit | std::ios::failbit);
			try {
				p.save(f);
			}
			catch (...) {
				console::print(FS_str("save failed"));
			}
		});
		console::register_command(FS_str("load"), [](fs::string filename) {
			auto path = platform::path("paths")/filename.str();
			auto f = std::ifstream(path, std::ios::binary);
			f.exceptions(std::ios::badbit | std::ios::failbit);
			try {
				p.load(f);
			}
			catch (...) {
				console::print(FS_str("load failed"));
			}
		});
		
		p.points.emplace_back(100, 100);
		p.points.emplace_back(200, 200);
		p.points.emplace_back(300, 300);

		p.calc_total_length();
	}
	~Tetris_Scene() override {
		rp.destroy();
	}

	fs::Render_Pass rp;
	v2f32* dp;
	bool dragging = false;
	bool show_control = true;
	bool show_points = true;
	float bp = 0.0f;
};

bool operator==(fs::string Left, char const* Right) {
	FS_FOR(Left.count) {
		if (Left.data[i] != Right[i]) {
			return false;
		}
	}
	return Left.count == strlen(Right);
}

fs::Scene* on_create_scene(fs::Scene_Key const& key) {
	using namespace fs;

	// Default:
	if (key.name().is_empty()) {
		return new Tetris_Scene;
	}
	else if (key.name() == "Tetris") {
		return new Tetris_Scene;
	}
	return nullptr; // scene id is undefined
}

fs::Defaults on_create() {
	engine.app_version = {0, 1, 7};
	engine.app_version_info = FS_str("vanilla/dev");
	engine.app_name         = FS_str("sandbox");
	return {
		.window_title = FS_str(u8" Sandbox reflexões 出展 うこそ"),
		.window_width = 800,
		.window_height = 800,
		.flags = fs::Defaults::fEnable_Graphics_Debugging,
	};
}
