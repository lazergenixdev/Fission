#include "GraphicsOpenGL.h"
#include "WindowsModule.h"
#include <Fission/Platform/Windows/winapi.h>
#include <Fission/Core/Console.hh>

// @TODO: -> premake.lua
#include "../../../vendor/glew/include/GL/glew.h"
#include "../../../vendor/glew/include/GL/wglew.h"

using ChoosePixelFormat_fn   = decltype(&::ChoosePixelFormat);
using DescribePixelFormat_fn = decltype(&::DescribePixelFormat);
using SetPixelFormat_fn      = decltype(&::SetPixelFormat);

// @TODO: These should be in the premake, NOT HERE.
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"../../../vendor/glew/lib/Release/x64/glew32.lib")

namespace gl
{
	using ShaderBinary = void(*)(GLsizei count, const GLuint *shaders, GLenum binaryFormat, const void *binary,	GLsizei length);
}

static bool CreateGLContext( HDC m_hDC, HGLRC& m_hrc )
{
	{
		auto gdi = Fission::Platform::Module( "gdi32.dll" );

		auto choosePixelFormat = gdi.Get<ChoosePixelFormat_fn>( "ChoosePixelFormat" );
		auto describePixelFormat = gdi.Get<DescribePixelFormat_fn>( "DescribePixelFormat" );
		auto setPixelFormat = gdi.Get<SetPixelFormat_fn>( "SetPixelFormat" );

		PIXELFORMATDESCRIPTOR pfd;
		memset( &pfd, 0, sizeof( PIXELFORMATDESCRIPTOR ) );
		pfd.nSize = sizeof( PIXELFORMATDESCRIPTOR );
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cDepthBits = 32;
		pfd.iLayerType = PFD_MAIN_PLANE;

		int nPixelFormat = choosePixelFormat( m_hDC, &pfd );

		if( nPixelFormat == 0 ) return false;

		BOOL bResult = setPixelFormat( m_hDC, nPixelFormat, &pfd );

		if( !bResult ) return false;
	}

	HGLRC tempContext = wglCreateContext( m_hDC );
	wglMakeCurrent( m_hDC, tempContext );

	GLenum err = glewInit();
	if( GLEW_OK != err )
	{
	//	AfxMessageBox( _T( "GLEW is not initialized!" ) );
	}

	int attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 1,
		WGL_CONTEXT_FLAGS_ARB, 0,
		0
	};

	if( GLEW_VERSION_4_1 )
	{
		m_hrc = wglCreateContextAttribsARB( m_hDC, 0, attribs );
		wglMakeCurrent( NULL, NULL );
		wglDeleteContext( tempContext );
		wglMakeCurrent( m_hDC, m_hrc );
	}
	else
	{	//It's not possible to make a GL 4.x context. Use the old style context (GL 2.1 and before)
		m_hrc = tempContext;
		throw "error";
	}

	//Checking GL version
	auto GLVersionString = (const char*)glGetString( GL_VERSION );

	Fission::Console::WriteLine( Fission::cat("Using OpenGL ", Fission::string{GLVersionString}) / Fission::colors::CadetBlue );

	//Or better yet, use the GL3 way to get the version number
	int OpenGLVersion[2];
	glGetIntegerv( GL_MAJOR_VERSION, &OpenGLVersion[0] );
	glGetIntegerv( GL_MINOR_VERSION, &OpenGLVersion[1] );

	Fission::Console::WriteLine( cat("Vendor: ",Fission::string{ (const char*)glGetString(GL_VENDOR) }) );
	Fission::Console::WriteLine( cat("Renderer: ",Fission::string{ (const char*)glGetString(GL_RENDERER) }) );
	Fission::Console::WriteLine( cat("Using GLSL ",Fission::string{ (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION) }) );

	__debugbreak();

	if( !m_hrc ) return false;

	return true;
}

Fission::GraphicsOpenGL::GraphicsOpenGL(): m_NativeHandle()
{
    // to get WGL functions we need valid GL context, so create dummy window for dummy GL contetx
    HWND dummy = CreateWindowExW(
        0, L"STATIC", L"DummyWindow", WS_OVERLAPPED,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, NULL, NULL );
    //Assert( dummy && "Failed to create dummy window" );

    HDC dc = GetDC( dummy );
	HGLRC rc = NULL;
	CreateGLContext( dc, rc );

	m_NativeHandle.hContext = rc;
}

Fission::IFGraphics::API Fission::GraphicsOpenGL::GetAPI()
{
	return API::OpenGL;
}

void Fission::GraphicsOpenGL::Draw( uint32_t vertexCount, uint32_t vertexOffset )
{
}

void Fission::GraphicsOpenGL::DrawIndexed( uint32_t indexCount, uint32_t indexOffset, uint32_t vertexOffset )
{
}

void Fission::GraphicsOpenGL::SetClipRect( rf32 rect )
{
}

Fission::IFGraphics::FrameBuffer* Fission::GraphicsOpenGL::CreateFrameBuffer( const FrameBuffer::CreateInfo& info )
{
	return nullptr;
}

Fission::IFGraphics::VertexBuffer* Fission::GraphicsOpenGL::CreateVertexBuffer( const VertexBuffer::CreateInfo& info )
{
	return nullptr;
}

Fission::IFGraphics::ConstantBuffer* Fission::GraphicsOpenGL::CreateConstantBuffer( const ConstantBuffer::CreateInfo& info )
{
	return nullptr;
}

Fission::IFGraphics::IndexBuffer* Fission::GraphicsOpenGL::CreateIndexBuffer( const IndexBuffer::CreateInfo& info )
{
	return nullptr;
}

Fission::IFGraphics::Shader* Fission::GraphicsOpenGL::CreateShader( const Shader::CreateInfo& info )
{
	return nullptr;
}

Fission::IFGraphics::Texture2D* Fission::GraphicsOpenGL::CreateTexture2D( const Texture2D::CreateInfo& info )
{
	return nullptr;
}

Fission::IFGraphics::Blender* Fission::GraphicsOpenGL::CreateBlender( const Blender::CreateInfo& info )
{
	return nullptr;
}

Fission::IFGraphics::SwapChain* Fission::GraphicsOpenGL::CreateSwapChain( const SwapChain::CreateInfo& info )
{
	return nullptr;
}

Fission::IFGraphics::native_handle_type Fission::GraphicsOpenGL::native_handle()
{
	return nullptr;
}

void Fission::GraphicsOpenGL::Destroy()
{
}
