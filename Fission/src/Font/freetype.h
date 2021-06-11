#pragma once
#include "Fission/config.h"
#include "freetype/freetype.h"

namespace freetype
{
	class Face
	{
		friend class Library;
	public:
		Face() = default;

		~Face();

		FT_Face m_Face;
		Face( FT_Library lib, const std::filesystem::path & path );
		Face( FT_Library lib, const void * data, size_t size );
	};

	class Library
	{
	public:

		static Face * LoadFaceFromFile( const std::filesystem::path & path );
		static Face * LoadFaceFromMemory( const void * pData, const size_t size );

	private:

		static Library & Get();

		Library();
		~Library();

		std::map<std::string, std::unique_ptr<Face>> m_Faces;

		FT_Library m_Library = NULL;
	};

}

