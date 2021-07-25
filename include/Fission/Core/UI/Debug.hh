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

#if FISSION_ENABLE_DEBUG_UI
#define FISSION_DEBUG_UI_FN(ret,func,...) FISSION_API static ret func( __VA_ARGS__ )
#define FISSION_RET_VOID ;
#define FISSION_RET_BOOL ;
#else
#define FISSION_DEBUG_UI_FN(ret,func,...) static constexpr   ret func( __VA_ARGS__ )
#define FISSION_RET_VOID {}
#define FISSION_RET_BOOL {return false;}
#endif

namespace Fission::UI
{

	//class UILayer : public IFLayer
	//{
	//public:
	//	UILayer(neutron::Context* ctx, bool owned=false){}

	//public:	
	//	neutron::Context* context;
	//};

	// small little problem with these debug functions,
	// is that they are designed after ImGui.
	// 
	// While this is not a problem on it's own, ImGui is an immediate mode
	//  GUI where the logic is designed to be run on a single thread (the same thread where you would render).
	//
	// This is a problem for two reasons:
	// - This engine is designed to utilize 2 threads, one for rendering and one for event handling;
	//     which means that mutexes are necessary for consistant memory access (not good for performance and does not scale very well)
	//
	// - These functions can ONLY be used on the rendering thread, this limits their debug capabilities and range for tweaking values.


	class Debug
	{
	public:
		FISSION_DEBUG_UI_FN(void, Text, const char * text ) FISSION_RET_VOID

		FISSION_DEBUG_UI_FN(bool, Button,   const char * label ) FISSION_RET_BOOL
	//	FISSION_DEBUG_UI_FN(bool, CheckBox, const char * label, bool * value ) FISSION_RET_BOOL

	//	FISSION_DEBUG_UI_FN(bool, InputFloat, const char * label, float * value, const char * format = "%.3f") FISSION_RET_BOOL
	//	FISSION_DEBUG_UI_FN(bool, InputInt  , const char * label, int   * value, const char * format = "%i"  ) FISSION_RET_BOOL

		FISSION_DEBUG_UI_FN(bool, SliderFloat, const char * label, float * value, float min, float max, const char * format = "%.3f" ) FISSION_RET_BOOL
		FISSION_DEBUG_UI_FN(bool, SliderInt  , const char * label, int   * value, int   min, int   max, const char * format = "%i"   ) FISSION_RET_BOOL
	};

} // namespace Fission::UI
