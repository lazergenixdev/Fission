#pragma once
#include "Window.h"
#include "../Platform/System.h"

//#define LAZER_EXCEPTION() lazer::Exception( __LINE__, __FILEW__ )

namespace lazer {

	static void error(const char * what ) {

	}

	//class Exception
	//{
	//public:
	//	Exception( int line, const wchar_t * file );

	//	int get_line() const				{ return m_Line; }
	//	const wchar_t * get_file() const	{ return m_File; }

	//	// gets files with line number
	//	const wchar_t * get_location() const {
	//		// "file: "stuff.cpp" @line 23"
	//	}

	//	const Exception & append_message( std::wstring & msg ) {
	//		m_What += L"\n\n";
	//		m_What += msg;
	//	}

	//	const wchar_t * get_message( int index ) const {
	//		
	//	}

	//	const int num_messages() const {

	//	}

	//	virtual const wchar_t * what() const { return m_What.c_str(); };

	//	virtual const wchar_t * name() const = 0;

	//	virtual ~Exception() = default;

	//	void Display( Window * window = nullptr )
	//	{
	//		if( window )
	//			window->DisplayMessageBox( std::wstring( name() ) + L" caught!", what() );
	//		else
	//			System::DisplayMessageBox( std::wstring( name() ) + L" caught!", what() );
	//	}
	//private:
	//	const wchar_t * m_File;
	//	int m_Line;
	//protected:
	//	std::wstring m_What;
	//};

} // namespace lazer
