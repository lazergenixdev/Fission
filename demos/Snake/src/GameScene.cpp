#include "GameScene.h"

GameScene::GameScene( Fission::IFEngine * engine ) { g_engine = engine; PushLayer( new GameLayer ); }
