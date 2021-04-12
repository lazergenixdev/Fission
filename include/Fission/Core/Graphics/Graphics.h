/**
*
* @file: Graphics.h
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

// todo: add documentation
// todo: add RenderTarget(framebuffer) API

#pragma once
#include "Fission/Core/Window.h"
#include "Bindable.h"
#include "Renderer.h"

namespace Fission {

class Graphics
{
public:

	using VertexBuffer =		Resource::VertexBuffer;
	using IndexBuffer =			Resource::IndexBuffer;
	using Texture2D =			Resource::Texture2D;
	using Shader =				Resource::Shader;
	using Blender =				Resource::Blender;

	using FrameBuffer =			Resource::FrameBuffer;

public:
	enum class API {
		DirectX11, /*!< DirectX 11 */
		DirectX12, /*!< DirectX 12 */
		Vulkan,    /*!< Vulkan */
		OpenGL,    /*!< OpenGL */

		__count__, /*!< Number of Graphics APIs available */
		Default    /*!< @Graphics will decide which api is best to use */
	};

	struct Properties
	{
		API api = API::Default;
	};

	struct State
	{
		API api;
		Resource::FrameBuffer * pFrameBuffers;
		int nFrameBuffers;
	};
		
public:

	static bool IsSupported( API _GFX_API );

	static scoped<Graphics> Create( const Properties & _Properties = {} );


/* ------------------------------------ Begin Base API Functions ----------------------------------- */

	//! @brief Get the internal API
	virtual API GetAPI() = 0;

	virtual void SetVSync( bool vsync ) = 0;

	virtual bool GetVSync() = 0;

	//virtual void SetProperties( const Properties * props ) = 0;

	virtual void SetFrameBuffer( Resource::FrameBuffer * buffer ) = 0;

	virtual void GetState( State * _Ptr_Dest_State ) {};

	virtual void Draw( uint32_t vertexCount, uint32_t vertexOffset = 0u ) = 0;

	virtual void DrawIndexed( uint32_t indexCount, uint32_t indexOffset = 0u, uint32_t vertexOffset = 0u ) = 0;

/* ------------------------------------ End Base API Functions ----------------------------------- */


/* ------------------------------------ Begin Graphics Primitives ----------------------------------- */

	virtual scoped<FrameBuffer> CreateFrameBuffer( const FrameBuffer::CreateInfo & info ) = 0;

	virtual scoped<VertexBuffer> CreateVertexBuffer( const VertexBuffer::CreateInfo & info ) = 0;

	virtual scoped<IndexBuffer> CreateIndexBuffer( const IndexBuffer::CreateInfo & info ) = 0;

	virtual scoped<Shader> CreateShader( const Shader::CreateInfo & info ) = 0;

	virtual scoped<Texture2D> CreateTexture2D( const Texture2D::CreateInfo & info ) = 0;

	virtual scoped<Blender> CreateBlender( const Blender::CreateInfo & info ) = 0;

/* ------------------------------------ End Graphics Primitives ----------------------------------- */

public:

	using native_handle_type = void *;

	/*!< @brief Only returned by `native_handle()` when `GetAPI() == DirectX11` */
	struct native_type_dx11
	{
		void * pDevice; /*!< d3d11device */
		void * pDeviceContext; /*!< immediate mode context */
	};

	virtual native_handle_type native_handle() = 0;

public:
	virtual ~Graphics() = default;

}; // class Fission::Graphics

} // namespace Fission
