#include <LazerEngine/Core/Configuration.h>
#include <LazerEngine/Core/Console.h>

#include "yaml-cpp/yaml.h"
#include "lazer/files.h"
#include <fstream>

static std::string ConfFile = "etc/appconf.yml";

using namespace lazer;

namespace convert {

	template <typename T>
	static T decode( const std::string & );

	template <typename T>
	static std::string encode( const T & );

	template <>
	static std::string encode<Window::Style>( const Window::Style & style ) {
		switch( style )
		{
		case Window::Style::Borderless:
			return "Borderless";
		case Window::Style::Border:
			return "Border";
		case Window::Style::BorderSizeable:
			return "BorderSizeable";
		default: 
			return "Default";
		}
	}

	template <>
	static std::string encode<bool>( const bool & b ) {
		switch( b )
		{
		case 0: return "False";
		default: return "True";
		}
	}

	template <>
	static Window::Style decode<Window::Style>( const std::string & s ) {
		if( s == "Border" )			return Window::Style::Border;
		if( s == "BorderSizeable" ) return Window::Style::BorderSizeable;
		if( s == "Borderless" )		return Window::Style::Borderless;
		return Window::Style::Default;
	}

}

struct ConfigurationInfo 
{
	Configuration::WindowConfig		Window;
	Configuration::GraphicsConfig	Graphics;
};

static ConfigurationInfo Conf{};

Window::Properties Configuration::GetWindowConfig( const Window::Properties & fallback ) noexcept
{
	Window::Properties wnd = fallback;
	utility::set_flag<(utility::flag)Window::Flags::IsMainWindow>( wnd.flags );
	if( auto style = Conf.Window.Style )
	{
		wnd.style = convert::decode<Window::Style>( style.value() );
	}
	if( auto full = Conf.Window.Fullscreen )
	{
		if( full.value() )
			utility::set_flag<(utility::flag)Window::Flags::Fullscreen>( wnd.flags );
		else
			utility::remove_flag<(utility::flag)Window::Flags::Fullscreen>( wnd.flags );
	}
	if( Conf.Window.Position )
	{
		wnd.position = Conf.Window.Position.value();
	}
	if( Conf.Window.Size )
	{
		wnd.size = Conf.Window.Size.value();
	}
	if( auto save = Conf.Window.SavePosition )
	{
		if( save.value() )
			utility::set_flag<(utility::flag)Window::Flags::SavePosition>( wnd.flags );
		else
			utility::remove_flag<(utility::flag)Window::Flags::SavePosition>( wnd.flags );
	}
	if( auto save = Conf.Window.SaveSize )
	{
		if( save.value() )
			utility::set_flag<(utility::flag)Window::Flags::SaveSize>( wnd.flags );
		else
			utility::remove_flag<(utility::flag)Window::Flags::SaveSize>( wnd.flags );
	}
	return wnd;
}

void Configuration::SetWindowConfig( const Window::Properties & props ) noexcept
{
	Conf.Window.Style = convert::encode( props.style );
	Conf.Window.Fullscreen = bool( props.flags & Window::Flags::Fullscreen );

	Conf.Window.SavePosition = bool( props.flags & Window::Flags::SavePosition );
	Conf.Window.SaveSize = bool( props.flags & Window::Flags::SaveSize );

	if( Conf.Window.SavePosition.value() )
	{
		Conf.Window.Position = props.position;
	}
	if( Conf.Window.SaveSize.value() )
	{
		Conf.Window.Size = props.size;
	}
}

