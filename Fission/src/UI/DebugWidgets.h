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

#define _neutron_point_type   Fission::v2i32
#define _neutron_rect_type    Fission::base::recti
#define _neutron_vector_type  std::vector

#include <Fission/neutron.hpp>

namespace Fission
{
	struct StyleInformation
	{
		float LeftPadding = 6.0f;
		float RightPadding = 6.0f;

		float VerticalPadding = 4.0f;

		float MaximumLabelWidth = 115.0f;

		float LeftTextBoxPadding = 2.0f;

		float SliderKnobWidth = 10.0f;

		float FramePadding = 2.0f;

	};

	struct KeyboardState
	{
		bool ctrl_down = false;
	};


	static StyleInformation g_DebugStyle;
	static KeyboardState g_KeyboardState;


	struct WindowContext
	{
		base::rectf rect;
		IFRenderer2D * r2d;
	};

	class DebugWidget : public neutron::Widget
	{
	public:
		static constexpr int ID = 0;

		bool active = true;

		virtual void OnUpdate( timestep dt, float * offsetY, const WindowContext* ctx ) = 0;
	};

	template <typename T> struct _type_id {};
	template <> struct _type_id<int>   { static constexpr int value = 0; };
	template <> struct _type_id<float> { static constexpr int value = 1; };
	template <typename T> static constexpr int type_id_of = _type_id<T>::value;


	template <typename T>
	class DebugSlider : public DebugWidget
	{
	public:
		static constexpr int ID = 0x45 + type_id_of<T>;

		DebugSlider( const char * label, const char * format, T initial_value ): label(label), fmt(format), internal_value(initial_value) {}

		virtual bool UpdateValue( T * pValue, T min, T max )
		{
			m_Max = max;
			m_Min = min;

			if( internal_value == *pValue ) return false;

			if( editing )
			{
				*pValue = internal_value;
				if( typing ) {
					typing = false;
					editing = false;
				}
				return true;
			}

			internal_value = *pValue;
			return false;
		}

		virtual bool isInside( neutron::point pt ) override
		{
			return Rect[pt];
		}

		virtual void OnUpdate( timestep dt, float * offsetY, const WindowContext * ctx ) override
		{

			auto tl = ctx->r2d->DrawString( label.c_str(), {g_DebugStyle.LeftPadding+g_DebugStyle.LeftTextBoxPadding, *offsetY + g_DebugStyle.VerticalPadding}, Colors::make_gray(0.8f) );

			base::rectf rect = { 
				g_DebugStyle.LeftPadding, ctx->rect.right() - g_DebugStyle.RightPadding, 
				g_DebugStyle.VerticalPadding, g_DebugStyle.VerticalPadding + tl.height
			};
			rect.y += *offsetY;

			rect.x.low += g_DebugStyle.MaximumLabelWidth;
			Rect = base::recti( rect );

			ctx->r2d->FillRect( rect, Colors::make_gray<rgb_color8>( 33 ) );


			if( parent->GetHover() == this )
			highlight += ( 1.0f - highlight ) * 5.0f * dt;
			else
			highlight += ( 0.0f - highlight ) * 2.5f * dt;

			if( highlight != 0.0f )
			ctx->r2d->DrawRect( rect.expanded(-0.5f), Colors::make_gray(highlight*0.5f,highlight), 1.0f );

			if( typing )
				tl = ctx->r2d->DrawString( temp_value_str.c_str(), rect.topLeft() + v2f32(g_DebugStyle.LeftTextBoxPadding,0.0f), Colors::make_gray(0.8f) );
			else
			{
				char text[100];
				sprintf( text, fmt, internal_value );
				tl = ctx->r2d->DrawString( text, rect.topLeft() + v2f32(g_DebugStyle.LeftTextBoxPadding,0.0f), Colors::make_gray(0.8f) );
			}

			if( typing )
			{
				ctx->r2d->DrawRect( rect, color(0.8f,0.9f,1.0f, 0.2f), 2.0f );
				ctx->r2d->FillRect( { rect.left() + tl.width + 2.0f, rect.left() + tl.width + 3.0f, rect.top()+2.0f, rect.bottom()-2.0f }, Colors::White );
			}


			float slider_pos = (rect.width()-g_DebugStyle.SliderKnobWidth) * (std::clamp( internal_value, m_Min, m_Max ) - m_Min) / (m_Max - m_Min);

			rect.x = { rect.left() + slider_pos, rect.left() + slider_pos + g_DebugStyle.SliderKnobWidth };
			ctx->r2d->FillRect( rect, color(1.0f,1.0f,1.0f,editing?0.1f:0.03f) );

			*offsetY += g_DebugStyle.VerticalPadding + tl.height;
		}

