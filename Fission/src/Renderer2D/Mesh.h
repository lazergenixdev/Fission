#pragma once
#include "Fission/Core/Graphics/Renderer2D.hh"

namespace Fission {

	struct MeshData
	{
		using index = uint32_t;

		struct vertex {
			base::vector2f pos;
			uint32_t color_index;
		};

		std::vector<vertex> vertex_buffer;
		std::vector<index> index_buffer;
		std::vector<color> color_buffer;

	};

}
