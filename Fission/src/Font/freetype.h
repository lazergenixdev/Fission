#pragma once
#include "freetype/freetype.h"
#include <map>
#include <filesystem>

namespace freetype
{
	class Face
	{
	public:
		FT_Face m_Face;

		Face() = default;
		~Face();

		Face( const std::filesystem::path & path );
		Face( const void * data, size_t size );

	private:
		FT_Library get_lib();

	private:
		static FT_Library	s_library;
		static int			s_ref_count;
	};
}


