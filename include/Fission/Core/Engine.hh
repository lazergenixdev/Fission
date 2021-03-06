/**
* 
* @file: Engine.h
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
#include <Fission/config.h>
#include <Fission/Core/Scene.hh>
#include <Fission/Core/Graphics/Renderer.hh>

namespace Fission
{
	struct IFEngine : public IFObject
	{
		/**
		 * @brief  Get Fission Engine Version.
		 * 
		 * @param  _Maj: [out] Major Version Number.
		 * @param  _Min: [out] Minor Version Number.
		 * @param  _Pat: [out] Patch Version Number.
		 */
		virtual void GetVersion( int * _Maj, int * _Min, int * _Pat ) = 0;

		//! @brief Gets version in format: "Fission vX.Y.Z [Debug/Release]"
		virtual const char * GetVersionString() = 0;

		virtual void Run( Platform::ExitCode * ) = 0;

		//! @brief Starts a shutdown of the engine, resulting in the application closing.
		virtual void Shutdown( Platform::ExitCode ) = 0;

		virtual void LoadEngine() = 0;

		/**
		 * @brief Loads all the reasources need to run our application,
		 *          this includes: IFGraphics* and IFWindow*, and also giving 
		 *          the application a reference to the Engine instance.
		 * 
		 * @note This function can only be called once after the engine is created,
		 *			subsequent calls will trigger an exception and will have no effect.
		 */
		virtual void LoadApplication( class FApplication * app ) = 0;

		virtual void RegisterRenderer( const char * identifier, IFRenderer * renderer ) = 0;

		/**
		 * @brief  Get a renderer from it's name.
		 *
		 * @param  _Identifier: Name of the renderer you wish to retrieve;
		 *                       "$internal2D" - type: IFRenderer2D - Engine's internal 2D renderer.
		 */
		virtual IFRenderer * GetRenderer( const char * _Identifier ) = 0;

		virtual IFDebugLayer * GetDebug() = 0;

		virtual void PushScene( FScene * _Ptr_Scene ) = 0;

		virtual void CloseScene() {}

		//! @brief Limits the FPS output by the engine.
		//! @note FPS Limiter is disabled by default.
		//! @note 10 is the minimum FPS, anything lower will disable the FPS limit.
		virtual void SetFPSLimit( int fps ) = 0;
		

	}; // struct Fission::IFEngine

	FISSION_API void CreateEngine( void * instance, IFEngine ** ppEngine );

} // namespace Fission