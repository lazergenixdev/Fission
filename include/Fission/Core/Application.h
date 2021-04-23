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
#include "Fission/config.h"
#include "Window.h"
#include "Graphics/Graphics.h"
#include "Scene.h"

namespace Fission {

	/*****************************************************************
	 * 
	 * @class:
	 *	 Application
	 * 
	 * @description:
	 *   Base Application structure that represents the
	 *   main application instance.
	 * 
	 * @note:
	 *   Override this class to construct your main application.  
	 * 
	 *	 This structure can only be constructed once at the start of
	 *   a program.
	 * 
	 */
	class Application : public IEventHandler
	{
	public:

		/********************************************************************
		 * 
		 * @struct:
		 *	 CreateInfo
		 * 
		 * @description:
		 *   Represents the starting configuration of the application.
		 * 
		 * @note:
		 *   These values will only be used IF there is no `etc/appconf.yml`
		 *   available, or a value is missing from the config file
		 * 
		 */
		struct CreateInfo
		{
			Window::Properties window = {};
			Graphics::State graphics = {};
		};

	public:

		/********************************************************************
		 *
		 * @function:
		 *	 OnCreate
		 *
		 * @description:
		 *   Callback function that is called after graphics context has been
		 *   created, or if graphics context had to be reset.
		 *
		 * @note:
		 *   This function is not meant to act like a constructor, so
		 *	 only use it to initialize graphics resources such as
		 *   renderers.
		 *
		 */
		virtual void OnCreate() = 0;

	public:
		Application( const Application & ) = delete; /*!< Copying Applications is not allowed. */


		/********************************************************************
		 *
		 * @constructor:
		 *	 Application
		 *
		 * @param 
		 *   _Fallback ::
		 *     Configuration to use when constructing the application if
		 *     no config file is found.
		 *
		 */
		FISSION_API Application( const CreateInfo & _Fallback = {} );


		/********************************************************************
		 *
		 * @destructor:
		 *	 Application
		 *
		 */
		FISSION_API virtual ~Application() noexcept;


		/********************************************************************
		 *
		 * @function:
		 *	 Get
		 * 
		 * @description:
		 *   Get the current instance of the application.
		 *
		 */
		FISSION_API static Application * Get();

	public:

		/********************************************************************
		 *
		 * @function:
		 *   PushLayer
		 * 
		 * @input
		 *   _Name ::
		 *     Name of the layer.
		 * 
		 *   _Ptr_Layer ::
		 *     Layer to be pushed on the layer stack.
		 * 
		 * @note:
		 *   This function WILL be REMOVED and replaced with a scene
		 *   system API.
		 *
		 */
		FISSION_API void PushLayer( const char * _Name, ILayer * _Ptr_Layer );
		

		/********************************************************************
		 *
		 * @function:
		 *   Exit
		 * 
		 * @input:
		 *   _Exit_Code ::
		 *     Exit code returned when the application instance exits.
		 * 
		 */
		FISSION_API void Exit( Platform::ExitCode _Exit_Code = 0 );


		/********************************************************************
		 *
		 * @function:
		 *   GetWindow
		 * 
		 * @return:
		 *   Pointer to the main Window.
		 * 
		 */
		FISSION_API Window * GetWindow();
		

		/********************************************************************
		 *
		 * @function:
		 *   GetGraphics
		 *
		 * @return:
		 *   Pointer to the Graphics instance.
		 *
		 */
		FISSION_API Graphics * GetGraphics();
		

		/********************************************************************
		 *
		 * @function:
		 *   Run
		 * 
		 * @description:
		 *   Main application loop.
		 * 
		 * @return:
		 *   Exit code that should be returned in from main.
		 * 
		 * @note:
		 *   This function is only meant to be called in `_fission_main`
		 * 
		 */
		FISSION_API Platform::ExitCode Run();

		
		/*!< Function not available yet. */
	//	FISSION_API void Recreate();
		
		/*!< Function not available yet. */
	//	FISSION_API basic_timer * GetTimer();
		
		/*!< Function not available yet. */
	//	FISSION_API void SetGraphicsProperties( Graphics::API gAPI );


/* ---------------------------------------- Begin Event Handler Functions ------------------------------------- */

		FISSION_API EventResult OnKeyDown
		( KeyDownEventArgs & )
		override;

		FISSION_API EventResult OnKeyUp
		( KeyUpEventArgs & )
		override;

		FISSION_API EventResult OnTextInput
		( TextInputEventArgs & )
		override;

		FISSION_API EventResult OnMouseMove
		( MouseMoveEventArgs & )
		override;

		FISSION_API EventResult OnMouseLeave
		( MouseLeaveEventArgs & )
		override;

		FISSION_API EventResult OnSetCursor
		( SetCursorEventArgs & )
		override;

		FISSION_API EventResult OnClose
		( CloseEventArgs & )
		override;

		FISSION_API EventResult OnHide
		()
		override;

		FISSION_API EventResult OnShow
		()
		override;

/* ------------------------------------------ End Event Handler Functions ------------------------------------- */

	private:
		struct ApplicationState * m_State; /*!< Pointer to the internal state of the Application */

	}; // class Fission::Application

} // namespace Fission
