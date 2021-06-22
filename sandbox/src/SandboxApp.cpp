#include <Fission/Platform/EntryPoint.h>
#include <Fission/Base/Utility/Timer.h>
#include <random>

static std::mt19937 rng( (unsigned int)time(nullptr) ); /* Use current time as seed for rng. */
static std::uniform_real_distribution<float> dist{ 0.0f, 1.0f };

template <typename T>
struct DefaultDelete : public T { virtual void Destroy() override { delete this; } };

class BallLayer : public DefaultDelete<Fission::IFLayer>
{
public:
	Fission::color rand_color() { 
		return Fission::hsv_colorf( dist( rng ), 1.0f, 1.0f ); 
	}

	virtual void OnCreate( Fission::FApplication * app ) override
	{
		renderer = static_cast<Fission::IFRenderer2D *>( app->pEngine->GetRenderer( "$internal2D" ) );
	}

	virtual void OnUpdate() override
	{
		// Update ball position
		pos += velocity * timer.gets() * 3.0f;

		// Collide with the top and bottom
		if( pos.y + radius >= 720.0f )
			pos.y = 720.0f - radius, velocity.y =- velocity.y, color = rand_color();
		else if( pos.y - radius <= 0.0f )
			pos.y = radius,          velocity.y =- velocity.y, color = rand_color();

		// Collide with the left and right
		if( pos.x + radius >= 1280.0f )
			pos.x = 1280.0f - radius, velocity.x =- velocity.x, color = rand_color();
		else if( pos.x - radius <= 0.0f )
			pos.x = radius,           velocity.x =- velocity.x, color = rand_color();

		// Draw the circle to the screen
		renderer->DrawCircle( pos, radius, {0.0f}, Fission::color( color, 0.5f ), 20.0f, Fission::StrokeStyle::Inside ); // inner glow
		renderer->DrawCircle( pos, radius, Fission::color( color, 0.5f ), {0.0f}, 20.0f, Fission::StrokeStyle::Outside ); // outer glow
		renderer->DrawCircle( pos, radius, color, 5.0f, Fission::StrokeStyle::Center ); // circle
		renderer->SetBlendMode( Fission::BlendMode::Add );
		renderer->Render();
		renderer->SetBlendMode( Fission::BlendMode::Normal );
	}
private:
	Fission::IFRenderer2D * renderer;
	Fission::base::vector2f velocity = { 50.0f, 100.0f };
	Fission::base::vector2f pos      = { 100.0f, 100.0f };
	float                   radius   = 50.0f;
	Fission::color          color    = Fission::Colors::Red;
	Fission::simple_timer   timer;
};

class BallScene : public DefaultDelete<Fission::FScene>
{
public:
	BallScene() { PushLayer( new BallLayer ); }
};

class BounceBallApp : public DefaultDelete<Fission::FApplication>
{
public:
	virtual void OnStartUp( CreateInfo * info ) override
	{
		info->startScene = new BallScene;
		info->window.title = u8"Sandbox 🅱🅱🅱";
	}
};

Fission::FApplication * CreateApplication() {
	return new BounceBallApp;
}