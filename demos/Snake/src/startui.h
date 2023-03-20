#pragma once
#include <Fission/Fission.h>
#include <Fission/Base/Rect.hpp>

#define _neutron_key_primary_mouse   Fission::Keys::Mouse_Left
#define _neutron_key_secondary_mouse Fission::Keys::Mouse_Right

#define _neutron_char_type    Fission::chr
#define _neutron_key_type     Fission::Keys::Key
#define _neutron_cursor_type  Fission::Cursor*

#define _neutron_point_type   Fission::v2i32
#define _neutron_rect_type    Fission::ri32
#define _neutron_vector_type  Fission::dynamic_buffer
#include "Fission/neutron.hpp"

namespace ui
{
	using Fission::string;
	using Fission::ri32;

	static Fission::Renderer2D* g_r2d = nullptr;

	class Button : public ::neutron::Button
	{
	public:
		Button( string label, Fission::v2i32 centerpos, Fission::size2 size )
			: rect(ri32::from_center(centerpos,size)), label(label)
		{}

		virtual bool isInside(neutron::point pos) override { return rect.closed_upper(pos); }

		void OnUpdate(float) override
		{
			using namespace Fission::colors;
			using Fission::StrokeStyle;

			auto frect = Fission::rf32(rect);
			auto tl = g_r2d->CreateTextLayout(label.c_str());

			Fission::color c = (parent->GetHover() == this) ? Cyan : DimGray;

			g_r2d->DrawRoundRect(frect, 10.0f, c, 0.75f, StrokeStyle::Inside);

			auto d = Fission::v2f32(frect.width() - tl.width, frect.height() - tl.height)*0.5f;
			g_r2d->DrawString(label.c_str(), frect.topLeft()+d, c);
		}

		neutron::Result OnSetCursor(neutron::SetCursorEventArgs& args) override
		{
			if (parent->GetHover() == this)
			{
				args.cursor = Fission::Cursor::Get(Fission::Cursor::Default_Hand);
				return neutron::Handled;
			}
			return neutron::Pass;
		}

	private:
		ri32 rect;
		string label;
	};

}
