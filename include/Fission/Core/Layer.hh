/**
*
* @file: Layer.h
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

// todo: redesign debug layer interface to not be SLOW AF!

#pragma once
#include <Fission/Base/TimeStep.h>
#include <Fission/Core/Object.hh>
#include <Fission/Core/Input/Event.hh>

namespace Fission
{
	// experimental
	struct IFtest
	{
		static struct IFEngine * s_pEngine;
	};

	struct IFLayer : public IFEventHandler, public IFObject, virtual public IFtest
	{
		//! @note If you got any resources that need to be sent to the GPU, 
		//!			now is the time to do it
		virtual void OnCreate( class FApplication * app ) = 0;

		//! @brief Function to update what is displayed on a frame
		virtual void OnUpdate( timestep dt ) = 0;
		
	}; // struct Fission::IFLayer

/* =================================================================================================== */
/* ----------------------------------------- Default Layers ------------------------------------------ */
/* =================================================================================================== */

	using DrawCallback = std::function<void( struct IFRenderer2D * _Renderer, void * _UserData )>;

	struct DebugFrameInfo
	{
		float * frame_times_array;
		int frame_times_count;

		int current_frame;
	};

	class IFDebugLayer : public IFLayer
	{
	public:
		virtual void RegisterDrawCallback( const char * _Key, DrawCallback _Callback, void * _UserData ) = 0;

		virtual void UnregisterDrawCallback( const char * _Key ) {};

		virtual void Text( const char * what ) = 0;

		template <size_t Buffer_Size = 128, typename...T>
		void Text( const char * fmt, T&&...args )
		{
			char _buffer[Buffer_Size];
			sprintf_s( _buffer, fmt, std::forward<T>( args )... );
			Text( _buffer );
		}

		virtual void GetFrameInfo( DebugFrameInfo * dest ) {};
	};

	struct IFConsoleLayer : public IFLayer {};

} // nanespace Fission
