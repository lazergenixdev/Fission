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
#include <Fission/Core/Window.hh>

namespace Fission
{
	class System
	{
	public:
		enum MessageBoxFlags : util::bit_flag<32> {
			None    = 0,
			Error   = util::make_flag<0>,
			Warning = util::make_flag<1>,
			Info    = util::make_flag<2>
		};
		using MessageBoxFlag_t = util::bit_flag_t<MessageBoxFlags>;


		FISSION_API static void ShowSimpleMessageBox( 
			const string& _Title, 
			const string& _Text, 
			const MessageBoxFlag_t& _Flags = MessageBoxFlags::None,
			IFWindow * _Parent_Window = nullptr
		);

		FISSION_API static bool OpenURL( const std::filesystem::path & _URL );

		FISSION_API static bool OpenFileLocation( const std::filesystem::path & _File );

		FISSION_API static const char * GetVersionString();

	}; // class Fission::System

} // namespace Fission