void Configuration::Load() noexcept
{
	if( !file::exists( "etc" ) )
	{
		lazer::Console::WriteLine( L"'etc/' not found. Creating directory .." );

		file::create_directory( "etc" );

#ifdef LAZER_PLATFORM_WINDOWS
		DWORD attr = GetFileAttributesW( L"etc" );
		if( !( attr & FILE_ATTRIBUTE_HIDDEN ) )
			SetFileAttributesW( L"etc", attr | FILE_ATTRIBUTE_HIDDEN );
#endif /* LAZER_PLATFORM_WINDOWS */
		
		return;
	}

	try 
	{
		auto yaml = YAML::LoadFile( ConfFile );

		if( auto wnd = yaml["Window"] )
		{
			if( auto style = wnd["Style"] )
			{
				Conf.Window.Style = style.as<std::string>();
			}
			if( auto full = wnd["Fullscreen"] )
			{
				Conf.Window.Fullscreen = full.as<bool>();
			}
			if( auto pos = wnd["Position"] )
			{
				Conf.Window.Position = lazer::vec2i{};
				Conf.Window.Position.value().x = pos["x"].as<int>();
				Conf.Window.Position.value().y = pos["y"].as<int>();
				Conf.Window.SavePosition = pos["Save On Exit"].as<bool>();
			}
			if( auto size = wnd["Size"] )
			{
				Conf.Window.Size = lazer::vec2i{};
				Conf.Window.Size.value().x = size["w"].as<int>();
				Conf.Window.Size.value().y = size["h"].as<int>();
				Conf.Window.SaveSize = size["Save On Exit"].as<bool>();
			}
		}

		if( yaml["Graphics"] )
		{
			if( auto res = yaml["Resolution"] )
			{
				Conf.Graphics.Resolution = lazer::vec2i{};
				Conf.Graphics.Resolution.value().x = res["w"].as<int>();
				Conf.Graphics.Resolution.value().y = res["h"].as<int>();
			}
			if( auto api = yaml["API"] )
			{
				Conf.Graphics.API = api.as<std::string>();
			}
			if( auto fr = yaml["Frame Rate"] )
			{
				Conf.Graphics.FrameRate = fr.as<std::string>();
			}
			if( auto fsaa = yaml["FSAA"] )
			{
				Conf.Graphics.FSAA = fsaa.as<std::string>();
			}
		}

	}
	catch( YAML::BadFile & ) 
	{
		lazer::Console::WriteLine( L"'appconf.yml' not found. Using default configuration .." );
	}
	catch( std::exception & e ) 
	{
		std::wstring what = utf8_to_wstring( e.what() );
		lazer::Console::WriteLine( L"Error reading 'appconf.yml' [%s]", what.c_str() );
	}

}

void Configuration::Save() noexcept
{
	try 
	{
		std::ofstream file;
		file.exceptions( std::ios::badbit | std::ios::failbit );

		file.open( ConfFile );

		Window::Properties defwnd = {};

		file << "Window: {\n\tStyle: ";
		file << Conf.Window.Style.value_or( convert::encode( defwnd.style ) );
		file << ",\n\n\tFullscreen: ";
		file << convert::encode<bool>( Conf.Window.Fullscreen.value_or( bool(defwnd.flags & Window::Flags::Fullscreen) ) );
		file << ", \n\n\tPosition: {\n\t\tx: ";
		file << Conf.Window.Position.value_or( lazer::vec2i{} ).x;
		file << ",\n\t\ty: ";
		file << Conf.Window.Position.value_or( lazer::vec2i{} ).y;
		file << ",\n\t\tSave On Exit: ";
		file << convert::encode( Conf.Window.SavePosition.value_or( bool( defwnd.flags & Window::Flags::SavePosition ) ) );
		file << "\n\t},\n\n\tSize: {\n\t\tw: ";
		file << Conf.Window.Size.value_or( defwnd.size ).x;
		file << ",\n\t\th: ";
		file << Conf.Window.Size.value_or( defwnd.size ).y;
		file << ",\n\t\tSave On Exit: ";
		file << convert::encode( Conf.Window.SavePosition.value_or( bool( defwnd.flags & Window::Flags::SaveSize ) ) );
		file << "\n\t}\n}";

	}
	catch( std::exception & e )
	{
		std::wstring what = utf8_to_wstring( e.what() );
		lazer::Console::Error( L"Failed to save to 'appconf.yml' [%s]", what.c_str() );
	}
}
