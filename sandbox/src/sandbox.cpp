#include <Fission/Platform.hpp>
#include <Fission/Core/Engine.hh>
#include <Fission/Core/Window.hh>
#include <Fission/Core/Graphics.hh>
#include <Fission/Core/Renderer_2D.hh>
#include <Fission/Base/Time.hpp>
#include <Fission/Base/Math/Matrix.hpp>
#include <Fission/Core/Scene.hh>
#include <Fission/Core/Input/Keys.hh>
#include <Fission/Core/Font.hh>
#include <Fission/Core/Console.hh>
#include "stdio.h"
#include <random>
#include <freetype/freetype.h>
#include "renderer3d.h"

#define random() ((float)rand()/(float)RAND_MAX)

namespace eggs {
	void owl_house(fs::string input) {
		fs::console::println(FS_str("King of demons!"));
	}
	void sus_amogus(fs::string input) {
		fs::console::println(FS_str("red is lookin kinda sus rn tbh"));
	}
}

namespace ui {
	using id = int;

	struct Global_State {
		id hot = 0;
		id active = 0;
		bool mouse_went_up;
		bool mouse_went_down;
		fs::v2f32 mouse;
	};
	inline Global_State state;

	void set_hot(id _id) {
		if (state.active == 0) {
			state.hot = _id;
		}
	}

	bool Button(id _id, const char* text, fs::rf32 rect) {
		bool result = false;
		bool inside = rect[state.mouse];

		if (_id == state.active) {
			if (state.mouse_went_up) {
				if (_id == state.hot && inside)
					result = true;
				state.active = 0;
			}
		}
		else if (_id == state.hot) {
			if (state.mouse_went_down) {
				state.active = _id;
			}
		}

		if (inside) {
			set_hot(_id);
		}

		static constexpr fs::color active_color = fs::colors::OrangeRed;
		static constexpr fs::color hot_color    = fs::colors::LightBlue;
		static constexpr fs::color def_color    = fs::colors::DimGray;

		fs::color c = def_color;
		if (_id == state.active) {
			c = active_color;
		}
		else if (_id == state.hot) {
			c = hot_color;
		}

		engine.renderer_2d.add_rect(rect, c);

		return result;
	}
}

struct DVD_Bouncer {
	fs::v2f32 p, v;

	DVD_Bouncer() {
		p.x = random()*500.0f;
		p.y = random()*500.0f;
		float ang = fs::math::tau<float> * random();
		v.x = cosf(ang) * 200.0f;
		v.y = sinf(ang) * 200.0f;
	}

	void bounce(float& pos, float& vel, float radius, float max) {
		float right = max - radius;
		if (pos < radius) {
			pos = radius + (radius - pos);
			vel = -vel;
		}
		else if (pos > right) {
			pos = right - (pos - right);
			vel = -vel;
		}
	}

	void update(double dt, fs::v2f32 bounds, fs::Renderer_2D& r) {
		p += v * (float)dt;
		static constexpr float rad = 60.0f;
		bounce(p.x, v.x, rad, bounds.x);
		bounce(p.y, v.y, rad, bounds.y);
		r.add_rect(fs::rf32::from_center(p, 2*rad, 2*rad), fs::color(0, 1, 0, 0.5));
	}
};

