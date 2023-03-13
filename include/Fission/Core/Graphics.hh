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

#pragma once
#include <Fission/Core/Graphics/Bindable.hh>
#include <Fission/Core/Graphics/Renderer.hh>

namespace Fission
{

struct IFGraphics : public IFObject
{
public:

	using VertexBuffer   = Resource::IFVertexBuffer;
	using IndexBuffer    = Resource::IFIndexBuffer;
	using ConstantBuffer = Resource::IFConstantBuffer;
	using Sampler        = Resource::IFSampler;
	using Texture2D      = Resource::IFTexture2D;
	using Shader         = Resource::IFShader;
	using Blender        = Resource::IFBlender;
	using FrameBuffer    = Resource::IFFrameBuffer;
	using SwapChain      = Resource::IFSwapChain;

public:
	enum class API {
		DirectX11, /*!< DirectX 11 */
		DirectX12, /*!< DirectX 12 */
		Vulkan,    /*!< Vulkan */
		OpenGL,	   /*!< OpenGL 4 */

		__count__, /*!< Number of Graphics APIs available */
		Default,   /*!< @Graphics will decide which api is best to use */
		Noop       /*!< No Graphics API is used, used for debugging only. */
	};

public:

	//! @brief Get the internal API
	virtual API GetAPI() = 0;

	//! @brief sets which window we are currently drawing to.
	virtual void SetContext( struct IFWindow * pWindow ) {};

	virtual void Draw( uint32_t vertexCount, uint32_t vertexOffset = 0u ) = 0;

	virtual void DrawIndexed( uint32_t indexCount, uint32_t indexOffset = 0u, uint32_t vertexOffset = 0u ) = 0;

	virtual void SetClipRect( rf32 rect ) = 0;


	virtual FrameBuffer*    CreateFrameBuffer   ( const FrameBuffer::CreateInfo & info ) = 0;
	virtual VertexBuffer*   CreateVertexBuffer  ( const VertexBuffer::CreateInfo & info ) = 0;
	virtual ConstantBuffer* CreateConstantBuffer( const ConstantBuffer::CreateInfo & info ) = 0;
	virtual Sampler*        CreateSampler       ( const Sampler::CreateInfo & info ) = 0;
	virtual IndexBuffer*    CreateIndexBuffer   ( const IndexBuffer::CreateInfo & info ) = 0;
	virtual Shader*         CreateShader        ( const Shader::CreateInfo & info ) = 0;
	virtual Texture2D*      CreateTexture2D     ( const Texture2D::CreateInfo & info ) = 0;
	virtual Blender*        CreateBlender       ( const Blender::CreateInfo & info ) = 0;
	virtual SwapChain*      CreateSwapChain     ( const SwapChain::CreateInfo & info ) = 0;

public:

	using native_handle_type = void *;

	/*!< @brief Only returned by `native_handle()` when `GetAPI() == DirectX11` */
	struct native_type_dx11
	{
		void * pDevice;			/*!< d3d11device */
		void * pDeviceContext;	/*!< d3d11devicecontext */
	};

	/*!< @brief Only returned by `native_handle()` when `GetAPI() == OpenGL` */
	struct native_type_opengl
	{
		//! @brief: HGLRC for windows
		void * hContext;
	};

	virtual native_handle_type native_handle() = 0;

}; // struct Fission::IFGraphics


struct GraphicsState
{
	IFGraphics::API api       = IFGraphics::API::Default;
	vsync_          vsync     = vsync_On;
	int	            framerate = 0; /*!< framerate only applies when vsync is off, 0 == unlimited framerate */
	int             msaa      = 1;

}; // class Fission::GraphicsState

} // namespace Fission