		virtual neutron::Result OnKeyDown( neutron::KeyDownEventArgs & args ) override {
			if( args.key == Keys::Mouse_Left )
			{
				if( g_KeyboardState.ctrl_down )
				{
					typing = true;
					temp_value_str = std::to_string(internal_value);
				}
				else
				{
					editing = true;
					parent->SetCapture( this );
					neutron::MouseMoveEventArgs mm_args{ neutron::GetMousePosition() };
					this->OnMouseMove( mm_args );
				}
				return neutron::Handled;
			}
			return neutron::Pass;
		}
		virtual neutron::Result OnKeyUp( neutron::KeyUpEventArgs & args ) override {
			if( args.key == Keys::Mouse_Left )
			{
				editing = false;
				parent->SetCapture( nullptr );
				return neutron::Handled;
			}
			return neutron::Pass;
		}
		virtual neutron::Result OnMouseMove( neutron::MouseMoveEventArgs & args ) override {
			if( editing )
			{
				auto ry = rangef64(Rect.x).expanded(double(g_DebugStyle.SliderKnobWidth)*-0.5);
				double slider_pos = ry.clamp( (double)args.pos.x );

				internal_value = static_cast<T>(
					( (double)m_Max - (double)m_Min ) * ( (slider_pos-ry.low) / ry.distance() ) + (double)m_Min
				);

				return neutron::Handled;
			}
			return neutron::Pass;
		}
		virtual neutron::Result OnTextInput( neutron::TextInputEventArgs & args ) override {
			if( typing )
			{
				switch( args.ch )
				{
				case '\b': if( temp_value_str.size() ) temp_value_str.pop_back(); break;
				case '\r': { StopTyping(); break; }

				default: temp_value_str = temp_value_str.str() += (char)args.ch; break;
				}
				return neutron::Handled;
			}
			return neutron::Pass;
		}
		virtual void OnFocusLost() override {
			typing = false;
		}

		void StopTyping()
		{
			// try to set our value from the temperary string.
			try
			{
				if constexpr ( std::is_floating_point_v<T> )
					internal_value = std::stof(temp_value_str.c_str());
				if constexpr ( std::is_integral_v<T> )
					internal_value = std::stoi(temp_value_str.c_str());
			} catch( ... ) {}

			parent->SetFocus( nullptr );

			editing = true;
		}

	private:

		string label;
		string temp_value_str;

		const char * fmt;
		T internal_value = static_cast<T>(0);

		T m_Min = static_cast<T>( 0 ), m_Max = static_cast<T>( 0 );

		bool editing = false;
		bool typing = false;


		neutron::rect Rect;

		float highlight = 0.0f;

	};

	template <typename T>
	class DebugInput : public DebugWidget
	{
	public:
		static constexpr int ID = 0x67 + type_id_of<T>;

		DebugInput( const char * label, const char * format, T initial_value ) : label( label ), fmt( format ), internal_value( initial_value ) {}

		virtual bool UpdateValue( T * pValue )
		{
			if( internal_value == *pValue ) return false;

			if( editing )
			{
				*pValue = internal_value;
				if( typing ) {
					typing = false;
					editing = false;
				}
				return true;
			}

			internal_value = *pValue;
			return false;
		}

		virtual bool isInside( neutron::point pt ) override { return Rect[pt]; }

