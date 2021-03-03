//
// USAGE:
//   include header where you create your application
//   this file contains the main function
//

#pragma once
#include "Fission/Core/Application.h"
#include "Fission/Platform/System.h"

#define FISSION_EXIT_SUCCESS 0x0 /* Success */
#define FISSION_EXIT_FAILURE 0x1 /* Failure */
#define FISSION_EXIT_UNKNOWN 0x2 /* Failure */


/* ================================================================================================== */
/* -------------------------------------- Function Definitions -------------------------------------- */
/* ================================================================================================== */

/// <summary>
/// Creates User Application.
/// </summary>
/// <returns>Pointer to a Fission::Application</returns>
static Fission::Application * CreateApplication( void );

/// <summary>
/// Runs main application loop
/// </summary>
/// <returns>ExitCode from the status of how the application exited</returns>
static Fission::Platform::ExitCode _fission_main( void );


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

Fission::Platform::ExitCode _fission_main()
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
		catch( exception & e ) {
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
	catch( exception & e ) {
		System::DisplayMessageBox( utf8_to_wstring( e.name() ) + L" caught", utf8_to_wstring( e.what() ) );
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