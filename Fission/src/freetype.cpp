#include "freetype.h"

#include "LazerEngine/Core/Console.h"

#define _LAZER_FREETYPE_THROW_FAILED( _ft_call, _Info )			if( _ft_call ) throw freetype::Exception(_lazer_exception_msg.append( _Info ))
#define _LAZER_FREETYPE_THROW_FAILED_EX( _ft_call, _Info, _Ex ) if( err = _ft_call ) throw freetype::Exception(_lazer_exception_msg.append("Error",FT_Error_String(err)).append(_Info,_Ex))

namespace freetype {

	Face::Face( FT_Library lib, const lazer::file::path & path )
	{
		FT_Error err;
		_LAZER_FREETYPE_THROW_FAILED_EX(
			FT_New_Face( lib, path.string().c_str(), 0, &m_Face ),
			"Loading Face from file",
			"Failed to load \"" + path.string() + "\" as a font face."
		);
	}

	static std::string ptr_to_str( const void * ptr ) {
		static char buffer[20] = {};
		sprintf( buffer, "0x%zx", (size_t)ptr );
		return buffer;
	}

	Face::Face( FT_Library lib, const void * data, size_t size )
	{
		FT_Error err;
		_LAZER_FREETYPE_THROW_FAILED_EX(
			FT_New_Memory_Face( lib, (FT_Byte *)data, (FT_Long)size, 0, &m_Face ),
			"Loading Face from file",
			"Failed to load memory at " + ptr_to_str( data ) + " as a font face."
		);
	}

	Face::~Face()
	{
		FT_Done_Face( m_Face );
	}

	Library::Library() {
		_LAZER_FREETYPE_THROW_FAILED( FT_Init_FreeType( &m_Library ), "Library Initialization" );

		int maj, min, pat;

		FT_Library_Version( m_Library, &maj, &min, &pat );

		static wchar_t buf[48] = {};
		swprintf( buf, 48, L"Freetype Library Version %i.%i.%i", maj, min, pat );

		lazer::Console::WriteLine( buf );
	}

	Library::~Library() {
		m_Faces.clear();
		if( m_Library )
			FT_Done_FreeType( m_Library );
	}

	Face * Library::LoadFaceFromFile( const lazer::file::path & path )
	{
		std::string location = path.string();

		auto key = Get().m_Faces.find( location );
		if( key != Get().m_Faces.end() ) {
			return key->second.get();
		}

		Get().m_Faces.emplace( location, std::make_unique<Face>( Get().m_Library, path ) );

		return Get().m_Faces[location].get();
	}

	Face * Library::LoadFaceFromMemory( const void * pData, const size_t size )
	{
		char buf[16];
		sprintf( buf, "%I64x", (uint64_t)pData );
		std::string location = buf;

		auto key = Get().m_Faces.find( location );
		if( key != Get().m_Faces.end() ) {
			return key->second.get();
		}

		Get().m_Faces.emplace( location, std::make_unique<Face>( Get().m_Library, pData, size ) );

		return Get().m_Faces[location].get();
	}

	Library & Library::Get()
	{
		static Library s_ftLib;
		return s_ftLib;
	}

	Exception::Exception( const lazer::exception_message & msg ) noexcept
		: lazer::exception( "Freetype Exception", msg )
	{}

}
