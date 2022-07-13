#pragma once
#include <Fission/Core/Scene.hh>
#include <Fission/Core/Application.hh>
#include <Fission/Simple2DLayer.h>
#include "DefaultDelete.h"

#if defined(DIST)
#define FISSION_ENABLE_DEBUG_UI 0
#else
#define FISSION_ENABLE_DEBUG_UI 1
#endif
#include <Fission/Core/UI/Debug.hh>

using v2f = Fission::v2f32;
using v2i = Fission::v2i32;
using rectf = Fission::base::rectf;

static float padding = 0.6f;
static Fission::IFEngine* g_engine = nullptr;

class Field
{
public:
	Field(float left, float top, float width, float height, v2i cell_count)
		: offset(left, top), size(width, height), cell_count(cell_count),
		cell_size(width/(float)cell_count.x, height/(float)cell_count.y),
		rect( rectf::from_topleft(left,top,width,height) )
	{}

	void Draw( Fission::IFRenderer2D * r2d, bool drawAllTiles )
	{
		if( drawAllTiles )
			for( int x = 0; x < (int)cell_count.x; ++x )
				for( int y = 0; y < (int)cell_count.y; ++y )
				{
					r2d->FillRect(
						rectf::from_topleft( { (float)x * cell_size.x + offset.x,(float)y * cell_size.y + offset.y }, cell_size ).expanded( -2.0f ),
						Fission::color( 0.1f, 0.1f, 0.1f, 0.5f )
					);
				}
		
		r2d->DrawRect( rect, Fission::Colors::White, 2.0f, Fission::StrokeStyle::Inside );
	}

	void DrawTile( Fission::IFRenderer2D * r2d, v2i location, Fission::color col )
	{
		r2d->FillRect( rectf::from_topleft( (v2f)location * cell_size + offset, cell_size ).expanded(-padding) , col );
	}

	v2f offset;
	v2f size;
	v2f cell_size;
	v2i cell_count;

	rectf rect;
};

class Snake
{
public:
	Snake( Field * field )
	{
		srand( (unsigned int)time( nullptr ) );
		body.reserve( 100 );

		const auto cells = field->cell_count;
		auto start = v2i( rand() % cells.x, rand() % cells.y );

		const v2i v = start - cells / 2;
		if( v.x * v.x > v.y * v.y )
			direction.x = v.x > 0 ? -1 : 1;
		else
			direction.y = v.y > 0 ? -1 : 1;

		body.emplace_back( start );
	}

	void Draw( Fission::IFRenderer2D * r2d, Field* field )
	{
		static constexpr Fission::color dead_color = Fission::Colors::Green;
		static constexpr auto color = Fission::hsva_colorf( Fission::rgb_colorf(Fission::Colors::LimeGreen) );
		static constexpr auto color2 = Fission::hsva_colorf( Fission::rgb_colorf(Fission::Colors::Teal) );

		static constexpr float factor = (color2.h - color.h) / 50.0f;

		if( dead )
		{
			for( auto &&pos : body )
				field->DrawTile( r2d, pos, dead_color );
		}
		else
		{
			auto c = color;
			for( auto && pos : body )
			{
				field->DrawTile( r2d, pos, c );
				c.h += factor;
			}
		}
	}

	void Update( float dt, Field * field )
	{
		if( dead ) return;

		t += dt;
		if( t >= delta )
		{
			// \/ Who wrote this comment? What framerate bugs? I am soo confused.. \/
			//
			// fuck those weird framerate bugs, will be the same as `t -= delta` in most cases.
			t = fmod( t, delta );

			auto next = body[0] + direction;
			if( CheckDead( next, field ) ) return;

			for( auto i = (int)body.size()-1; i > 0; --i )
				body[i] = body[i-1];
			body[0] = next;
		}
	}

	void Grow() { body.emplace_back( body.back() ); }
	bool GrowIf( v2i pos ) { if( body[0] == pos ) { body.emplace_back( body.back() ); return true; } return false; }

	void SetDirection( v2i dir ) { direction = dir; }
	bool IsDie() { return dead; }

	v2i GetPosition() const { return body[0]; }

private:
	bool CheckDead(v2i new_pos, Field* field)
	{
		// Wall Collision
		if( new_pos.x < 0 || new_pos.y < 0 || new_pos.x >= field->cell_count.x || new_pos.y >= field->cell_count.y )
			goto we_dead;

		// Self Collision
		for( int i = 0; i < (int)body.size() - 1; ++i )
		{
			if( new_pos == body[i] )
				goto we_dead;
		}

		return false;

	we_dead:
		return( dead = true );
	}

private:
	std::vector<v2i> body;
	v2i direction = { 0, 0 };
	float delta = 0.25f;
	float t = 0.0f;
	bool dead = false;
};

class GameLayer : public DefaultDelete<Fission::Simple2DLayer>
{
public:
	GameLayer()
		: field( 100.0f, 0.0f, 600.0f, 600.0f, { 15, 15 } ), snek( &field ), 
		food_pos(rand()%field.cell_count.x,rand()%field.cell_count.y)
	{}

	void OnUpdate( Fission::timestep dt )
	{
		snek.Update( dt, &field );

		if( snek.GrowIf( food_pos ) )
			food_pos = { rand() % field.cell_count.x,rand() % field.cell_count.y };

		Fission::UI::Debug::SliderFloat( "Padding", &padding, 0.0f, 10.0f );
		g_engine->GetDebug()->Text( "position: (%i, %i)", snek.GetPosition().x, snek.GetPosition().y );

		static bool drawtiles = true;
		Fission::UI::Debug::CheckBox( "Draw All Tiles", &drawtiles );

		field.DrawTile( m_pRenderer2D, food_pos, Fission::Colors::Red );

		snek.Draw( m_pRenderer2D, &field );
		field.Draw( m_pRenderer2D, drawtiles );
		m_pRenderer2D->Render();
	}

	virtual Fission::EventResult OnKeyDown( Fission::KeyDownEventArgs & args ) override
	{
		if( args.repeat ) return Fission::EventResult::Pass;
		switch( args.key )
		{
		case Fission::Keys::Left:  snek.SetDirection( {-1, 0} ); break;
		case Fission::Keys::Up:    snek.SetDirection( { 0,-1} ); break;
		case Fission::Keys::Right: snek.SetDirection( { 1, 0} ); break;
		case Fission::Keys::Down:  snek.SetDirection( { 0, 1} ); break;

		case Fission::Keys::Space: if( snek.IsDie() ) snek = Snake{ &field }; break;

		case Fission::Keys::Escape: { g_engine->EnterScene( {0} ); break; }
		default:break;
		}
		return Fission::EventResult::Pass;
	}

private:
	Field field;
	Snake snek;

	v2i food_pos;
};

class GameScene : public DefaultDelete<Fission::FMultiLayerScene>
{
public:
	GameScene(Fission::IFEngine * engine);

	virtual Fission::SceneKey GetKey() override { return {1}; }
};