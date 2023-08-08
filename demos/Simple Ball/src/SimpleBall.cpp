#include <Fission/Core/Engine.hh>
#include <random>
#include <format>

static std::mt19937 rng{ (unsigned int)time(nullptr) }; /* Use current time as seed for rng. */
static std::uniform_real_distribution<float> dist{ 0.0f, 1.0f };

extern fs::Engine engine;

bool bounce(float& pos, float& vel, float radius, float max) {
	float right = max - radius;
	if (pos < radius) {
		pos = radius + (radius - pos);
		vel = -vel;
		return true;
	}
	if (pos > right) {
		pos = right - (pos - right);
		vel = -vel;
		return true;
	}
	return false;
}

class Ball_Scene : public fs::Scene
{
public:
	Ball_Scene() {
		rp.create(VK_SAMPLE_COUNT_1_BIT, true);
	}
	virtual ~Ball_Scene() override {
		rp.destroy();
	}
	virtual void on_update(double _dt, std::vector<fs::Event> const&, fs::Render_Context* ctx) override
	{
		float dt = (float)_dt;
		using namespace fs;
		// Update ball position
		pos += velocity * dt;

		radius = 40.0f * sinf( count += dt ) + 70.0f;

		// Collide with the top and bottom
		if (bounce(pos.y, velocity.y, radius, 720.0f))
			color = rgb( hsv( dist(rng), 1.0f, 1.0f ) );

		// Collide with the left and right
		if (bounce(pos.x, velocity.x, radius, 1280.0f))
			color = rgb(hsv(dist(rng), 1.0f, 1.0f));

		// Draw the circle to the screen
	//	renderer2d->DrawCircle( pos, radius, {}, color( color, 0.5f ), 20.0f, StrokeStyle::Inside ); // inner glow
	//	renderer2d->DrawCircle( pos, radius, color( color, 0.5f ), {}, 20.0f, StrokeStyle::Outside ); // outer glow
	//	renderer2d->DrawCircle( pos, radius, color, 5.0f, StrokeStyle::Center ); // circle
	//	renderer2d->SetBlendMode( Fission::BlendMode::Add );
	//	renderer2d->Render();
	//	renderer2d->SetBlendMode( Fission::BlendMode::Normal );
		rp.begin(ctx, fs::colors::Black);
		VK_GFX_BIND_DESCRIPTOR_SETS(ctx->command_buffer, engine.renderer_2d.pipeline_layout, 1, &engine.transform_2d.set);
		engine.renderer_2d.add_rect(rf32::from_center(pos, radius*2, radius*2), color);
		engine.renderer_2d.draw(*ctx);
		rp.end(ctx);
	}
private:
	fs::v2f32 velocity = { 150.0f, 300.0f };
	fs::v2f32 pos      = { 100.0f, 100.0f };
	float     radius   = 50.0f;
	fs::rgb   color    = fs::colors::Red;
	float     count    = 0.0f;
	fs::Render_Pass rp; // default render pass
};

fs::Scene* on_create_scene(fs::Scene_Key const& key) {
	return new Ball_Scene;
}

std::string title;

fs::Defaults on_create() {
	engine.app_name = FS_str("Balls");
	engine.app_version = {2, 2, 0};
	engine.app_version_info = FS_str("ball to the wall");
	title = std::format("Ball Demo [{}]", engine.get_version_string().str());
	return {
		.window_title = FS_str_std(title),
	};
}
