/**
*
* @file: Bindable.h
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
#include <Fission/Base/String.h>
#include <Fission/Base/Math/Matrix.h>
#include <Fission/Core/Object.hh>
#include <Fission/Core/Surface.hh>
#include <Fission/Core/Monitor.hh>
#include <Fission/Core/Graphics/VertexLayout.hh>

namespace Fission 
{ 
	struct IFWindow; 

	enum vsync_ {
		vsync_Off = 0,
		vsync_OneFrame = 1,
		vsync_TwoFrame = 2,

		vsync_On = vsync_OneFrame,
	};
}

namespace Fission::Resource
{
	struct IFBindable : public IFObject
	{
		//! @brief Bind this current Object to the Graphics Pipeline.
		virtual void Bind() = 0;

		//! @brief Sets object in pipeline to it's default.
		virtual void Unbind() = 0;
	};

	//! @brief Texture Object.
	struct IFTexture2D : public IFBindable
	{
		enum class Type {
			Static, Dynamic,
		};

		struct CreateInfo {
			Type type = Type::Static;
			const wchar_t * filePath = nullptr;
			Surface * pSurface = nullptr;
			Texture::Format format = Texture::Format_RGBA8_UNORM;
			base::size size = {};
		};

		virtual uint32_t GetWidth() = 0;
		virtual uint32_t GetHeight() = 0;
		virtual void Bind( int slot ) = 0;
		
		//! @brief Put texture onto system memory for CPU access.
		//! @note Function only succeeds for Textures created with `Dynamic` Type.
		virtual Surface * GetSurface() { return nullptr; }
	};

	struct IFFrameBuffer : public IFBindable
	{

		struct CreateInfo {
			base::size size = {};
			Texture::Format format = Texture::Format_RGBA8_UNORM;
		};

		using Properties = CreateInfo; // optional alias

		virtual base::size GetSize() = 0;

		virtual void Clear( color clear_color ) = 0;

		virtual IFTexture2D * GetTexture2D() = 0;
	};

	//! @note SwapChains are created internally when a new window is created;
	//!			SwapChains may not exist independent of a window.
	struct IFSwapChain : public IFBindable
	{
		struct CreateInfo {
			IFWindow * pWindow;
			base::size size = {};
			Texture::Format format = Texture::Format_RGBA8_UNORM;
		};

		virtual base::size GetSize() = 0;

		virtual void Resize( base::size ) = 0;

	//	virtual FrameBuffer * GetBackBuffer() = 0;

		virtual void SetFullscreen( bool fullscreen, Monitor * pMonitor ) = 0;

		virtual void Clear( color clear_color ) = 0;

		virtual void Present( vsync_ vsync ) = 0;
	};

	struct IFVertexBuffer : public IFBindable
	{
		enum class Type {
			Static, Dynamic,
		};

		struct CreateInfo {
			Type type = Type::Static;
			VertexLayout * pVertexLayout;
			void * pVertexData = nullptr;
			uint32_t vtxCount;
		};

		virtual void SetData( const void * pVertexData, uint32_t vtxCount ) = 0;

		virtual uint32_t GetCount() = 0;

	};

	struct IFIndexBuffer : public IFBindable
	{
		enum class Type {
			Static, Dynamic,
		};

		enum class Size {
			UInt32, UInt16,
		};

		struct CreateInfo {
			Type type = Type::Static;
			Size size = Size::UInt32;
			void * pIndexData = nullptr;
			uint32_t idxCount;
		};

		virtual void SetData( const void * pIndexData, uint32_t idxCount ) = 0;

		virtual uint32_t GetCount() = 0;

	};

	struct IFConstantBuffer : public IFBindable
	{
		enum class Target {
			Vertex, Pixel,
		};

		enum class Type {
			Static, Dynamic,
		};

		struct CreateInfo {
			Type type = Type::Dynamic;
			void * pData = nullptr;
			uint32_t max_size;
		};

		virtual void SetData( const void * pData, uint32_t size ) = 0;

		virtual void Bind( Target target, int slot ) = 0;
	};

	struct IFShader : public IFBindable
	{
		struct CreateInfo {
			string sourceCode;
			VertexLayout * pVertexLayout;
		};

	};

	struct IFTopology : public IFBindable
	{
		enum Type {
			LineList,     LineStrip, 
			TriangleList, TriangleStrip, 
		};

		struct CreateInfo {
			Type type = Type::TriangleList;
		};

		virtual Type Get() = 0;
		virtual void Set( Type ) = 0;
	};

	//!
	struct IFSampler : public IFBindable
	{
		enum Filter {
			Point, Linear
		};

		struct CreateInfo {
			Filter type = Filter::Linear;
		};
	};

	struct IFBlender : public IFBindable
	{
		enum Blend {
			Normal, 
			Multiply, 
			Add, 
			Subtract, 
			Divide, 
			Source,
		};

		struct CreateInfo {
			Blend blend = Blend::Normal;
		};

	};
}

