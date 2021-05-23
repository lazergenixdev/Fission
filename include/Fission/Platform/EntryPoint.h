/**
*
* USAGE:
*   include header where you create your application,
*   this file contains the main function.
*
*
* @file: EntryPoint.h
* @author: lazergenixdev@gmail.com
*
*
* This file is provided under the MIT License:
*
* Copyright (c) 2021 Lazergenix Software
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*/


#pragma once
#include "Fission/Base/Exception.h"
#include "Fission/Core/Application.h"
#include "Fission/Platform/System.h"

#define FISSION_EXIT_SUCCESS 0x0 /* Success */
#define FISSION_EXIT_FAILURE 0x1 /* Failure */
#define FISSION_EXIT_UNKNOWN 0x2 /* Failure */


/* ================================================================================================== */
/* -------------------------------------- Function Definitions -------------------------------------- */
/* ================================================================================================== */

//! @brief Creates User Application.
//! @return Pointer to a Fission::Application.
static Fission::Application * CreateApplication( void );

//! @brief Runs main application loop.
//! @return ExitCode from the status of how the application exited.
inline Fission::Platform::ExitCode _fission_main( void );


/* =================================================================================================== */
/* -------------------------------------- Platform Entry Points -------------------------------------- */
/* =================================================================================================== */

#ifdef FISSION_PLATFORM_WINDOWS

int WINAPI wWinMain( _In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int ) {
	return _fission_main();
}

#endif // FISSION_PLATFORM_WINDOWS


/* =================================================================================================== */
/* ------------------------------------ fission main definition -------------------------------------- */
/* =================================================================================================== */

inline Fission::Platform::ExitCode _fission_main()
{
	using namespace Fission;

	Platform::ExitCode ec = FISSION_EXIT_UNKNOWN;

	try 
	{
		System::Initialize();

		auto _App = CreateApplication();

		try 
		{
			ec = _App->Run();
		}
		catch( base::base_exception & e ) {
			_App->GetWindow()->DisplayMessageBox( utf8_to_wstring( e.name() ) + L" caught", utf8_to_wstring( e.what() ) );
			ec = FISSION_EXIT_FAILURE;
		}
		catch( std::exception & e ) {
			_App->GetWindow()->DisplayMessageBox( L"C++ exception caught", utf8_to_wstring( e.what() ) );
			ec = FISSION_EXIT_FAILURE;
		}
		catch( ... ) {
			_App->GetWindow()->DisplayMessageBox( L"Unknown Error", L"No information provided, lol" );
		}

		delete _App;

		System::Shutdown();
	}
	catch( base::base_exception * e ) {
		System::DisplayMessageBox( utf8_to_wstring( e->name() ) + L" caught", utf8_to_wstring( e->what() ) );
		ec = FISSION_EXIT_FAILURE;
	}
	catch( std::exception & e ) {
		System::DisplayMessageBox( L"C++ exception caught", utf8_to_wstring( e.what() ) );
		ec = FISSION_EXIT_FAILURE;
	}
	catch( ... ) {
		System::DisplayMessageBox( L"Unknown Error", L"No information provided, lol" );
	}

	return ec;
}