#pragma once
#include "Fission/config.hpp"

namespace Fission
{
	struct WindowsModule
	{
	public:

		virtual bool Load() = 0;

		virtual void UnLoad() = 0;

		virtual ~WindowsModule() noexcept
		{
			Free();
		}

	protected:

		constexpr WindowsModule() noexcept = default;

		void Free() noexcept
		{
			if( hModule )
			{
				FreeLibrary( hModule );
				hModule = NULL;
			}
		}

		bool LoadLibrary( const char * _Library_Name ) noexcept
		{
			return bool( hModule = LoadLibraryA(_Library_Name) );
		}

		template <class _FnPtr>
		inline constexpr bool LoadFunction( _FnPtr * _Out_Ptr_Function_Pointer, const char * _Function_Name ) FISSION_NDEBUG_NOEXCEPT
		{
			return LoadFunction( reinterpret_cast<void**>( _Out_Ptr_Function_Pointer ), _Function_Name );
		}

		bool LoadFunction( void ** _Out_Ptr_Function_Pointer, const char * _Function_Name ) FISSION_NDEBUG_NOEXCEPT
		{
			FISSION_ASSERT( hModule != NULL, "No module is loaded" );
			return bool( *_Out_Ptr_Function_Pointer = GetProcAddress( hModule, _Function_Name ) );
		}

	private:
		HMODULE hModule = NULL;
	};
}
