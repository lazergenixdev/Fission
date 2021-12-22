#include <Fission/Platform/EntryPoint.h>
#include <Fission/Base/Utility/Timer.h>
#include <Fission/Simple2DLayer.h>
#include <random>

static std::mt19937 rng( (unsigned int)time(nullptr) ); /* Use current time as seed for rng. */
static std::uniform_real_distribution<float> dist{ 0.0f, 1.0f };

template <typename T>
struct DefaultDelete : public T { virtual void Destroy() override { delete this; } };

class BallLayer : public DefaultDelete<Fission::Simple2DLayer>
{
public:
	Fission::color rand_color() { 
		return Fission::hsv_colorf( dist( rng ), 1.0f, 1.0f ); 
	}

	void OnUpdate(Fission::timestep dt)
	{
		// Update ball position
		pos += velocity * dt * 3.0f;

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
		m_pRenderer2D->DrawCircle( pos, radius, {0.0f}, Fission::color( color, 0.5f ), 20.0f, Fission::StrokeStyle::Inside ); // inner glow
		m_pRenderer2D->DrawCircle( pos, radius, Fission::color( color, 0.5f ), {0.0f}, 20.0f, Fission::StrokeStyle::Outside ); // outer glow
		m_pRenderer2D->DrawCircle( pos, radius, color, 5.0f, Fission::StrokeStyle::Center ); // circle
		m_pRenderer2D->SetBlendMode( Fission::BlendMode::Add );
		m_pRenderer2D->Render();
		m_pRenderer2D->SetBlendMode( Fission::BlendMode::Normal );
	}
private:
	Fission::base::vector2f velocity = { 50.0f, 100.0f };
	Fission::base::vector2f pos      = { 100.0f, 100.0f };
	float                   radius   = 50.0f;
	Fission::color          color    = Fission::Colors::Red;
};

class BallScene : public DefaultDelete<Fission::FMultiLayerScene>
{
public:
	BallScene() { PushLayer( new BallLayer ); }

	virtual Fission::SceneKey GetKey() override { return {}; }
};

class BounceBallApp : public DefaultDelete<Fission::FApplication>
{
public:
	void OnStartUp( CreateInfo * info )
	{
		char title[100];
		sprintf( title, "Bouncing Ball Demo [%s]", pEngine->GetVersionString() );
		info->window.title = title;
		info->window.size = { 1280,720 };
		strcpy(info->name_utf8, "Balls");
		strcpy(info->version_utf8, "1.0.0");
	}
	virtual Fission::IFScene * OnCreateScene( const Fission::SceneKey & key ) override
	{
		return new BallScene;
	}
};

Fission::FApplication * CreateApplication() {
	return new BounceBallApp;
}