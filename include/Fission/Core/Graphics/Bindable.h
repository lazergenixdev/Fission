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

// todo: shader API gonna need an overhaul in preparation for Vulkan
// todo: dynamic textures (textures that can be accessed by CPU)

#pragma once
#include "Fission/config.h"
#include "Fission/Core/Surface.h"
#include "VertexLayout.h"

namespace Fission { class Window; }

namespace Fission::Resource
{
	struct IBindable
	{
		virtual void Bind() = 0;
		virtual void Unbind() = 0;
	};

	class Texture2D : public IBindable
	{
	public:
		enum class Type {
			Static, Dynamic,
		};

		struct CreateInfo {
			Type type = Type::Static;
			const wchar_t * filePath = nullptr;
			Surface * pSurface = nullptr;
			Texture::Format format = Texture::Format_RGBA8_UNORM;
			int width = 0, height = 0;
		};
	public:
		virtual uint32_t GetWidth() = 0;
		virtual uint32_t GetHeight() = 0;
		virtual void Bind( int slot ) = 0;
	//	virtual Surface GetSurface() = 0;
	};

	class FrameBuffer
	{
	public:
		struct CreateInfo {
			vec2i size = {};
			Window * pWindow = nullptr;
			Texture::Format format = Texture::Format_RGBA8_UNORM;
		};
	public:
		virtual uint32_t GetWidth() = 0;
		virtual uint32_t GetHeight() = 0;

		virtual void Clear( color clear_color ) = 0;

		virtual void Present() = 0;
	//	virtual Surface GetSurface() = 0;
	};

	class VertexBuffer : public IBindable
	{
	public:
		enum class Type {
			Static, Dynamic,
		};

		struct CreateInfo {
			Type type = Type::Static;
			VertexLayout * pVertexLayout;
			void * pVertexData = nullptr;
			uint32_t vtxCount;
		};
	public:
		virtual void SetData( const void * pVertexData, uint32_t vtxCount ) = 0;

		virtual uint32_t GetCount() = 0;
	};

	class IndexBuffer : public IBindable
	{
	public:
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
	public:
		virtual void SetData( const void * pIndexData, uint32_t idxCount ) = 0;

		virtual uint32_t GetCount() = 0;
	};

	class Shader : public IBindable
	{
	public:

		struct CreateInfo {
			std::string source_code;
			std::wstring name;
			VertexLayout * pVertexLayout;
		};

		// temp
		using mat3x3f = struct {
			float data[3*3];
		};
		using mat4x4f = struct {
			float data[4*4];
		};

	public:
		virtual bool SetVariable( const char * name, float val ) = 0;
		virtual bool SetVariable( const char * name, vec2f val ) = 0;
		virtual bool SetVariable( const char * name, vec3f val ) = 0;
		virtual bool SetVariable( const char * name, vec4f val ) = 0;

		virtual bool SetVariable( const char * name, int val ) = 0;
		virtual bool SetVariable( const char * name, vec2i val ) = 0;
		virtual bool SetVariable( const char * name, vec3i val ) = 0;
		virtual bool SetVariable( const char * name, vec4i val ) = 0;

		virtual bool SetVariable( const char * name, mat2x2f val ) = 0;
		virtual bool SetVariable( const char * name, mat3x2f val ) = 0;
		virtual bool SetVariable( const char * name, mat3x3f val ) = 0;
		virtual bool SetVariable( const char * name, mat4x4f val ) = 0;

	//	virtual bool SetVariable( const char * name, float * pBegin, uint32_t count ) = 0;
	//	virtual bool SetVariable( const char * name, int * pBegin, uint32_t count ) = 0;

		inline bool SetVariable( const char * name, colorf val ) { return SetVariable( name, *(vec4f *)&val ); };
	};

	//class Topology : public Bindable
	//{
	//public:
	//	enum Type {
	//		LineList, LineStrip, 
	//		TriangleList, TriangleStrip, 
	//	};

	//	virtual Type Get() = 0;
	//	virtual void Set( Type ) = 0;
	//};

	class Sampler : public IBindable
	{
	public:
		enum {
			Point, Linear
		};
	};

	class Blender : public IBindable
	{
	public:
		enum class Blend {
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

