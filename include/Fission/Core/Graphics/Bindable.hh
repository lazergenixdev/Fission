/**
 *	______________              _____
 *	___  ____/__(_)________________(_)____________
 *	__  /_   __  /__  ___/_  ___/_  /_  __ \_  __ \
 *	_  __/   _  / _(__  )_(__  )_  / / /_/ /  / / /
 *	/_/      /_/  /____/ /____/ /_/  \____//_/ /_/
 *
 *
 * @Author:       lazergenixdev@gmail.com
 * @Development:  (https://github.com/lazergenixdev/Fission)
 * @License:      MIT (see end of file)
 */
#pragma once
#include <Fission/Base/String.hpp>
#include <Fission/Base/Math/Matrix.hpp>
#include <Fission/Core/Object.hh>
#include <Fission/Core/Surface.hh>
#include <Fission/Core/Monitor.hh>
#include <Fission/Core/Graphics/VertexLayout.hh>

namespace Fission 
{ 
	struct Window; 

	enum vsync_ {
		vsync_Off = 0,
		vsync_OneFrame = 1,
		vsync_TwoFrame = 2,

		vsync_On = vsync_OneFrame,
	};
}

namespace Fission::gfx
{
	struct Bindable : public ManagedObject
	{
		//! @brief Bind this current Object to the Graphics Pipeline.
		virtual void Bind() = 0;

		//! @brief Sets object in pipeline to it's default.
		virtual void Unbind() = 0;
	};

	//! @brief Texture Object.
	struct Texture2D : public Bindable
	{
		enum class Type {
			Static, Dynamic,
		};

		struct CreateInfo {
			Type type = Type::Static;
			const wchar_t * filePath = nullptr;
			Surface * pSurface = nullptr;
			Texture::Format format = Texture::Format_RGBA8_UNORM;
			size2 size = {};
		};

		virtual uint32_t GetWidth() = 0;
		virtual uint32_t GetHeight() = 0;
		virtual void Bind( int slot ) = 0;
		
		//! @brief Put texture onto system memory for CPU access.
		//! @note Function only succeeds for Textures created with `Dynamic` Type.
		virtual Surface * GetSurface() { return nullptr; }
	};

	struct FrameBuffer : public Bindable
	{

		struct CreateInfo {
			size2 size = {};
			Texture::Format format = Texture::Format_RGBA8_UNORM;
		};

		using Properties = CreateInfo; // optional alias

		virtual size2 GetSize() = 0;

		virtual void Clear( color clear_color ) = 0;

		virtual Texture2D * GetTexture2D() = 0;
	};

	//! @note SwapChains are created internally when a new window is created;
	//!			SwapChains may not exist independent of a window.
	struct SwapChain : public Bindable
	{
		struct CreateInfo {
			Window * pWindow;
			size2 size = {};
			Texture::Format format = Texture::Format_RGBA8_UNORM;
		};

		virtual size2 GetSize() = 0;

		virtual void Resize( size2 ) = 0;

	//	virtual FrameBuffer * GetBackBuffer() = 0;

		virtual void SetFullscreen( bool fullscreen, Monitor * pMonitor ) = 0;

		virtual void Clear( color clear_color ) = 0;

		virtual void Present( vsync_ vsync ) = 0;
	};

	struct VertexBuffer : public Bindable
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

	struct IndexBuffer : public Bindable
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

	struct ConstantBuffer : public Bindable
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

	struct Shader : public Bindable
	{
		struct CreateInfo {
			string_view sourceCode;
			VertexLayout * pVertexLayout;
		};

	};

	struct Topology : public Bindable
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
	struct Sampler : public Bindable
	{
		enum Filter {
			Point, Linear
		};

		enum Target {
			Vertex, Pixel
		};

		struct CreateInfo {
			Filter filter = Filter::Linear;
			Target target = Target::Pixel;
		};

		virtual void Bind( Target target, int slot ) = 0;
	};

	struct Blender : public Bindable
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

/**
 *	MIT License
 *
 *	Copyright (c) 2021-2023 lazergenixdev
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this software and associated documentation files (the "Software"), to deal
 *	in the Software without restriction, including without limitation the rights
 *	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *	copies of the Software, and to permit persons to whom the Software is
 *	furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in all
 *	copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 */