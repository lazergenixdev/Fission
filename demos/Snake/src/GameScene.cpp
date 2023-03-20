#include "GameScene.h"

GameScene::GameScene( Fission::Engine * engine ) { g_engine = engine; PushLayer( new GameLayer ); }
