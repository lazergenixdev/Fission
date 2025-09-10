#include "Fission/core.hpp"

//struct Scene: public fs::Scene
//{
//};

//App_Info::App_Info():
//    version      ( fs::make_compressed_version<0,1,7> ),
//    version_info ( "vanilla/dev" ),
//    name         ( "line-effect" )
//{}

auto on_create () -> fission::Defaults
{
    return {
        .window_title = "네, 한국어 정말 재미있어요! ❤🎄",
        .window_width  = 1280,
        .window_height =  720,
    };
}

//auto on_create_scene (fs::Scene_Key const& key) -> fs::Scene *
//{
//    (void)key;
//    return new Scene;
//}