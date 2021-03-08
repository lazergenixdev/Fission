#include "Fission/config.h"
#include "Fission/Core/Application.h"

#if !defined(IMGUI_DISABLE)
#include "imgui.cpp"
#include "imgui_demo.cpp"
#include "imgui_draw.cpp"
#include "imgui_tables.cpp"
#include "imgui_widgets.cpp"

#ifdef FISSION_PLATFORM_WINDOWS
#include "backends/imgui_impl_dx11.cpp"
#include "backends/imgui_impl_win32.cpp"

//	ImGuiPlatformIO & platform_io = ImGui::GetPlatformIO();
//	platform_io.Platform_CreateWindow = ImGui_ImplWin32_CreateWindow;
//	platform_io.Platform_DestroyWindow = ImGui_ImplWin32_DestroyWindow;
//	platform_io.Platform_ShowWindow = ImGui_ImplWin32_ShowWindow;
//	platform_io.Platform_SetWindowPos = ImGui_ImplWin32_SetWindowPos;
//	platform_io.Platform_GetWindowPos = ImGui_ImplWin32_GetWindowPos;
//	platform_io.Platform_SetWindowSize = ImGui_ImplWin32_SetWindowSize;
//	platform_io.Platform_GetWindowSize = ImGui_ImplWin32_GetWindowSize;
//	platform_io.Platform_SetWindowFocus = ImGui_ImplWin32_SetWindowFocus;
//	platform_io.Platform_GetWindowFocus = ImGui_ImplWin32_GetWindowFocus;
//	platform_io.Platform_GetWindowMinimized = ImGui_ImplWin32_GetWindowMinimized;
//	platform_io.Platform_SetWindowTitle = ImGui_ImplWin32_SetWindowTitle;
//	platform_io.Platform_SetWindowAlpha = ImGui_ImplWin32_SetWindowAlpha;
//	platform_io.Platform_UpdateWindow = ImGui_ImplWin32_UpdateWindow;
//	platform_io.Platform_GetWindowDpiScale = ImGui_ImplWin32_GetWindowDpiScale; // FIXME-DPI
//	platform_io.Platform_OnChangedViewport = ImGui_ImplWin32_OnChangedViewport; // FIXME-DPI

void platform_create_window( struct ImGuiViewport * vp )
{
	auto app = Fission::Application::Get();
	auto create_window = [&] {
		ImGui_ImplWin32_CreateWindow( vp );
	};
	app->GetWindow()->Call( create_window );
}

void platform_destroy_window( struct ImGuiViewport * vp )
{
	auto app = Fission::Application::Get();
	auto destroy_window = [&] {
		ImGui_ImplWin32_DestroyWindow( vp );
	};
	app->GetWindow()->Call( destroy_window );
}

void platform_update_window( struct ImGuiViewport * vp )
{
	auto app = Fission::Application::Get();
	auto fn = [&] {
		ImGui_ImplWin32_UpdateWindow( vp );
	};
	app->GetWindow()->Call( fn );
}

bool platform_get_window_focus( struct ImGuiViewport * vp )
{
	auto app = Fission::Application::Get();
	bool ret = false;
	auto fn = [&] {
		ret = ImGui_ImplWin32_GetWindowFocus( vp );
	};
	app->GetWindow()->Call( fn );
	return ret;
}

void platform_set_window_focus( struct ImGuiViewport * vp )
{
	auto app = Fission::Application::Get();
	auto fn = [&] {
		ImGui_ImplWin32_SetWindowFocus( vp );
	};
	app->GetWindow()->Call( fn );
}

// override platform functions to ensure consistancy with Fission::Window implementation
void SetImGuiPlatformIO()
{
	auto & PlatformIO = ImGui::GetPlatformIO();
	PlatformIO.Platform_CreateWindow = platform_create_window;
	PlatformIO.Platform_DestroyWindow = platform_destroy_window;
	PlatformIO.Platform_UpdateWindow = platform_update_window;
	PlatformIO.Platform_SetWindowFocus = platform_set_window_focus;
	PlatformIO.Platform_GetWindowFocus = platform_get_window_focus;
}

#endif

#endif // !IMGUI_DISABLE