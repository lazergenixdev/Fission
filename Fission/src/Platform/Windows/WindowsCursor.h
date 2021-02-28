#pragma once
#include "Fission/Core/Input/Cursor.h"

namespace Fission::Platform
{
	class WindowsCursor : public Cursor
	{
	public:
		WindowsCursor( HCURSOR hCursor, bool shared = true );

		static WindowsCursor * Default( Cursor::Default_ def );

		virtual bool Use() override;

		virtual ~WindowsCursor() override;

	private:
		HCURSOR handle = NULL;
		bool bShared;
	};

}