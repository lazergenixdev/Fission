#include <Fission/Platform/EntryPoint.h>
#include <Fission/Simple2DLayer.h>
#include <random>

static std::mt19937 rng{ (unsigned int)time(nullptr) }; /* Use current time as seed for rng. */
static std::uniform_real_distribution<float> dist{ 0.0f, 1.0f };

// This will set the Destroy() function to just use "delete" function.
template <typename T>
struct DefaultDelete : public T { virtual void Destroy() override { delete this; } };

class BallScene : public DefaultDelete<Fission::IFScene>
{
public:
	virtual void OnCreate(Fission::FApplication * app) override
	{
		renderer2d = app->f_pEngine->GetRenderer<Fission::IFRenderer2D>("$internal2D");
	}
	virtual void OnUpdate(Fission::timestep dt) override
	{
		using Fission::rgb, Fission::hsv;
		// Update ball position
		pos += velocity * dt;

		radius = 40.0f * sinf( count += dt ) + 70.0f;

		// Collide with the top and bottom
		if( pos.y + radius >= 720.0f )
			pos.y = 720.0f - radius,
			velocity.y =- velocity.y,
			color = rgb( hsv( dist(rng), 1.0f, 1.0f ) );
		else if( pos.y - radius <= 0.0f )
			pos.y = radius,
			velocity.y =- velocity.y,
			color = rgb( hsv( dist(rng), 1.0f, 1.0f ) );

		// Collide with the left and right
		if( pos.x + radius >= 1280.0f )
			pos.x = 1280.0f - radius,
			velocity.x =- velocity.x,
			color = rgb( hsv( dist(rng), 1.0f, 1.0f ) );
		else if( pos.x - radius <= 0.0f )
			pos.x = radius,
			velocity.x =- velocity.x,
			color = rgb( hsv( dist(rng), 1.0f, 1.0f ) );

		// Draw the circle to the screen
		renderer2d->DrawCircle( pos, radius, {}, Fission::color( color, 0.5f ), 20.0f, Fission::StrokeStyle::Inside ); // inner glow
		renderer2d->DrawCircle( pos, radius, Fission::color( color, 0.5f ), {}, 20.0f, Fission::StrokeStyle::Outside ); // outer glow
		renderer2d->DrawCircle( pos, radius, color, 5.0f, Fission::StrokeStyle::Center ); // circle
		renderer2d->SetBlendMode( Fission::BlendMode::Add );
		renderer2d->Render();
		renderer2d->SetBlendMode( Fission::BlendMode::Normal );
	}
	virtual Fission::SceneKey GetKey() override { return {}; }
private:
	Fission::v2f32 velocity = { 150.0f, 300.0f };
	Fission::v2f32 pos      = { 100.0f, 100.0f };
	float          radius   = 50.0f;
	Fission::rgb   color    = Fission::colors::Red;
	float          count    = 0.0f;
	Fission::IFRenderer2D * renderer2d;
};

class BallApp : public DefaultDelete<Fission::FApplication>
{
public:
	void OnStartUp( CreateInfo * info )
	{
		info->window.title = std::format("Ball Demo [{}]", f_pEngine->GetVersionString());
		info->window.size = { 1280,720 };
		this->f_Name = "Balls";
		info->graphics.api = Fission::IFGraphics::API::DirectX11;
	}
	virtual Fission::IFScene * OnCreateScene( const Fission::SceneKey & key ) override
	{
		return new BallScene;
	}
};

Fission::FApplication * CreateApplication() {
	return new BallApp;
}