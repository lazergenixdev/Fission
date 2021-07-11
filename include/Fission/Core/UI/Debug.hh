/**
*
* @file: UI.hh
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

#ifndef FISSION_ENABLE_DEBUG_UI
	#if defined(FISSION_DIST)
		#define FISSION_ENABLE_DEBUG_UI 0
	#else
		#define FISSION_ENABLE_DEBUG_UI 1
	#endif
#endif

#define FISSION_DEBUG_API(RET) FISSION_API static RET

namespace Fission::UI
{

	//class UILayer : public IFLayer
	//{
	//public:
	//	UILayer(neutron::Context* ctx, bool owned=false){}

	//public:	
	//	neutron::Context* context;
	//};

	class Debug
	{
	public:

#if FISSION_ENABLE_DEBUG_UI

		FISSION_DEBUG_API(void) Text( const char * text );

		FISSION_DEBUG_API(bool) Button( const char * label );
		FISSION_DEBUG_API(bool) CheckBox( const char * label, bool * value );

		FISSION_DEBUG_API(bool) InputFloat( const char * label, float * value, const char * format = "%.3f");
		FISSION_DEBUG_API(bool) InputInt( const char * label, int * value );

		FISSION_DEBUG_API(bool) SliderFloat( const char * label, float * value, const char * format = "%.3f" );
		FISSION_DEBUG_API(bool) SliderInt( const char * label, int * value );

#endif
	};

} // namespace Fission::UI
