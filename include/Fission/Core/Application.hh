/**
*
* @file: Application.h
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
#include <Fission/Core/Engine.hh>
#include <Fission/Core/Graphics.hh>
#include <Fission/Core/Window.hh>

namespace Fission
{
	class FApplication : public IFObject
	{
	public:
		IFEngine * pEngine   = nullptr;
		IFWindow * pMainWindow = nullptr;
		IFGraphics * pGraphics = nullptr;

	public:

		struct CreateInfo
		{
			FScene *              startScene;
			IFWindow::Properties  window = {};
			IFGraphics::State     graphics = {};
		};

	public:

		// called only once: after Engine is initialized.
		virtual void OnStartUp( CreateInfo * pCreateInfo ) {}

		// called after a Graphics context has been created.
		virtual void OnCreate() {}

		// called right before Graphics context gets destroyed and the program exits.
		virtual void OnShutdown() {}

	public:
		constexpr FApplication() noexcept = default;

		FApplication( const FApplication & ) = delete; /*!< Copying Applications is not allowed. */

	}; // class Fission::FApplication

} // namespace Fission
