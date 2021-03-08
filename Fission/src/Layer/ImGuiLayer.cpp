#include "ImGuiLayer.h"
#include "Fission/Core/Application.h"

#if !defined(IMGUI_DISABLE)

#include "imgui.h"
#include "imgui_internal.h"

#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"

#include "Fission/Core/UI/UI.h"
#include "Fission/Core/Console.h"

namespace Fission {
	ImGuiContext * GetImGuiContext()
	{
		return GImGui;
	}
}

using namespace Fission;

std::mutex mutex;
std::vector<wchar_t> character_buffer;

extern void SetImGuiPlatformIO();

//namespace JetBrainsMonoTTF {
//#include "Static Fonts/JetBrainsMono-Regular.inl"
//}

static void SetImGuiColors()
{
	auto & style = ImGui::GetStyle();

	ImVec4 * colors = style.Colors;
	colors[ImGuiCol_WindowBg] = ImVec4( 0.00f, 0.00f, 0.00f, 0.94f );
	colors[ImGuiCol_Border] = ImVec4( 1.00f, 1.00f, 1.00f, 0.14f );
	colors[ImGuiCol_FrameBg] = ImVec4( 0.34f, 0.34f, 0.34f, 0.14f );

	style.FrameRounding = 2.0f;
	style.WindowRounding = 2.0f;
}

void ImGuiLayer::OnCreate()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO & io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
	io.IniFilename = "etc/imgui.ini"; // "etc/" is guaranteed to exist
	SetImGuiColors();

	//io.Fonts->AddFontFromMemoryTTF( (void *)JetBrainsMonoTTF::data, JetBrainsMonoTTF::size, 13.0f );
	io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\Consola.ttf", 13.0f);

	auto win = GetApp()->GetWindow();
	auto gfx = GetApp()->GetGraphics();

	auto dx11 = reinterpret_cast<Graphics::native_type_dx11 *>( gfx->native_handle() );

	ImGui_ImplWin32_Init( win->native_handle() );
	ImGui_ImplDX11_Init( (ID3D11Device*)dx11->pDevice, (ID3D11DeviceContext *)dx11->pDeviceContext );

	SetImGuiPlatformIO();

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::DockSpaceOverViewport( nullptr, ImGuiDockNodeFlags_NoDockingInCentralNode | ImGuiDockNodeFlags_PassthruCentralNode );

	//ImGui::ShowDemoWindow();
}

static EventResult HandleEvent( Platform::Event * pEvent )
{
	extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
	return !!ImGui_ImplWin32_WndProcHandler( pEvent->hWnd, pEvent->Msg, pEvent->wParam, pEvent->lParam ) ? EventResult::Handled : EventResult::Pass;
}

void ImGuiLayer::OnUpdate()
{
	auto io = ImGui::GetIO();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData() );

	// Update and Render additional Platform Windows
	if( io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable )
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::DockSpaceOverViewport( nullptr, ImGuiDockNodeFlags_NoDockingInCentralNode | ImGuiDockNodeFlags_PassthruCentralNode );

	//ImGui::ShowDemoWindow();
}

EventResult ImGuiLayer::OnKeyDown( KeyDownEventArgs & args )
{
	HandleEvent( args.native_event );
	if( ImGui::GetIO().WantCaptureKeyboard && Keys::is_keyboard( args.key ) ) return EventResult::Handled;
	if( ImGui::GetIO().WantCaptureMouse && Keys::is_mouse( args.key ) ) return EventResult::Handled;
	return EventResult::Pass;
}

EventResult ImGuiLayer::OnKeyUp( KeyUpEventArgs & args )
{
	return HandleEvent( args.native_event );
}

EventResult ImGuiLayer::OnTextInput( TextInputEventArgs & args )
{
	//if( ImGui::GetCurrentContext() == NULL )
	//	return EventResult::Pass;
	//auto io = ImGui::GetIO();
	//unsigned short ch = args.character;
	//if( ch > 0 && ch < 0x10000 )
	//	io.AddInputCharacter( ch );
	//Console::Message( L"Got character: %c", args.character );
	//return EventResult::Pass;
	return HandleEvent( args.native_event );
}

EventResult ImGuiLayer::OnMouseMove( MouseMoveEventArgs & args )
{
	return HandleEvent( args.native_event );
}

EventResult ImGuiLayer::OnMouseLeave( MouseLeaveEventArgs & args )
{
	return HandleEvent( args.native_event );
}

EventResult ImGuiLayer::OnSetCursor( SetCursorEventArgs & args )
{
	args.bUseCursor = false; // we set the cursor ourselves
	return HandleEvent( args.native_event );
}

#endif // !IMGUI_DISABLE