struct Main_Menu_Scene : public fs::Scene {
	void on_update(double dt, std::vector<fs::Event> const& events, fs::Render_Context* ctx) override {
		static bool vsync = true;
		static constexpr float extended = 200;

		ui::state.mouse = (fs::v2f32)engine.window.mouse_position;

		for (auto const& event : events) {
			switch (event.type)
			{
			default:
			break; case fs::Event_Key_Down:
				if (event.key_down.key_id == fs::keys::Delete) {
					engine.next_scene_key = {};
					engine.flags |= engine.fChange_Scene;
				}
				if (event.key_down.key_id == fs::keys::Space) {
					r3d.t = 0.0f;
				}
				if (event.key_down.key_id == fs::keys::A) {
					engine.swap_chain_info.present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
					engine.flags |= engine.fGraphics_Recreate_Swap_Chain;
					vsync = false;
				}
				if (event.key_down.key_id == fs::keys::B) {
					engine.swap_chain_info.present_mode = VK_PRESENT_MODE_FIFO_KHR;
					engine.flags |= engine.fGraphics_Recreate_Swap_Chain;
					vsync = true;
				}
				if (event.key_down.key_id == fs::keys::Mouse_Left) {
					ui::state.mouse_went_down = true;
				}
			break; case fs::Event_Key_Up:
				if (event.key_down.key_id == fs::keys::Mouse_Left) {
					ui::state.mouse_went_up   = true;
				}
			break;
			}
		}

		using namespace fs;

	//	engine.debug_layer.add("flags = %u", (unsigned int)engine.modifier_keys);
	//	engine.debug_layer.add("console buffer count = %u", (unsigned int)engine.console_layer.buffer_view.count);

		engine.debug_layer.add("vsync = %s", FS_BTF(vsync));
#if USE_SUBPASS
		rp.begin(ctx, r3d.get_frame_buffer(ctx), colors::Black);
		
		r3d.draw(ctx, (float)dt);
		r3d.draw_postfx(ctx);
		
		rp.end(ctx);
#else
		rp.begin(ctx, r3d.get_frame_buffer(ctx), colors::Black);
		r3d.draw(ctx, (float)dt);
		rp.end(ctx);

		VkImageMemoryBarrier imageBarrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
		imageBarrier.image = r3d.depth_image;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
		imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		imageBarrier.subresourceRange.layerCount = 1;
		imageBarrier.subresourceRange.levelCount = 1;
		imageBarrier.srcAccessMask = VK_ACCESS_NONE;
		imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(ctx->command_buffer, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);

		VkRenderPassBeginInfo beginInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
		beginInfo.framebuffer = ctx->frame_buffer;
		beginInfo.renderArea = {.offset = {0,0}, .extent = ctx->gfx->sc_extent};
		beginInfo.renderPass = postrp;
		vkCmdBeginRenderPass(ctx->command_buffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
		r3d.draw_postfx(ctx);
		vkCmdEndRenderPass(ctx->command_buffer);
#endif

	//	if (ui::Button(0x45, "weh?", { 220,320,200,250 }))
	//		console::println(FS_str("jjjacksfilms"));
	//	if (ui::Button(0x46, "yes!", { 100,200,200,250 }))
	//		console::println(FS_str("jjjacksfilms2"));
	
	//	engine.renderer_2d.add_rect({400, 500, 400, 450}, rgb{1,0,0}, rgb{1,1,0});
	//	engine.renderer_2d.add_rect({500, 600, 400, 450}, rgb{1,1,0}, rgb{0,1,0});
	//	engine.renderer_2d.add_rect({600, 700, 400, 450}, rgb{0,1,0}, rgb{0,1,1});
	//	engine.renderer_2d.add_rect({700, 800, 400, 450}, rgb{0,1,1}, rgb{0,0,1});
	//	engine.renderer_2d.add_rect({800, 900, 400, 450}, rgb{0,0,1}, rgb{1,0,1});
	//	engine.renderer_2d.add_rect({900,1000, 400, 450}, rgb{1,0,1}, rgb{1,0,0});

		// reset
		ui::state.mouse_went_down = false;
		ui::state.mouse_went_up   = false;
	}

	void on_resize() override {
		r3d.recreate_framebuffers(engine.graphics, rp);
	}

	Main_Menu_Scene() {
		auto& gfx = engine.graphics;

		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = gfx.sc_format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		auto attachments = { colorAttachment };
		VkRenderPassCreateInfo renderPassInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
		renderPassInfo.attachmentCount = (fs::u32)attachments.size();
		renderPassInfo.pAttachments = attachments.begin();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		vkCreateRenderPass(gfx.device, &renderPassInfo, nullptr, &postrp);

		rp.create(engine.graphics);
		r3d.create(engine.graphics, rp, postrp);

		fs::console::register_command(FS_str("weh"), eggs::owl_house);
		fs::console::register_command(FS_str("sus"), eggs::sus_amogus);
	}
	~Main_Menu_Scene() override {
		r3d.destroy(engine.graphics);
		rp.destroy(engine.graphics);
		vkDestroyRenderPass(engine.graphics.device, postrp, nullptr);
	}

	Render_Pass_3D rp;
	VkRenderPass postrp;
	Renderer_3D r3d;
};

fs::Scene* on_create_scene(fs::Scene_Key const& key) {
	// if(fs::equal(key.id, "Level_Editor")) {
	// 	return new Level_Editor(level_path);
	// }
	return new Main_Menu_Scene;
}

fs::Defaults on_create() {
	engine.app_version = {0, 1, 7};
#ifdef DEBUG
	engine.app_version_info = FS_str("vanilla/dev");
#else
	engine.app_version_info = FS_str("vanilla");
#endif
	engine.app_name = FS_str("secret project :)");
	return {
		.window_title = FS_str("Not Malware."),
	//	.window_width = 160*5,
	//	.window_height = 90*5,
	};
}
