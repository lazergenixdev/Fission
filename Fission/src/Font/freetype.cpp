#include "freetype.h"

#include <Fission/Core/Console.hh>
#include <Fission/Base/Exception.hpp>

#define _FREETYPE_THROW_FAILED( _ft_call, _Info )			if( _ft_call ) { FISSION_THROW("FreeType Exception",.append( _Info )); } (void)0
#define _FREETYPE_THROW_FAILED_EX( _ft_call, _Info, _Ex ) if( err = _ft_call ) { FISSION_THROW("FreeType Exception",.append("Error",FT_Error_String(err)).append(_Info,_Ex)); } (void)0

namespace freetype {

	FT_Library	Face::s_library   = NULL;
	int			Face::s_ref_count = 0;

	Face::Face( const std::filesystem::path & path )
	{
		FT_Error err;
		_FREETYPE_THROW_FAILED_EX(
			FT_New_Face( get_lib(), path.string().c_str(), 0, &m_Face ),
			"Loading Face from file",
			"Failed to load \"" + path.string() + "\" as a font face."
		);
	}

	static std::string ptr_to_str( const void * ptr ) {
		static char buffer[20] = {};
		sprintf( buffer, "0x%zx", (size_t)ptr );
		return buffer;
	}

	Face::Face( const void * data, size_t size )
	{
		FT_Error err;
		_FREETYPE_THROW_FAILED_EX(
			FT_New_Memory_Face( get_lib(), (FT_Byte *)data, (FT_Long)size, 0, &m_Face ),
			"Loading Face from file",
			"Failed to load memory at " + ptr_to_str( data ) + " as a font face."
		);
	}

	Face::~Face()
	{
		FT_Done_Face( m_Face );
		--s_ref_count;
		
		// No more Font Faces exist to justify keeping the FT_Library around
		if( s_ref_count <= 0 ) {
			FT_Done_FreeType( s_library );
		}
	}

	FT_Library Face::get_lib()
	{
		// Library does not exist yet
		if( s_ref_count == 0 ) {
			_FREETYPE_THROW_FAILED( FT_Init_FreeType( &s_library ), "Library Initialization" );
		}
		++s_ref_count;
		return s_library;
	}

}
