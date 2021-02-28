#pragma once
#include "LazerEngine/Core/Graphics/Renderer2D.h"

namespace lazer {

	struct MeshData
	{
		using index = uint32_t;

		struct vertex {
			lazer::vec2f pos;
			uint32_t color_index;
		};

		std::vector<vertex> vertex_buffer;
		std::vector<index> index_buffer;
		std::vector<color> color_buffer;

	};

}
