#pragma once
#include "Fission/config.h"
#include "Color.h"
#include "Serializable.h"

namespace Fission
{
	using PixelCallback = std::function<colorf(int x, int y)>;

	namespace Texture {
		typedef enum Format_ {
			Format_Unkown,
			Format_RGBA8_UNORM,
			Format_RGBA32_FLOAT,
		} Format;
	}

	class Surface : public ISerializable
	{
	public:
		using vec2 = vec2<uInt32>;

		struct CreateInfo {
			uInt32 Width = 0, Height = 0;
			std::optional<color> FillColor = {};
			Texture::Format Format = Texture::Format_RGBA8_UNORM;
		};

		enum ResizeOptions_ {
			ResizeOptions_Clip,
			ResizeOptions_Stretch,
		};
	public:
		FISSION_API static std::unique_ptr<Surface> Create( const CreateInfo & info = {} );


		virtual void resize( vec2 new_size, ResizeOptions_ options = ResizeOptions_Clip ) = 0;
		virtual void set_width( uInt32 new_width, ResizeOptions_ options = ResizeOptions_Clip ) = 0;
		virtual void set_height( uInt32 new_height, ResizeOptions_ options = ResizeOptions_Clip ) = 0;

		// seems familiar
		virtual void PutPixel( uInt32 x, uInt32 y, color color ) = 0;

		virtual void insert( int x, int y, PixelCallback src, vec2i src_size ) = 0;
	//	void Insert( uint32_t x, uint32_t y, const SurfaceRGBA & src, recti src_region = {} );

		// shrink the surface if there is any 'clear_color' on any side
		virtual void shrink_to_fit( color clear_color = coloru(0,0,0,0) ) = 0;

		virtual Texture::Format format() const = 0;

		virtual const void * data() const = 0;
		virtual void * data() = 0;

		virtual uInt32 width() const = 0;
		virtual uInt32 height() const = 0;

		virtual vec2 size() const = 0;

		virtual uInt32 byte_size() const = 0;
		virtual uInt32 pixel_count() const = 0;

		virtual bool empty() const = 0;

	}; // class Fission::Surface

} // namespace Fission
