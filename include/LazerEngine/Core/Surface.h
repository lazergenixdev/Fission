#pragma once
#include "LazerEngine/config.h"
#include "Color.h"
#include "Serializable.h"

namespace lazer
{
	using PixelCallback = std::function<colorf(int x, int y)>;

	namespace Format {
		enum SurfaceFormat_ : uint16 {
			SurfaceFormat_Unkown,
			SurfaceFormat_RGBA8_UNORM,
			SurfaceFormat_RGBA32_FLOAT,
		};
	}

	class Surface : public ISerializable
	{
	public:
		using vec2 = vec2<uint32>;

		struct CreateInfo {
			uint32 Width = 0, Height = 0;
			std::optional<color> FillColor = {};
			Format::SurfaceFormat_ Format = Format::SurfaceFormat_RGBA8_UNORM;
		};

		enum ResizeOptions_ {
			ResizeOptions_Clip,
			ResizeOptions_Stretch,
		};
	public:
		LAZER_API static std::unique_ptr<Surface> Create( const CreateInfo & info = {} );


		virtual void resize( vec2 new_size, ResizeOptions_ options = ResizeOptions_Clip ) = 0;
		virtual void set_width( uint32 new_width, ResizeOptions_ options = ResizeOptions_Clip ) = 0;
		virtual void set_height( uint32 new_height, ResizeOptions_ options = ResizeOptions_Clip ) = 0;

		virtual void PutPixel( uint32 x, uint32 y, color color ) = 0;

		virtual void insert( int x, int y, PixelCallback src, vec2i src_size ) = 0;
	//	void Insert( uint32_t x, uint32_t y, const SurfaceRGBA & src, recti src_region = {} );

		// shrink the surface if there is any 'clear_color' on any side
		virtual void shrink_to_fit( color clear_color = coloru(0,0,0,0) ) = 0;

		virtual Format::SurfaceFormat_ format() const = 0;

		virtual const void * data() const = 0;
		virtual void * data() = 0;

		virtual uint32 width() const = 0;
		virtual uint32 height() const = 0;

		virtual vec2 size() const = 0;

		virtual uint32 byte_size() const = 0;
		virtual uint32 pixel_count() const = 0;

		virtual bool empty() const = 0;

	}; // interface lazer::ISurface

} // namespace lazer
