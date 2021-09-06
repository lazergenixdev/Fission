#include "StartScene.h"
#include "SnakeApp.h"

void StartMenuLayer::OnCreate( Fission::FApplication * app )
{
	m_App = app;
	r2d = (decltype( r2d ))app->pEngine->GetRenderer( "$internal2D" );
}

void StartMenuLayer::OnUpdate( Fission::timestep dt )
{
	r2d->SelectFont( Fission::FontManager::GetFont( "$debug" ) );

	Fission::base::size szWindow = ((SnakeApp*)m_App)->GetWindowSize();
	Fission::base::rectf rect;
	rect.x = { 100.0f, (float)szWindow.width() - 100.0f };
	rect.y = { 100.0f, (float)szWindow.height() - 100.0f };

	r2d->DrawRect( rect, Fission::Colors::Aqua, 3.0f );

	r2d->Render();
}

StartScene::StartScene() { PushLayer( new StartMenuLayer ); }
