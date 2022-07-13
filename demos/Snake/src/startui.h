#pragma once
#include <Fission/Fission.h>

#define _neutron_key_primary_mouse   Fission::Keys::Mouse_Left
#define _neutron_key_secondary_mouse Fission::Keys::Mouse_Right

#define _neutron_char_type    Fission::chr
#define _neutron_key_type     Fission::Keys::Key
#define _neutron_cursor_type  Fission::Cursor*

#define _neutron_point_type   Fission::base::vector2i
#define _neutron_rect_type    Fission::base::recti
#define _neutron_vector_type  std::vector
#include "Fission/neutron.hpp"

namespace ui
{
	using namespace Fission::base;
	using namespace Fission;

	static Fission::IFRenderer2D* g_r2d = nullptr;

	class Button : public neutron::Button
	{
	public:
		Button( string label, vector2i centerpos, size2i size )
			: rect(recti::from_center(centerpos,size)), label(label)
		{}

		virtual bool isInside(neutron::point pos) override { return rect.upper(pos); }

		void OnUpdate(float) override
		{
			auto frect = rectf(rect);
			auto tl = g_r2d->CreateTextLayout(label.c_str());

			color c = (parent->GetHover() == this) ? Colors::Cyan : Colors::DimGray;

			g_r2d->DrawRoundRect(frect, 10.0f, c, 0.75f, StrokeStyle::Inside);

			auto d = vector2f(frect.width() - tl.width, frect.height() - tl.height)*0.5f;
			g_r2d->DrawString(label.c_str(), frect.topLeft()+d, c);
		}

		neutron::Result OnSetCursor(neutron::SetCursorEventArgs& args) override
		{
			if (parent->GetHover() == this)
			{
				args.cursor = Cursor::Get(Cursor::Default_Hand);
				return neutron::Handled;
			}
			return neutron::Pass;
		}

	private:
		recti rect;
		string label;
	};

}
