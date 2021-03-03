#pragma once
#include "Fission/Core/Window.h"
#include "Bindable.h"
#include "Renderer.h"

namespace Fission {

class Graphics
{
public:

	using VertexBuffer =	Resource::VertexBuffer;
	using IndexBuffer =		Resource::IndexBuffer;
	using ConstantBuffer =	Resource::ConstantBuffer;
	using Texture2D =		Resource::Texture2D;
	using Shader =			Resource::Shader;
	using Blender =			Resource::Blender;

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

	static std::unique_ptr<Graphics> Create( Window * _Ptr_Window, const Properties & _Properties );


/* ------------------------------------ Begin Base API Functions ----------------------------------- */

	virtual API GetAPI() = 0;

	virtual void BeginFrame() = 0;

	virtual void EndFrame() = 0;

	virtual void SetVSync( bool vsync ) = 0;

	virtual bool GetVSync() = 0;

	virtual void SetResolution( vec2i ) { ( void )0; }; // Not Implemented

	virtual vec2i GetResolution() = 0;

	// \returns conversion from window space to screen space
	virtual vec2f to_screen( vec2i mouse_pos ) = 0; // TODO: Keep?? unsure

	virtual void Draw( uint32_t vertexCount, uint32_t vertexOffset = 0u ) = 0;

	virtual void DrawIndexed( uint32_t indexCount, uint32_t indexOffset = 0u, uint32_t vertexOffset = 0u ) = 0;

/* ------------------------------------ End Base API Functions ----------------------------------- */


/* ------------------------------------ Begin Graphics Primitives ----------------------------------- */

	virtual std::unique_ptr<VertexBuffer>	CreateVertexBuffer	( const VertexBuffer::CreateInfo & info		) = 0;

	virtual std::unique_ptr<IndexBuffer>	CreateIndexBuffer	( const IndexBuffer::CreateInfo & info		) = 0;

	virtual std::unique_ptr<Shader>			CreateShader		( const Shader::CreateInfo & info			) = 0;

	virtual std::unique_ptr<ConstantBuffer>	CreateConstantBuffer( const ConstantBuffer::CreateInfo & info	) = 0;

	virtual std::unique_ptr<Texture2D>		CreateTexture2D		( const Texture2D::CreateInfo & info		) = 0;

	virtual std::unique_ptr<Blender>		CreateBlender		( const Blender::CreateInfo & info			) = 0;

/* ------------------------------------ End Graphics Primitives ----------------------------------- */

public:
	virtual ~Graphics() = default;

}; // class Fission::Graphics

} // namespace Fission