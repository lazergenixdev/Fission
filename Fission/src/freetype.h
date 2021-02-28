#pragma once
#include "LazerEngine/config.h"
#include "lazer/exception.h"
#include "freetype/freetype.h"

namespace freetype
{
	struct Exception : public lazer::exception {
		Exception( const lazer::exception_message & msg ) noexcept;
	};

	class Face
	{
		friend class Library;
	public:
		Face() = default;

		~Face();

		FT_Face m_Face;
		Face( FT_Library lib, const lazer::file::path & path );
		Face( FT_Library lib, const void * data, size_t size );
	};

	class Library
	{
	public:

		static Face * LoadFaceFromFile( const lazer::file::path & path );
		static Face * LoadFaceFromMemory( const void * pData, const size_t size );

	private:

		static Library & Get();

		Library();
		~Library();

		std::map<std::string, std::unique_ptr<Face>> m_Faces;

		FT_Library m_Library = NULL;
	};

}

