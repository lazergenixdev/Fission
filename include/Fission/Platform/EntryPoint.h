/* ===================================================== */
/* - File: "EntryPoint.h" ------------------------------ */
/* - Author: Lazergenix (lazergenixdev@gmail.com) ------ */
/* - Last Modified: 2021-02-02 ------------------------- */
/* ===================================================== */
/*
 
 Copyright (c) 2021 Lazergenix Software

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

//
// USAGE:
//   include header where you create your application
//   this file contains the main function
//

#pragma once
#include "LazerEngine/Core/Application.h"
#include "LazerEngine/Core/Exception.h"
#include "LazerEngine/Platform/System.h"
#include "lazer/string.h"

#define LAZER_EXIT_SUCCESS 0x0 /* Success */
#define LAZER_EXIT_FAILURE 0x1 /* Failure */
#define LAZER_EXIT_UNKNOWN 0x2 /* Failure */


/* ================================================================================================== */
/* -------------------------------------- Function Definitions -------------------------------------- */
/* ================================================================================================== */

/// <summary>
/// Creates User Application.
/// </summary>
/// <returns>Pointer to a lazer::Application</returns>
extern lazer::Application * CreateApplication( void );

/// <summary>
/// Runs main application loop
/// </summary>
/// <returns>ExitCode from the status of how the application exited</returns>
inline lazer::Platform::ExitCode _lazer_main( void );


/* =================================================================================================== */
/* -------------------------------------- Platform Entry Points -------------------------------------- */
/* =================================================================================================== */

#ifdef LAZER_PLATFORM_WINDOWS

int WINAPI wWinMain( _In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int ) {
	return _lazer_main();
}

#endif // LAZER_PLATFORM_WINDOWS


/* =================================================================================================== */
/* -------------------------------------- lazer main definition -------------------------------------- */
/* =================================================================================================== */

lazer::Platform::ExitCode _lazer_main()
{
	lazer::Platform::ExitCode ec = LAZER_EXIT_UNKNOWN;

	try 
	{
		lazer::System::Initialize();

		auto _App = CreateApplication();

		try 
		{
			ec = _App->Run();
		}
		catch( lazer::exception & e ) {
			_App->GetWindow()->DisplayMessageBox( lazer::utf8_to_wstring( e.name() ) + L" caught", lazer::utf8_to_wstring( e.what() ) );
			ec = LAZER_EXIT_FAILURE;
		}
		catch( std::exception & e ) {
			_App->GetWindow()->DisplayMessageBox( L"C++ exception caught", lazer::utf8_to_wstring( e.what() ) );
			ec = LAZER_EXIT_FAILURE;
		}
		catch( ... ) {
			_App->GetWindow()->DisplayMessageBox( L"Unknown Error", L"No information provided, lol" );
		}

		delete _App;

		lazer::System::Shutdown();
	}
	catch( lazer::exception & e ) {
		lazer::System::DisplayMessageBox( lazer::utf8_to_wstring( e.name() ) + L" caught", lazer::utf8_to_wstring( e.what() ) );
		ec = LAZER_EXIT_FAILURE;
	}
	catch( std::exception & e ) {
		lazer::System::DisplayMessageBox( L"C++ exception caught", lazer::utf8_to_wstring( e.what() ) );
		ec = LAZER_EXIT_FAILURE;
	}
	catch( ... ) {
		lazer::System::DisplayMessageBox( L"Unknown Error", L"No information provided, lol" );
	}

	return ec;
}