#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>

// todo: load in the necessary function pointers on `Initialize()`

class dx11_library
{
public:
	// \returns `true` on success
	static bool Initialize();

	static void Shutdown();

	// I hate how these function inputs are sooo long :(

	static HRESULT CreateDeviceAndSwapChain(
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
		ID3D11DeviceContext ** ppImmediateContext
	);

	static HRESULT CreateDevice(
		IDXGIAdapter * pAdapter,
		D3D_DRIVER_TYPE DriverType,
		HMODULE Software,
		UINT Flags,
		const D3D_FEATURE_LEVEL * pFeatureLevels,
		UINT FeatureLevels,
		UINT SDKVersion,
		ID3D11Device ** ppDevice,
		D3D_FEATURE_LEVEL * pFeatureLevel,
		ID3D11DeviceContext ** ppImmediateContext
	);

	static HRESULT ReadFileToBlob( 
		LPCWSTR pFileName,
		ID3DBlob ** ppContents
	);

	static HRESULT Compile( 
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
		ID3DBlob ** ppErrorMsgs
	);

private:

	static HMODULE hD3D11Library;
	static HMODULE hD3DCompilerLibrary;

	static bool bLoadSuccess;

	static int nref;
};

