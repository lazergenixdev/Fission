#include "StartScene.h"
#include "SnakeApp.h"

void StartMenuLayer::OnCreate( Fission::FApplication * app )
{
	Fission::Simple2DLayer::OnCreate(app);

	m_App = app;
	ui::g_r2d = m_pRenderer2D;

	auto ws = app->pMainWindow->GetSize();
	wm.Initialize(ws.width(), ws.height());

	wm.addWindow( new ui::Button("New Game", { 300, 200 }, { 200, 40 }) );
}

Fission::EventResult StartMenuLayer::OnMouseMove(Fission::MouseMoveEventArgs& args)
{
	neutron::MouseMoveEventArgs nargs;
	nargs.pos = args.position;
	return (Fission::EventResult)wm.OnMouseMove(nargs);
}

Fission::EventResult StartMenuLayer::OnSetCursor(Fission::SetCursorEventArgs& args)
{
	neutron::SetCursorEventArgs nargs;
	nargs.cursor = args.cursor;
	auto r = (Fission::EventResult)wm.OnSetCursor(nargs);
	if (nargs.cursor != args.cursor)
	{
		args.cursor = nargs.cursor;
		args.bUseCursor = true;
	}
	return r;
}

void StartMenuLayer::OnUpdate( Fission::timestep dt )
{
	m_pRenderer2D->SelectFont( Fission::FontManager::GetFont( "$debug" ) );

	wm.OnUpdate(0.0f);

	m_pRenderer2D->Render();
}

StartScene::StartScene() { PushLayer( new StartMenuLayer ); }
