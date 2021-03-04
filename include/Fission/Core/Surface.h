#pragma once
#include "Fission/config.h"
#include "Color.h"
#include "Serializable.h"

namespace Fission
{
	using PixelCallback = std::function<colorf( uint32_t x, uint32_t y)>;

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
		struct CreateInfo {
			uint32_t Width = 0, Height = 0;
			std::optional<color> FillColor = {};
			Texture::Format Format = Texture::Format_RGBA8_UNORM;
		};

		enum ResizeOptions_ {
			ResizeOptions_Clip,
			ResizeOptions_Stretch,
		};
	public:
		FISSION_API static std::unique_ptr<Surface> Create( const CreateInfo & info = {} );


		virtual void resize( vec2u new_size, ResizeOptions_ options = ResizeOptions_Clip ) = 0;
		virtual void set_width( uint32_t new_width, ResizeOptions_ options = ResizeOptions_Clip ) = 0;
		virtual void set_height( uint32_t new_height, ResizeOptions_ options = ResizeOptions_Clip ) = 0;

		// seems familiar
		virtual void PutPixel( uint32_t x, uint32_t y, color color ) = 0;
		virtual color GetPixel( uint32_t x, uint32_t y ) const = 0;

		virtual void insert( uint32_t x, uint32_t y, PixelCallback src, vec2u src_size ) = 0;
		virtual void insert( uint32_t x, uint32_t y, const Surface * src, std::optional<recti> src_region = {} ) = 0;

		// shrink the surface if there is any 'clear_color' on any side
		virtual void shrink_to_fit( color clear_color = coloru(0,0,0,0) ) = 0;

		virtual Texture::Format format() const = 0;

		virtual const void * data() const = 0;
		virtual void * data() = 0;

		virtual uint32_t width() const = 0;
		virtual uint32_t height() const = 0;

		virtual vec2u size() const = 0;

		virtual uint32_t byte_size() const = 0;
		virtual uint32_t pixel_count() const = 0;

		virtual bool empty() const = 0;

	}; // class Fission::Surface

} // namespace Fission
