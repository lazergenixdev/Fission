#pragma once
#include "Input/Event.h"
#include "Graphics/Renderer2D.h"

namespace Fission {

	class Application;

	interface ILayer : public IEventHandler
	{
		virtual void OnCreate() = 0;
		virtual void OnUpdate() = 0;

		virtual ~ILayer() noexcept = default;

	protected:
		
		FISSION_API Application * GetApp();
		
	}; // interface Fission::ILayer


	///////////////////////////////////////////////////////////////////////
	// Default Layers /////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////

	using DebugDrawCallback = std::function<void( Renderer2D * pr2d )>;

	class DebugLayer : public ILayer
	{
	public:
		virtual void RegisterDrawCallback( const char * _Key, DebugDrawCallback _Callback ) = 0;

		FISSION_API static void Push( const char * name );

		FISSION_API static void Pop();

		FISSION_API static void Text( const char * what );

		template <size_t Buffer_Size = 128, typename...T>
		static void Text( const char * fmt, T&&...args )
		{
			char _buffer[Buffer_Size];
			sprintf_s( _buffer, fmt, std::forward<T>( args )... );
			Text( _buffer );
		}
	};

	interface IConsoleLayer : public ILayer
	{
	};

	interface IUILayer : public ILayer
	{
	};
	
} // namespace Fission
