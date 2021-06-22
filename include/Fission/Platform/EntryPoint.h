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
#include <Fission/Base/Exception.h>
#include <Fission/Base/Utility/SmartPointer.h>
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
extern Fission::FApplication * CreateApplication( void );

//! @brief Runs main application loop.
//! @return ExitCode from the status of how the application exited.
FISSION_MAIN_EXPORT(Fission::Platform::ExitCode) _fission_main( void * );


/* =================================================================================================== */
/* -------------------------------------- Platform Entry Points -------------------------------------- */
/* =================================================================================================== */

#if defined(FISSION_PLATFORM_WINDOWS)

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
	using namespace string_literals;

	Platform::ExitCode      nExitCode = FISSION_EXIT_UNKNOWN;
	FPointer<IFEngine>      fsnEngine;
	FPointer<FApplication>  app;

	Fission::CreateEngine( instance, &fsnEngine );

	try
	{
		fsnEngine->LoadEngine();

		app = CreateApplication();

		fsnEngine->LoadApplication( app.get() );

		try 
		{
			fsnEngine->Run( &nExitCode );
		}

		catch( base::runtime_error & e )
		{
			System::ShowSimpleMessageBox( string( e.name() ) + " caught"_utf8, e.what(), System::Error, app->pMainWindow );
			nExitCode = FISSION_EXIT_FAILURE;
		}

		catch( std::exception & e )
		{
			System::ShowSimpleMessageBox( "C++ exception caught"_utf8, e.what(), System::Error, app->pMainWindow );
			nExitCode = FISSION_EXIT_FAILURE;
		}

		catch( ... ) 
		{
			System::ShowSimpleMessageBox( "Unknown Error"_utf8, "No information provided, lol"_utf8, System::Error, app->pMainWindow );
		}
	}

	catch( base::runtime_error & e )
	{
		System::ShowSimpleMessageBox( string( e.name() ) + " caught"_utf8, e.what(), System::Error );
		nExitCode = FISSION_EXIT_FAILURE;
	}

	catch( std::exception & e )
	{
		System::ShowSimpleMessageBox( "C++ exception caught"_utf8, e.what(), System::Error );
		nExitCode = FISSION_EXIT_FAILURE;
	}

	catch( ... )
	{
		System::ShowSimpleMessageBox( "Unknown Error"_utf8, "No information provided, lol"_utf8, System::Error );
	}

	return nExitCode;
}