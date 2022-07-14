/**
*
* @file: Surface.h
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
#include <Fission/config.hpp>
#include <Fission/Core/Serializable.hh>
#include <Fission/Core/Object.hh>
#include <Fission/Base/util/SmartPointer.hpp>
#include <Fission/Base/Color.hpp>
#include <Fission/Base/Rect.hpp>
#include <functional>

// todo: add documentation

namespace Fission
{
	using PixelCallback = std::function<color( int x, int y )>;

	namespace Texture {
		typedef enum Format_ {
			Format_Unkown,
			Format_RGB8_UNORM,
			Format_RGBA8_UNORM,
			Format_RGB32_FLOAT,
			Format_RGBA32_FLOAT,
		} Format;
	}

	struct Surface : public ISerializable
	{
	public:
		struct CreateInfo {
			size2 size;
			std::optional<color> fillColor = {};
			Texture::Format format = Texture::Format_RGBA8_UNORM;
		};

		enum ResizeOptions_ {
			ResizeOptions_Clip,
			ResizeOptions_Stretch,
		};
	public:
		FISSION_API static ptr<Surface> Create( const CreateInfo & _Create_Info = {} );


		virtual void resize( size2 _New_Size, ResizeOptions_ _Options = ResizeOptions_Clip ) = 0;
		virtual void set_width( int _New_Width, ResizeOptions_ _Options = ResizeOptions_Clip ) = 0;
		virtual void set_height( int _New_Height, ResizeOptions_ _Options = ResizeOptions_Clip ) = 0;

		// seems familiar
		virtual void PutPixel( int _X, int _Y, color _Color ) = 0;
		virtual color GetPixel( int _X, int _Y ) const = 0;

		virtual void insert( int _X, int _Y, PixelCallback _Source, size2 _Source_Size ) = 0;
		virtual void insert( int _X, int _Y, const Surface * _Source, std::optional<ri32> _Source_Rect = {} ) = 0;

		// shrink the surface if there is any 'clear_color' on any side
		virtual void shrink_to_fit( color _Clear_Color = {} ) = 0;

		virtual Texture::Format format() const = 0;

		virtual const void * data() const = 0;
		virtual void * data() = 0;

		virtual int width() const = 0;
		virtual int height() const = 0;

		virtual size2 size() const = 0;

		virtual u64 byte_size() const = 0;
		virtual u64 pixel_count() const = 0;

		virtual bool empty() const = 0;

		virtual ~Surface() noexcept = default;

	}; // class Fission::Surface

} // namespace Fission
