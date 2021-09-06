#include <Fission/Platform/EntryPoint.h>

#include "SnakeApp.h"
#include "GameScene.h"
#include "StartScene.h"

void SnakeApp::OnStartUp( CreateInfo * info )
{
	char title[100];
	sprintf( title, "Snek Demo [%s]", pEngine->GetVersionString() );
	info->window.title = title;
	info->window.size = { 800, 600 };
	info->startScene = new GameScene();

	strcpy_s(info->name_utf8, "snek");

	m_WindowSize = info->window.size;
}

Fission::base::size SnakeApp::GetWindowSize() const
{
	return m_WindowSize;
}

Fission::FApplication * CreateApplication() {
	return new SnakeApp;
}