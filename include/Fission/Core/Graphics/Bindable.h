#pragma once
#include "Fission/config.h"
#include "Fission/Core/Surface.h"
#include "VertexLayout.h"

namespace Fission::Resource
{
	interface IBindable
	{
		virtual void Bind() = 0;
		virtual void Unbind() = 0;
	};

	class Texture2D : public IBindable
	{
	public:
		enum class Type {
			Static, Dynamic,
		};

		struct CreateInfo {
			Type type = Type::Static;
			const wchar_t * filePath = nullptr;
			Surface * pSurface = nullptr;
		};
	public:
		virtual uint32_t GetWidth() = 0;
		virtual uint32_t GetHeight() = 0;
		virtual void Bind( int slot ) = 0;
	//	virtual Surface GetSurface() = 0;
	};

	class VertexBuffer : public IBindable
	{
	public:
		enum class Type {
			Static, Dynamic,
		};

		struct CreateInfo {
			Type type = Type::Static;
			VertexLayout * pVertexLayout;
			void * pVertexData = nullptr;
			uint32_t vtxCount;
		};
	public:
		virtual void SetData( const void * pVertexData, uint32_t vtxCount ) = 0;

		virtual uint32_t GetCount() = 0;
	};

	class IndexBuffer : public IBindable
	{
	public:
		enum class Type {
			Static, Dynamic,
		};

		enum class Size {
			UInt32, UInt16,
		};

		struct CreateInfo {
			Type type = Type::Static;
			Size size = Size::UInt32;
			void * pIndexData = nullptr;
			uint32_t idxCount;
		};
	public:
		virtual void SetData( const void * pIndexData, uint32_t idxCount ) = 0;

		virtual uint32_t GetCount() = 0;
	};

	class Shader : public IBindable
	{
	public:

		struct CreateInfo {
			std::string source_code;
			std::wstring name;
			VertexLayout * pVertexLayout;
		};

	//public:
	//	void SetVariable( const char * name, float val ) {}
	//	void SetVariable( const char * name, vec2f val ) {}
	//	void SetVariable( const char * name, vec3f val ) {}
	//	void SetVariable( const char * name, vec4f val ) {}
	//	void SetVariable( const char * name, colorf val ) {}
	//	void SetVariable( const char * name, int val ) {}
	};

	// todo: remove this and integrate into Shader
	class ConstantBuffer : public IBindable
	{
	public:
		enum class Type {
			Vertex, Pixel,
		};

		struct CreateInfo {
			Type type = Type::Vertex;
			uint32_t ByteSize;
			uint32_t slot = 0;
		};
	public:
		virtual void Update( void * pData ) = 0;
	};

	//class Topology : public Bindable
	//{
	//public:
	//	enum Type {
	//		LineList, LineStrip, 
	//		TriangleList, TriangleStrip, 
	//	};

	//	virtual Type Get() = 0;
	//	virtual void Set( Type ) = 0;
	//};

	class Sampler : public IBindable
	{
	public:
		enum {
			Point, Linear
		};
	};

	class Blender : public IBindable
	{
	public:
		enum class Blend {
			Normal, 
			Multiply, 
			Add, 
			Subtract, 
			Divide, 
			Source,
		};
		struct CreateInfo {
			Blend blend = Blend::Normal;
		};
	};
}

