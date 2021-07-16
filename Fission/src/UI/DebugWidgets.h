#pragma once
#include <Fission/Base/TimeStep.h>
#include <Fission/Base/Rect.h>

#include <Fission/Core/Input/Event.hh>
#include <Fission/Core/Graphics/Renderer2D.hh>

#define _neutron_key_primary_mouse   Fission::Keys::Mouse_Left
#define _neutron_key_secondary_mouse Fission::Keys::Mouse_Right

#define _neutron_char_type    char32_t
#define _neutron_key_type     Fission::Keys::Key
#define _neutron_cursor_type  Fission::Cursor*

#define _neutron_point_type   Fission::base::vector2i
#define _neutron_rect_type    Fission::base::recti
#define _neutron_vector_type  std::vector

#include <Fission/neutron.hpp>

namespace Fission
{
	struct StyleInformation
	{
		float LeftPadding = 5.0f;
		float RightPadding = 5.0f;

		float VerticalPadding = 4.0f;

		float MaximumLabelWidth = 115.0f;

		float LeftTextBoxPadding = 2.0f;
	};

	static StyleInformation g_DebugStyle;

	struct WindowContext
	{
		base::rectf rect;
		IFRenderer2D * r2d;
	};

	class DebugWidget
	{
	public:
		static constexpr int ID = 0;

		bool active = true;

		virtual void OnUpdate( timestep dt, float * offsetY, const WindowContext* ctx ) = 0;
	};

	template <typename T>
	class DebugSlider : public neutron::Slider, public DebugWidget
	{
	public:
		static constexpr int ID = 0x45;

		DebugSlider( const char * label, const char * format ): label(label) { fmt = format; }

		virtual bool UpdateValue( T * pValue )
		{
			internal_value = *pValue;
			return false;
		}

		virtual void OnUpdate( timestep dt, float * offsetY, const WindowContext * ctx ) override
		{
			static float pos = 0.0f;
			static bool goright = true;

			if(goright){pos+=10.0f*dt;if(pos>80.0f)pos=80.0f,goright=false;}
			else       {pos-=10.0f*dt;if(pos< 0.0f)pos= 0.0f,goright= true;}

			char text[100];
			sprintf( text, fmt, internal_value );

			auto tl = ctx->r2d->DrawString( label.c_str(), {g_DebugStyle.LeftPadding+g_DebugStyle.LeftTextBoxPadding, *offsetY + g_DebugStyle.VerticalPadding}, Colors::make_gray(0.8f) );

			base::rectf rect = { 
				g_DebugStyle.LeftPadding, ctx->rect.right() - g_DebugStyle.RightPadding, 
				g_DebugStyle.VerticalPadding, g_DebugStyle.VerticalPadding + tl.height
			};

			rect.y += *offsetY;


		//	ctx->r2d->DrawRect( rect, Colors::Red, 0.5f );

			rect.x.low += g_DebugStyle.MaximumLabelWidth;
			ctx->r2d->FillRect( rect, Colors::make_gray<rgb_color8>( 33 ) );

			ctx->r2d->DrawString( text, rect.topLeft() + base::vector2f(g_DebugStyle.LeftTextBoxPadding,0.0f), Colors::make_gray(0.8f) );

			rect.x = { rect.left() + pos, rect.left() + pos + 10.0f };
			ctx->r2d->FillRect( rect, color(1.0f,1.0f,1.0f,0.03f) );

			*offsetY += g_DebugStyle.VerticalPadding + tl.height;
		}

	private:

		string label;

		const char * fmt;
		T internal_value = static_cast<T>(0);

	};

}