		virtual void OnUpdate( timestep dt, float * offsetY, const WindowContext * ctx ) override
		{

			auto tl = ctx->r2d->DrawString( label.c_str(), { g_DebugStyle.LeftPadding + g_DebugStyle.LeftTextBoxPadding, *offsetY + g_DebugStyle.VerticalPadding }, Colors::make_gray( 0.8f ) );

			base::rectf rect = {
				g_DebugStyle.LeftPadding, ctx->rect.right() - g_DebugStyle.RightPadding,
				g_DebugStyle.VerticalPadding, g_DebugStyle.VerticalPadding + tl.height
			};
			rect.y += *offsetY;

			rect.x.low += g_DebugStyle.MaximumLabelWidth;
			Rect = base::recti( rect );

			ctx->r2d->FillRect( rect, Colors::make_gray<rgb_color8>( 33 ) );


			if( parent->GetHover() == this )
				highlight += ( 1.0f - highlight ) * 5.0f * dt;
			else
				highlight += ( 0.0f - highlight ) * 2.5f * dt;

			if( highlight != 0.0f )
				ctx->r2d->DrawRect( rect.expanded( -0.5f ), Colors::make_gray( highlight * 0.5f, highlight ), 1.0f );

			if( typing )
				tl = ctx->r2d->DrawString( temp_value_str.c_str(), rect.topLeft() + v2f32( g_DebugStyle.LeftTextBoxPadding, 0.0f ), Colors::make_gray( 0.8f ) );
			else
			{
				char text[100];
				sprintf( text, fmt, internal_value );
				tl = ctx->r2d->DrawString( text, rect.topLeft() + v2f32( g_DebugStyle.LeftTextBoxPadding, 0.0f ), Colors::make_gray( 0.8f ) );
			}

			if( typing )
			{
				ctx->r2d->DrawRect( rect, color( 0.8f, 0.9f, 1.0f, 0.2f ), 2.0f );
				ctx->r2d->FillRect( { rect.left() + tl.width + 2.0f, rect.left() + tl.width + 3.0f, rect.top() + 2.0f, rect.bottom() - 2.0f }, Colors::White );
			}

			*offsetY += g_DebugStyle.VerticalPadding + tl.height;
		}

		virtual neutron::Result OnKeyDown( neutron::KeyDownEventArgs & args ) override {
			if( args.key == Keys::Mouse_Left )
			{
				typing = true;
				temp_value_str = std::to_string( internal_value );
				return neutron::Handled;
			}
			return neutron::Pass;
		}
		virtual neutron::Result OnKeyUp( neutron::KeyUpEventArgs & args ) override {
			if( args.key == Keys::Mouse_Left )
			{
			//	editing = false;
			//	parent->SetCapture( nullptr );
				return neutron::Handled;
			}
			return neutron::Pass;
		}
		virtual neutron::Result OnTextInput( neutron::TextInputEventArgs & args ) override {
			if( typing )
			{
				switch( args.ch )
				{
				case '\b': if( temp_value_str.size() ) temp_value_str.pop_back(); break;
				case '\r': { StopTyping(); break; }

				default: temp_value_str = temp_value_str.str() += (char)args.ch; break;
				}
				return neutron::Handled;
			}
			return neutron::Pass;
		}
		virtual void OnFocusLost() override {
			typing = false;
		}

		void StopTyping()
		{
			// try to set our value from the temperary string.
			try
			{
				if constexpr( std::is_floating_point_v<T> )
					internal_value = std::stof( temp_value_str.c_str() );
				if constexpr( std::is_integral_v<T> )
					internal_value = std::stoi( temp_value_str.c_str() );
			}
			catch( ... ) {}

			parent->SetFocus( nullptr );

			editing = true;
		}

	private:

		string label;
		string temp_value_str;

		const char * fmt;
		T internal_value = static_cast<T>( 0 );

		bool editing = false;
		bool typing = false;

		neutron::rect Rect;
		float highlight = 0.0f;
	};

	class DebugText : public DebugWidget
	{
	public:
		static constexpr int ID = 0x100;

		DebugText( const char * text ) : label( text ) {}

