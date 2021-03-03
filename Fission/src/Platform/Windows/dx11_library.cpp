#include "dx11_library.h"
#include "Fission/config.h"

HMODULE dx11_library::hD3D11Library = NULL;
HMODULE dx11_library::hD3DCompilerLibrary = NULL;

bool dx11_library::bLoadSuccess = false;

int dx11_library::nref = 0;

// epic meta-programming:
struct FCreateDeviceAndSwapChain
{
	using Ty = decltype( &D3D11CreateDeviceAndSwapChain );
	static constexpr const char * Name = FISSION_MK_STR( D3D11CreateDeviceAndSwapChain );
};
struct FCreateDevice
{
	using Ty = decltype( &D3D11CreateDevice );
	static constexpr const char * Name = FISSION_MK_STR( D3D11CreateDevice );
};
struct FReadFileToBlob
{
	using Ty = decltype( &D3DReadFileToBlob );
	static constexpr const char * Name = FISSION_MK_STR( D3DReadFileToBlob );
};
struct FCompile
{
	using Ty = decltype( &D3DCompile );
	static constexpr const char * Name = FISSION_MK_STR( D3DCompile );
};
template <typename _Func, typename..._Params> static HRESULT constexpr call(HMODULE lib, _Params&&...params) {
	return ( ( typename _Func::Ty )GetProcAddress( lib, _Func::Name ) )( std::forward<_Params>( params ) ... );
}


bool dx11_library::Initialize()
{
	if( nref > 0 ) return bLoadSuccess;

	hD3D11Library = LoadLibraryW( L"d3d11.dll" );
	if( hD3D11Library == NULL ) return false;

	hD3DCompilerLibrary = LoadLibraryW( L"d3dcompiler_47.dll" ); // ??? only check for 47? ???
	if( hD3DCompilerLibrary == NULL ) // if we loaded d3d11.dll, then this will most likely never happen, but just in case
	{
		FreeLibrary( hD3D11Library );
		return false;
	}

	bLoadSuccess = true, ++nref;
	return true;
}

void dx11_library::Shutdown()
{
	if( nref > 0 || !bLoadSuccess ) { --nref; return; }

	FreeLibrary( hD3D11Library );
	FreeLibrary( hD3DCompilerLibrary );
	bLoadSuccess = false;
	nref = 0;
}

HRESULT dx11_library::CreateDeviceAndSwapChain( 
	IDXGIAdapter * pAdapter, 
	D3D_DRIVER_TYPE DriverType, 
	HMODULE Software, 
	UINT Flags, 
	const D3D_FEATURE_LEVEL * pFeatureLevels, 
	UINT FeatureLevels, 
	UINT SDKVersion, 
	const DXGI_SWAP_CHAIN_DESC * pSwapChainDesc, 
	IDXGISwapChain ** ppSwapChain, 
	ID3D11Device ** ppDevice, 
	D3D_FEATURE_LEVEL * pFeatureLevel, 
	ID3D11DeviceContext ** ppImmediateContext )
{
	if( hD3D11Library == NULL ) throw lazer::exception( "DX11 Library Exception", _lazer_exception_msg.append( "Failed to call `Initialize()`" ) );
	return call<FCreateDeviceAndSwapChain>( hD3D11Library, pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext );
}

HRESULT dx11_library::CreateDevice( 
	IDXGIAdapter * pAdapter, 
	D3D_DRIVER_TYPE DriverType, 
	HMODULE Software, 
	UINT Flags, 
	const D3D_FEATURE_LEVEL * pFeatureLevels, 
	UINT FeatureLevels, 
	UINT SDKVersion, 
	ID3D11Device ** ppDevice, 
	D3D_FEATURE_LEVEL * pFeatureLevel, 
	ID3D11DeviceContext ** ppImmediateContext )
{
	if( hD3D11Library == NULL ) throw lazer::exception( "DX11 Library Exception", _lazer_exception_msg.append( "Failed to call `Initialize()`" ) );
	return call<FCreateDevice>( hD3D11Library, pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext );
}

HRESULT dx11_library::ReadFileToBlob( LPCWSTR pFileName, ID3DBlob ** ppContents )
{
	if( hD3DCompilerLibrary == NULL ) throw lazer::exception( "DX11 Library Exception", _lazer_exception_msg.append( "Failed to call `Initialize()`" ) );
	return call<FReadFileToBlob>( hD3DCompilerLibrary, pFileName, ppContents );
}

HRESULT dx11_library::Compile( 
	LPCVOID pSrcData, 
	SIZE_T SrcDataSize, 
	LPCSTR pSourceName, 
	const D3D_SHADER_MACRO * pDefines, 
	ID3DInclude * pInclude, 
	LPCSTR pEntrypoint, 
	LPCSTR pTarget, 
	UINT Flags1, 
	UINT Flags2, 
	ID3DBlob ** ppCode, 
	ID3DBlob ** ppErrorMsgs )
{
	if( hD3DCompilerLibrary == NULL ) throw lazer::exception( "DX11 Library Exception", _lazer_exception_msg.append( "Failed to call `Initialize()`" ) );
	return call<FCompile>( hD3DCompilerLibrary, pSrcData, SrcDataSize, pSourceName, pDefines, pInclude, pEntrypoint, pTarget, Flags1, Flags2, ppCode, ppErrorMsgs );
}
