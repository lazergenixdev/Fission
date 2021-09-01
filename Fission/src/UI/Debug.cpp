#include "Debug.h"
#if FISSION_ENABLE_DEBUG_UI
#include "DebugWindow.h"

namespace Fission
{
	static struct DebugWindow * s_DebugWindow;

	namespace UI
	{
		bool Debug::Button( const char * label )
		{
			auto widget = s_DebugWindow->GetWidget( label, DebugButton::ID, nullptr, nullptr );

			if( auto button = dynamic_cast<DebugButton*>( widget ) )
				return button->IsPressed();

			return false;
		}

		bool Debug::CheckBox( const char * label, bool * value )
		{
			auto widget = s_DebugWindow->GetWidget( label, DebugCheckbox::ID, nullptr, value );

			if( auto checkbox = dynamic_cast<DebugCheckbox *>( widget ) )
				return checkbox->UpdateValue( value );

			return false;
		}

		bool Debug::SliderFloat( const char * label, float * value, float min, float max, const char * format )
		{
			auto widget = s_DebugWindow->GetWidget( label, DebugSlider<float>::ID, format, value );

			if( auto slider = dynamic_cast<DebugSlider<float> *>( widget ) )
				return slider->UpdateValue( value, min, max );

			return false;
		}

		bool Debug::SliderInt( const char * label, int * value, int min, int max, const char * format )
		{
			auto widget = s_DebugWindow->GetWidget( label, DebugSlider<int>::ID, format, value );

			if( auto slider = dynamic_cast<DebugSlider<int> *>( widget ) )
				return slider->UpdateValue( value, min, max );

			return false;
		}

		bool Debug::InputFloat( const char * label, float * value, const char * format )
		{
			auto widget = s_DebugWindow->GetWidget( label, DebugInput<float>::ID, format, value );

			if( auto input = dynamic_cast<DebugInput<float> *>( widget ) )
				return input->UpdateValue( value );

			return false;
		}

		bool Debug::InputInt( const char * label, int * value, const char * format )
		{
			auto widget = s_DebugWindow->GetWidget( label, DebugInput<int>::ID, format, value );

			if( auto input = dynamic_cast<DebugInput<int> *>( widget ) )
				return input->UpdateValue( value );

			return false;
		}

		void Debug::Text( const char * label )
		{
			s_DebugWindow->GetWidget( label, DebugText::ID, nullptr, nullptr );
		}

	}


	void CreateDebug( WindowManager * pWindowManager, FApplication * app )
	{
		s_DebugWindow = new DebugWindow;

		IFWindow::CreateInfo info = {};
		info.pEventHandler = s_DebugWindow;
		info.wProperties.style = IFWindow::Style::Borderless;
		info.wProperties.size = s_DebugWindow->Rect.size();
		pWindowManager->CreateWindow( &info, &s_DebugWindow->window );

		s_DebugWindow->OnCreate( app );
	}

	void RenderDebug( timestep dt )
	{
		auto SwapChain = s_DebugWindow->window->GetSwapChain();

		SwapChain->Bind();
		s_DebugWindow->OnUpdate( dt );
		SwapChain->Present( vsync_Off );
	}

	void DestroyDebug()
	{
		s_DebugWindow->Destroy();
	}
}

#endif // FISSION_ENABLE_DEBUG_UI
