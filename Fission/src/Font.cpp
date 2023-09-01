#include <Fission/Core/Font.hh>
#include <Fission/Core/Engine.hh>
#include <freetype/freetype.h>
#include <MaxRectsBinPack.hpp>

void display_fatal_error(const char* title, const char* what);
#define check(X, WHAT) if(X) { display_fatal_error("Font Error", WHAT); return; } (void)0

#define for_(IDX, COUNT) for (decltype(COUNT) IDX = 0; IDX < (COUNT); ++IDX)

extern fs::Engine engine;

using namespace fs;

void Font_Static::create(void const* ttf_data, size_t _size, float _height, VkDescriptorSet set, VkSampler sampler) {
	FT_Error error = FT_Err_Ok;
	FT_Face face = nullptr;

	texture = set;

	error = FT_New_Memory_Face(engine.fonts.library, (FT_Byte const*)ttf_data, (FT_Long)_size, 0, &face);
	check(error, "Failed to create font face [FT_New_Memory_Face]");

	error = FT_Set_Pixel_Sizes(face, 0, (FT_UInt)_height);
	check(error, "Failed to set pixel sizes [FT_Set_Pixel_Sizes]");

	float yMax = float(face->size->metrics.ascender >> 6);
	height = float(face->size->metrics.height >> 6);

	auto _h = u32(face->size->metrics.height >> 6);
	v2u32 size = { _h * 6, _h * 6 };
	auto pixel_data = (rgba8*)calloc(sizeof(rgba8), size.x * size.y);
	check(!pixel_data, "Failed to allocate pixel data");
	
	auto pack = rbp::MaxRectsBinPack(size.x, size.y, false);

	auto generate_glyph = [&]() -> fs::Glyph {
		Glyph g;

		auto offsetx = (float)(face->glyph->bitmap_left);
		auto offsety = (float)(-face->glyph->bitmap_top) + yMax;
		auto sizex = (float)(face->glyph->metrics.width >> 6);
		auto sizey = (float)(face->glyph->metrics.height >> 6);
		g.rc = rf32::from_topleft(offsetx, offsety, sizex, sizey);
		g.advance = (float)(face->glyph->metrics.horiAdvance >> 6);

		auto bitmap = face->glyph->bitmap;

		auto rect = pack.Insert(bitmap.width, bitmap.rows, rbp::MaxRectsBinPack::RectBestAreaFit);

		/* now, copy to our target surface */
		for_(y, bitmap.rows) {
			for_(x, bitmap.width) {
				pixel_data[(rect.y + y) * size.x + (rect.x + x)]
					= rgba8(255, 255, 255, bitmap.buffer[y * bitmap.width + x]);
			}
		}

		g.uv.x = { (float)rect.x, (float)(rect.x + bitmap.width) };
		g.uv.x /= (float)size.x;
		g.uv.y = { (float)rect.y, (float)(rect.y + bitmap.rows) };
		g.uv.y /= (float)size.y;
		return g;
	};

	error = FT_Load_Glyph(face, 0, FT_LOAD_RENDER);
	check(error, "no fallback");

	// Set Fallback glyph
	table.fallback = generate_glyph();

	// Glyph ranges for most english characters, Todo: better font control (languages)
	for (c32 ch = 0x20; ch < 0x7F; ch++)
	{
		if (error = FT_Load_Char(face, ch, FT_LOAD_RENDER))
			continue;

		if (face->glyph->glyph_index == 0) // <-- why?
			continue;

		table.glyphs[ch - 0x20] = generate_glyph();
	}

	{
		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.arrayLayers = 1;
		imageInfo.extent = { .width = size.x, .height = size.y, .depth = 1 };
		imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.mipLevels = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		vmaCreateImage(engine.graphics.allocator, &imageInfo, &allocInfo, &atlas_image, &atlas_allocation, nullptr);
		engine.graphics.upload_image(atlas_image, pixel_data, imageInfo.extent, VK_FORMAT_R8G8B8A8_SRGB);

		free(pixel_data);
	}
	auto viewInfo = vk::image_view_2d(atlas_image, VK_FORMAT_R8G8B8A8_SRGB);
	vkCreateImageView(engine.graphics.device, &viewInfo, nullptr, &atlas_view);

	VkDescriptorImageInfo imageInfo;
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = atlas_view;
	imageInfo.sampler = sampler;
	VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write.descriptorCount = 1;
	write.dstBinding = 0;
	write.dstSet = texture;
	write.pImageInfo = &imageInfo;
	vkUpdateDescriptorSets(engine.graphics.device, 1, &write, 0, nullptr);

	FT_Done_Face(face);
}

Glyph const* Font_Static::lookup(c32 c) {
	if (c < 32 || c > 127) return &table.fallback;
	return &table.glyphs[c - 32];
}

void Font_Static::destroy() {
	vkDestroyImageView(engine.graphics.device, atlas_view, nullptr);
	vmaDestroyImage(engine.graphics.allocator, atlas_image, atlas_allocation);
}
