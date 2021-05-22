/**
*
* @file: System.h
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
#include "Fission/config.h"

// todo: change comment style to be consistant with rest of project

namespace Fission {

	/// <summary>
	/// System acts as an interface for the platform operating system
	/// </summary>
	class System
	{
	public:
		/// <summary>
		/// Initialize Summary.
		/// </summary>
		FISSION_API static void Initialize();

		/// <summary>
		/// Shutdown Summary.
		/// </summary>
		FISSION_API static void Shutdown();

		/// <summary>
		/// Blocks current thread with a OK message box.
		/// </summary>
		/// <param name="Title">- What gets set as the title.</param>
		/// <param name="Text">.</param>
		FISSION_API static void DisplayMessageBox( const std::wstring & _Title, const std::wstring & _Text );

		/// <summary>
		/// Save text to the Clipboard.
		/// </summary>
		/// <param name="Text">- utf8 string to put onto the clipboard.</param>
		FISSION_API static void SaveToClipboard( const std::string & _Text );

		FISSION_API static bool OpenURL( const std::string & _URL );
		FISSION_API static bool OpenFile( const file::path & _File );

		/// <summary>
		/// Writes out information about the system to the console.
		/// </summary>
		FISSION_API static void WriteInfoToConsole();

	}; // class Fission::System

} // namespace Fission