		virtual void OnUpdate( timestep dt, float * offsetY, const WindowContext * ctx ) override
		{
			auto tl = ctx->r2d->DrawString( label.c_str(), { g_DebugStyle.LeftPadding + g_DebugStyle.LeftTextBoxPadding, *offsetY + g_DebugStyle.VerticalPadding }, Colors::make_gray( 0.8f ) );
			*offsetY += g_DebugStyle.VerticalPadding + tl.height;
		}

	private:
		string label;
	};

	class DebugButton : public DebugWidget
	{
	public:
		static constexpr int ID = 0x123;

		DebugButton( const char * label ) : label( label ) {}

		virtual bool IsPressed()
		{
			bool out = pressed;
			pressed = false;
			return out;
		}

		virtual bool isInside( neutron::point pt ) override
		{
			return Rect[pt];
		}

		virtual void OnUpdate( timestep dt, float * offsetY, const WindowContext * ctx ) override
		{
			if( parent->GetHover() == this )
				highlight += ( 1.0f - highlight ) * 5.0f * dt;
			else
				highlight += ( 0.0f - highlight ) * 2.5f * dt;

			auto pos = v2f32(g_DebugStyle.LeftPadding + g_DebugStyle.LeftTextBoxPadding, *offsetY + g_DebugStyle.VerticalPadding);
			auto tl = ctx->r2d->CreateTextLayout( label.c_str() );

			auto rect = base::rectf::from_topleft( pos, tl.width + g_DebugStyle.FramePadding * 2.0f, tl.height + g_DebugStyle.FramePadding * 2.0f );
			Rect = base::recti( rect );

			if( down )
			ctx->r2d->FillRect( rect, Colors::make_gray<rgb_color8>( 88 ) );
			else
			ctx->r2d->FillRect( rect, Colors::make_gray<rgb_color8>( 66 ) );

			ctx->r2d->DrawRect( rect.expanded( -0.5f ), Colors::make_gray( highlight * 0.5f, highlight ), 1.0f );

			ctx->r2d->DrawString( label.c_str(), pos+v2f32::from1(g_DebugStyle.FramePadding), Colors::make_gray( 0.8f ) );

			*offsetY += g_DebugStyle.VerticalPadding * 2.0f + tl.height;
		}


		virtual neutron::Result OnKeyDown( neutron::KeyDownEventArgs & args ) override {
			if( args.key == Keys::Mouse_Left )
			{
				down = true;
				parent->SetCapture( this );
				return neutron::Result::Pass;
			}
			return neutron::Result::Pass;
		}
		virtual neutron::Result OnKeyUp( neutron::KeyUpEventArgs & args ) override {
			if( args.key == Keys::Mouse_Left )
			{
				down = false;
				if( Rect[neutron::GetMousePosition()] )
					pressed = true;
				parent->SetCapture( nullptr );
				return neutron::Result::Pass;
			}
			return neutron::Result::Handled;
		}

	private:
		string label;

		bool pressed = false;
		bool down = false;

		neutron::rect Rect;

		float highlight = 0.0f;
	};


	class DebugCheckbox : public DebugWidget
	{
	public:
		static constexpr int ID = 0x225;

		DebugCheckbox( const char * label, bool initial_value ) : label( label ), internal_value( initial_value ) {}

		virtual bool UpdateValue( bool * pValue )
		{
			if( internal_value == *pValue ) return false;

			if( editing )
			{
				*pValue = internal_value;
				editing = false;
				return true;
			}

			internal_value = *pValue;
			return false;
		}

		virtual bool isInside( neutron::point pt ) override
		{
			return Rect[pt];
		}

