#include "Fission/Platform/EntryPoint.h"
#include <random>

static Fission::IFRenderer2D * renderer;
static std::mt19937 rng;
static std::uniform_real_distribution<float> dist{ 0.0f, 1.0f };

class BallLayer : public Fission::IFLayer
{
public:
	Fission::color rand_color() { 
		return Fission::color( dist( rng ), dist( rng ), dist( rng ) ); 
	}

	virtual void OnCreate() override {}

	virtual void OnUpdate() override
	{
		// Update ball position
		pos += velocity * 0.05f;

		// Collide with the top and bottom
		if( pos.y + radius >= 720.0f )
			pos.y = 720.0f - radius, velocity.y = -velocity.y, color = rand_color();
		else if( pos.y - radius <= 0.0f )
			pos.y = radius,          velocity.y =- velocity.y, color = rand_color();

		// Collide with the left and right
		if( pos.x + radius >= 1280.0f )
			pos.x = 1280.0f - radius, velocity.x =- velocity.x, color = rand_color();
		else if( pos.x - radius <= 0.0f )
			pos.x = radius,           velocity.x =- velocity.x, color = rand_color();

		// Draw the circle to the screen
		renderer->DrawCircle( pos, radius, color, 5.0f, Fission::StrokeStyle::Inside );
		renderer->Render();
	}
	virtual void Destroy() override { delete this; }
private:
	Fission::base::vector2f velocity = { 50.0f, 100.0f };
	Fission::base::vector2f pos      = { 100.0f, 100.0f };
	float radius = 50.0f;
	Fission::color color = Fission::Colors::White;
};

class BallScene : public Fission::FScene
{
public:
	BallScene() { PushLayer( new BallLayer ); }
};

class SandboxApp : public Fission::FApplication
{
public:
	virtual void OnStartUp( CreateInfo * info ) override
	{
		info->startScene = new BallScene;
		char title[120];
		sprintf( title, "sandbox [%s]", pEngine->GetVersionString() );
		info->window.title = title;
		info->window.style = Fission::IFWindow::Style::BorderSizeable;

		Fission::CreateRenderer2D( &renderer );
		pEngine->RegisterRenderer( "main", renderer );
	}
	virtual void Destroy() override { delete this; }
};

Fission::FApplication * CreateApplication() {
	return new SandboxApp;
}