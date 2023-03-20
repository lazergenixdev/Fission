/**
 *	______________              _____
 *	___  ____/__(_)________________(_)____________
 *	__  /_   __  /__  ___/_  ___/_  /_  __ \_  __ \
 *	_  __/   _  / _(__  )_(__  )_  / / /_/ /  / / /
 *	/_/      /_/  /____/ /____/ /_/  \____//_/ /_/
 *
 *
 * @Author:       lazergenixdev@gmail.com
 * @Development:  (https://github.com/lazergenixdev/Fission)
 * @License:      MIT (see end of file)
 *
 * USAGE:
 *   include header where you create your application,
 *   this file contains the main function.
 */
#pragma once
#include <Fission/Base/Exception.hpp>
#include <Fission/Base/util/SmartPointer.hpp>
#include <Fission/Core/Engine.hh>
#include <Fission/Core/Application.hh>
#include <Fission/Platform/System.h>

#define FISSION_EXIT_SUCCESS 0 /* Success */
#define FISSION_EXIT_FAILURE 1 /* Failure */
#define FISSION_EXIT_UNKNOWN 2 /* Failure */


/* ================================================================================================== */
/* -------------------------------------- Function Definitions -------------------------------------- */
/* ================================================================================================== */

//! @brief Creates User Application.
//! @return Pointer to a Fission::FApplication.
extern Fission::Application * CreateApplication( void );

//! @brief Runs main application loop.
//! @return ExitCode from the status of how the application exited.
FISSION_MAIN_EXPORT(Fission::Platform::ExitCode) _fission_main( void * );


/* =================================================================================================== */
/* -------------------------------------- Platform Entry Points -------------------------------------- */
/* =================================================================================================== */

#if defined(FISSION_PLATFORM_WINDOWS)

// TODO: Try `__try` and `__except` https://docs.microsoft.com/en-us/cpp/cpp/try-except-statement?view=msvc-170

int WINAPI wWinMain( _In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int ) {
	// Instance data is not needed on Windows, as there are
	// WinAPI functions to get all the information needed; so
	// there is no reason to pass anything to our main function.
	return _fission_main(nullptr);
}

#elif defined(FISSION_PLATFORM_LINUX)

#elif defined(FISSION_PLATFORM_ANDROID)

/**
 * I've been looking into Android development, and the only way to
 * get a C++ program up and running is to compile into a shared library (.so)
 * and call our C++ functions from a Java Android Activity.
 * 
 * So there is no need for a platform main function, our `_fission_main`
 * will work just fine.
 */

#endif


/* =================================================================================================== */
/* ------------------------------------ fission main definition -------------------------------------- */
/* =================================================================================================== */

FISSION_MAIN_EXPORT(Fission::Platform::ExitCode) _fission_main( void * instance )
{
	using namespace Fission;
	using sv = string_view;

	Platform::ExitCode    nExitCode = FISSION_EXIT_UNKNOWN;
	fsn_ptr<Engine>		  fsnEngine;
	fsn_ptr<Application>  app;

	try
	{
		/* Initialize Fission Engine. */
		Fission::CreateEngine( instance, &fsnEngine );

		/* Initialize the Application. (User code) */
		app = CreateApplication();

		/* Attach the Application to Fission Engine. */
		fsnEngine->LoadApplication( app.get() );

		try 
		{
			/* Use the Engine to run our Application. */
			fsnEngine->Run( &nExitCode );
		}

		catch( base::runtime_error & e ) {
			System::ShowSimpleMessageBox( sv{ e.name() }, sv{ e.what() }, System::Error, app->f_pMainWindow );
			nExitCode = FISSION_EXIT_FAILURE;
		}

		catch( std::exception & e ) {
			System::ShowSimpleMessageBox( u8"C++ exception caught", sv{ e.what() }, System::Error, app->f_pMainWindow );
			nExitCode = FISSION_EXIT_FAILURE;
		}

		catch( ... ) {
			System::ShowNativeMessageBox(
				FISSION_PLATFORM_STRING("Unknown Error"),
				FISSION_PLATFORM_STRING("No information provided, lol"),
				System::Error, app->f_pMainWindow
			);
		}
	}

	catch( base::runtime_error & e ) {
		System::ShowSimpleMessageBox( sv{ e.name() }, sv{ e.what() }, System::Error );
		nExitCode = FISSION_EXIT_FAILURE;
	}

	catch( std::exception & e ) {
		System::ShowSimpleMessageBox( u8"C++ exception caught", sv{ e.what() }, System::Error );
		nExitCode = FISSION_EXIT_FAILURE;
	}

	catch( ... ) {
		System::ShowNativeMessageBox(
			FISSION_PLATFORM_STRING( "Unknown Error" ),
			FISSION_PLATFORM_STRING( "No information provided, lol" ),
			System::Error
		);
	}

	return nExitCode;
}

/**
 *	MIT License
 *
 *	Copyright (c) 2021-2023 lazergenixdev
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this software and associated documentation files (the "Software"), to deal
 *	in the Software without restriction, including without limitation the rights
 *	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *	copies of the Software, and to permit persons to whom the Software is
 *	furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in all
 *	copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 */