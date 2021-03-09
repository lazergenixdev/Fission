#pragma once
#include "Fission/Core/Window.h"
#include "Bindable.h"
#include "Renderer.h"

namespace Fission {

class Graphics
{
public:

	using VertexBuffer =		Resource::VertexBuffer;
	using IndexBuffer =			Resource::IndexBuffer;
	using Texture2D =			Resource::Texture2D;
	using Shader =				Resource::Shader;
	using Blender =				Resource::Blender;

public:
	enum class API {
		DirectX11, /*!< DirectX 11 */
		DirectX12, /*!< DirectX 12 */
		Vulkan,    /*!< Vulkan */
		OpenGL,    /*!< OpenGL */

		__count__, /*!< Number of Graphics APIs available */
		Default /*!< @Graphics will decide which api is best to use */
	};

	struct Properties
	{
		API api = API::Default;
		vec2i resolution = { 1280, 720 };
	};
		
public:

	static bool IsSupported( API _GFX_API );

	static scoped<Graphics> Create( Window * _Ptr_Window, const Properties & _Properties );


/* ------------------------------------ Begin Base API Functions ----------------------------------- */

	//! @brief Get the internal
	virtual API GetAPI() = 0;

	//! @todo: rename these
	virtual void BeginFrame() = 0;
	virtual void EndFrame() = 0;

	virtual void SetVSync( bool vsync ) = 0;

	virtual bool GetVSync() = 0;

	//! @note THESE WILL BE REPLACED WITH A RENDER TARGET API
	virtual void SetResolution( vec2i ) { ( void )0; }; // Not Implemented
	virtual vec2i GetResolution() = 0;

	//! @brief conversion from window space to screen space
	virtual vec2f to_screen( vec2i mouse_pos ) = 0; // TODO: Keep?? unsure

	virtual void Draw( uint32_t vertexCount, uint32_t vertexOffset = 0u ) = 0;

	virtual void DrawIndexed( uint32_t indexCount, uint32_t indexOffset = 0u, uint32_t vertexOffset = 0u ) = 0;

/* ------------------------------------ End Base API Functions ----------------------------------- */


/* ------------------------------------ Begin Graphics Primitives ----------------------------------- */

	virtual scoped<VertexBuffer> CreateVertexBuffer( const VertexBuffer::CreateInfo & info ) = 0;

	virtual scoped<IndexBuffer> CreateIndexBuffer( const IndexBuffer::CreateInfo & info ) = 0;

	virtual scoped<Shader> CreateShader( const Shader::CreateInfo & info ) = 0;

	virtual scoped<Texture2D> CreateTexture2D( const Texture2D::CreateInfo & info ) = 0;

	virtual scoped<Blender> CreateBlender( const Blender::CreateInfo & info ) = 0;

/* ------------------------------------ End Graphics Primitives ----------------------------------- */

public:

	using native_handle_type = void *;

	/*!< @brief Only returned by `native_handle()` when `GetAPI() == DirectX11` */
	struct native_type_dx11
	{
		void * pDevice; /*!< d3d11device */
		void * pDeviceContext; /*!< immediate mode context */
	};

	virtual native_handle_type native_handle() = 0;

public:
	virtual ~Graphics() = default;

}; // class Fission::Graphics

} // namespace Fission