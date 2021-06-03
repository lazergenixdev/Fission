#include "ConfigurationImpl.h"
#include <Fission/Core/Console.h>

#include "yaml-cpp/yaml.h"
#include <fstream>

using namespace Fission;

namespace Fission {

	static ConfigImpl g_Config;

	ConfigImpl & ConfigImpl::Get()
	{
		return g_Config;
	}

	void ConfigImpl::GetWindowProperties( Window::Properties * pFallback ) 
	{
		FISSION_ASSERT( pFallback->save != Window::NoSaveID );
		auto it = m_WindowPropertyMap.find(pFallback->save);
		if( it == m_WindowPropertyMap.end() ) return;
		auto & props = it->second;

		if( props.flags.has_value() )
			utility::set_flag( pFallback->flags, props.flags.value() );

		if( props.position.has_value() )
			pFallback->position = props.position.value();

		if( props.size.has_value() )
			pFallback->size = props.size.value();

		if( props.monitor_idx.has_value() )
			pFallback->monitor_idx = props.monitor_idx.value();
	}

	void ConfigImpl::SetWindowProperties( const Window::Properties & _Properties ) 
	{
		FISSION_ASSERT( _Properties.save != Window::NoSaveID );
		OptWindowProperties props;

		if( bool( _Properties.flags & Window::Flags::SavePosition ) )
			props.position = _Properties.position;

		if( bool( _Properties.flags & Window::Flags::SaveSize ) )
			props.size = _Properties.size;

		props.monitor_idx = _Properties.monitor_idx;
		props.flags = _Properties.flags;

		m_WindowPropertyMap[_Properties.save] = props;
	}

	static void FormatConfigFilename( std::filesystem::path & path )
	{
		path = "etc"/path;
		path.replace_extension( ".yml" );
	}

	// Get a reference to the underlying value of the optional,
	// setting its value to `def` if opt has no value.
	template <typename T, typename K>
	auto & getopt( std::optional<T> & opt, const K & def ) { return opt.has_value() ? ( opt.value() ) : ( opt = static_cast<T>( def ) ).value(); }

	bool Config::Load( std::filesystem::path _Save_Location ) noexcept
	{
		if( !std::filesystem::exists( "etc" ) )
		{
			Console::WriteLine( "'etc/' not found. Creating directory .." );

			std::filesystem::create_directory( "etc" );

#ifdef FISSION_PLATFORM_WINDOWS
			DWORD attr = GetFileAttributesW( L"etc" );
			if( !( attr & FILE_ATTRIBUTE_HIDDEN ) )
				SetFileAttributesW( L"etc", attr | FILE_ATTRIBUTE_HIDDEN );
#endif /* FISSION_PLATFORM_WINDOWS */

			return false;
		}

		FormatConfigFilename( _Save_Location );

		try
		{
			auto conf = YAML::LoadFile( _Save_Location.string() );

			if( auto windows = conf["Windows"] )
			{
				if( windows.IsSequence() )
				{
					windows["_"] = 0; // turn sequence into map
					windows.remove( "_" );
				}

				for( auto && w : windows )
				{
					auto & props = g_Config.m_WindowPropertyMap[w.first.as<Window::SaveID>()];

					if( auto position = w.second["Position"] )
					{
						props.position = base::vector2i( position[0].as<int>(0), position[1].as<int>(0) );
					}
					if( auto spos = w.second["Save Position"] )
					{
						utility::set_flag( getopt(props.flags,0), spos.as<bool>(true) ? Window::Flags::SavePosition : Window::Flags::None );
					}
					if( auto size = w.second["Size"] )
					{
						props.size = base::size( size[0].as<int>(0), size[1].as<int>(0) );
					}
					if( auto ssz = w.second["Save Size"] )
					{
						utility::set_flag( getopt(props.flags,0), ssz.as<bool>(true) ? Window::Flags::SaveSize : Window::Flags::None );
					}
					if( auto monitor = w.second["Monitor"] )
					{
						props.monitor_idx = monitor.as<int>(0);
					}
				}
			}

		}
		catch( YAML::BadFile & )
		{
			Console::WriteLine( "'%s' not found. Using default configuration ..", _Save_Location.string().c_str() );
			return false;
		}
		catch( std::exception & e )
		{
			Console::WriteLine<256>( "Error reading '%s' [%s]", _Save_Location.string().c_str(), e.what() );
			return false;
		}

		return true;
	}

	bool Config::Save( std::filesystem::path _Save_Location ) noexcept
	{
		try
		{
			FormatConfigFilename( _Save_Location );

			auto conf = YAML::Node();

			if( !g_Config.m_WindowPropertyMap.empty() )
			{
				auto windows = conf["Windows"];
				for( auto && [id, props] : g_Config.m_WindowPropertyMap )
				{
					auto wnd = windows[id];

					if( props.position.has_value() )
					{
						auto out = wnd["Position"];
						out.SetStyle( YAML::EmitterStyle::Flow );
						out.push_back( props.position.value().x );
						out.push_back( props.position.value().y );
					}
					if( props.flags.has_value() )
					{
						wnd["Save Position"] = bool( props.flags.value() & Window::Flags::SavePosition );
					}
					if( props.size.has_value() )
					{
						auto out = wnd["Size"];
						out.SetStyle( YAML::EmitterStyle::Flow );
						out.push_back( props.size->width() );
						out.push_back( props.size->height() );
					}
					if( props.flags.has_value() )
					{
						wnd["Save Size"] = bool( props.flags.value() & Window::Flags::SaveSize );
					}
					if( props.monitor_idx.has_value() )
					{
						wnd["Monitor"] = props.monitor_idx.value();
					}
				}
			}

			// save to file
			std::ofstream( _Save_Location.string() ) << conf << '\n';
		}
		catch( std::exception & e )
		{
			Console::WriteLine( "Error writing to '%s' [%s]", _Save_Location.string().c_str(), e.what() );
		}
		return true;
	}

} // namespace Fission