		virtual void OnUpdate( timestep dt, float * offsetY, const WindowContext * ctx ) override
		{

			auto tl = ctx->r2d->DrawString( label.c_str(), { g_DebugStyle.LeftPadding + g_DebugStyle.LeftTextBoxPadding, *offsetY + g_DebugStyle.VerticalPadding }, Colors::make_gray( 0.8f ) );

			base::rectf rect = {
				g_DebugStyle.LeftPadding, g_DebugStyle.LeftPadding + tl.height,
				g_DebugStyle.VerticalPadding, g_DebugStyle.VerticalPadding + tl.height
			};
			rect.y += *offsetY;

			rect.x.low += g_DebugStyle.MaximumLabelWidth;
			rect.x.high += g_DebugStyle.MaximumLabelWidth;
			Rect = base::recti( rect );

			ctx->r2d->FillRect( rect, down ? Colors::make_gray<rgb_color8>( 57 ) : Colors::make_gray<rgb_color8>( 33 ) );


			if( parent->GetHover() == this )
				highlight += ( 1.0f - highlight ) * 5.0f * dt;
			else
				highlight += ( 0.0f - highlight ) * 2.5f * dt;

			if( highlight != 0.0f )
				ctx->r2d->DrawRect( rect.expanded( -0.5f ), Colors::make_gray( highlight * 0.5f, highlight ), 1.0f );

			//if( typing )
			//	tl = ctx->r2d->DrawString( temp_value_str.c_str(), rect.topLeft() + v2f32( g_DebugStyle.LeftTextBoxPadding, 0.0f ), Colors::make_gray( 0.8f ) );
			//else
			//{
			//	char text[100];
			//	sprintf( text, fmt, internal_value );
			//	tl = ctx->r2d->DrawString( text, rect.topLeft() + v2f32( g_DebugStyle.LeftTextBoxPadding, 0.0f ), Colors::make_gray( 0.8f ) );
			//}

			//if( typing )
			//{
			//	ctx->r2d->DrawRect( rect, color( 0.8f, 0.9f, 1.0f, 0.2f ), 2.0f );
			//	ctx->r2d->FillRect( { rect.left() + tl.width + 2.0f, rect.left() + tl.width + 3.0f, rect.top() + 2.0f, rect.bottom() - 2.0f }, Colors::White );
			//}

			/////////////////////////////////////////////////////////
			static Fission::Mesh mesh = []
			{
				Fission::Mesh mesh( 6, 12 );

				mesh.push_vertex( { 0.0f,1.5f }, 0 );
				mesh.push_vertex( { 0.5f,1.0f }, 0 );
				mesh.push_vertex( { 1.0f,1.5f }, 0 );
				mesh.push_vertex( { 1.0f,2.5f }, 0 );
				mesh.push_vertex( { 2.5f,0.0f }, 0 );
				mesh.push_vertex( { 3.0f,0.5f }, 0 );

				mesh.push_index( 0 ); mesh.push_index( 1 ); mesh.push_index( 2 );
				mesh.push_index( 0 ); mesh.push_index( 2 ); mesh.push_index( 3 );
				mesh.push_index( 2 ); mesh.push_index( 3 ); mesh.push_index( 5 );
				mesh.push_index( 2 ); mesh.push_index( 5 ); mesh.push_index( 4 );

				mesh.push_color( Colors::White );

				return mesh;
			}( );

			if( internal_value )
			{
				ctx->r2d->PushTransform( m23::Scaling( 5.0f, 5.0f ) * m23::Translation( (rect.left()+1.0f)/5.0f, (rect.top()+2.0f)/5.0f ) );
				ctx->r2d->DrawMesh( &mesh );
				ctx->r2d->PopTransform();
			}

			/////////////////////////////////////////////////////////

			*offsetY += g_DebugStyle.VerticalPadding + tl.height;
		}

		virtual neutron::Result OnKeyDown( neutron::KeyDownEventArgs & args ) override {
			if( args.key == Keys::Mouse_Left )
			{
				down = true;
				parent->SetCapture( this );
				return neutron::Handled;
			}
			return neutron::Pass;
		}
		virtual neutron::Result OnKeyUp( neutron::KeyUpEventArgs & args ) override {
			if( args.key == Keys::Mouse_Left )
			{
				down = false;
				if( Rect[neutron::GetMousePosition()] )
				{
					internal_value = !internal_value;
					editing = true;
				}
				parent->SetCapture( nullptr );
				return neutron::Handled;
			}
			return neutron::Pass;
		}

	private:

		string label;

		bool internal_value = false;
		bool down = false;
		bool editing = false;

		neutron::rect Rect;

		float highlight = 0.0f;

	};
}