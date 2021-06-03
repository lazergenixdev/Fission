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
#include "Fission/config.h"
#include "Fission/Base/String.h"
#include "Fission/Base/Math/Matrix.h"
#include "Fission/Core/Surface.h"
#include "Fission/Core/Monitor.h"
#include "VertexLayout.h"

namespace Fission 
{ 
	class Window; 

	enum vsync_ {
		vsync_Off = 0,
		vsync_OneFrame = 1,
		vsync_TwoFrame = 2,

		vsync_On = vsync_OneFrame,
	};
}

namespace Fission::Resource
{
	Fission_Interface IBindable
	{
		virtual void Bind() = 0;
		virtual void Unbind() = 0;
	};

	Fission_Interface Texture2D : public IBindable
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

	Fission_Interface FrameBuffer : public IBindable
	{

		struct CreateInfo {
			base::size size = {};
			Texture::Format format = Texture::Format_RGBA8_UNORM;
		};

		using Properties = CreateInfo; // optional alias

		virtual base::size GetSize() = 0;

		virtual void Clear( color clear_color ) = 0;

		virtual Texture2D * GetTexture2D() = 0;
	};

	//! @note SwapChains are created internally when a new window is created;
	//!			SwapChains may not exist independent of a window.
	Fission_Interface SwapChain : public IBindable
	{
		struct CreateInfo {
			Window * pWindow;
			base::size size = {};
			Texture::Format format = Texture::Format_RGBA8_UNORM;
		};

		virtual base::size GetSize() = 0;

	//	virtual FrameBuffer * GetBackBuffer() = 0;

		virtual void SetFullscreen( bool fullscreen, Monitor * pMonitor ) = 0;

		virtual void Clear( color clear_color ) = 0;

		virtual void Present( vsync_ vsync ) = 0;
	};

	Fission_Interface VertexBuffer : public IBindable
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

	Fission_Interface IndexBuffer : public IBindable
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

	Fission_Interface ConstantBuffer : public IBindable
	{
		enum class Type {
			Static, Dynamic,
		};

		struct CreateInfo {
			Type type = Type::Dynamic;
			void * pData = nullptr;
		};

		virtual void SetData( const void * pIndexData ) = 0;
	};

	Fission_Interface Shader : public IBindable
	{
		struct CreateInfo {
			string sourceCode;
			VertexLayout * pVertexLayout;
		};
	};

	Fission_Interface Topology : public IBindable
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
	Fission_Interface Sampler : public IBindable
	{
		enum Filter {
			Point, Linear
		};

		struct CreateInfo {
			Filter type = Filter::Linear;
		};
	};

	Fission_Interface Blender : public IBindable
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

