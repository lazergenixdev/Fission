//--------------------------------------------------------------------------------------
// File: DXErr.h
//
// DirectX Error Library
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#if !defined(NOMINMAX)
#define NOMINMAX
#endif

#include <windows.h>
#include <sal.h>

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------------------------
// DXGetErrorString
//--------------------------------------------------------------------------------------
	static const WCHAR * WINAPI DXGetErrorStringW( _In_ HRESULT hr );
	static const CHAR * WINAPI DXGetErrorStringA( _In_ HRESULT hr );
#ifdef UNICODE
#define DXGetErrorString DXGetErrorStringW
#else
#define DXGetErrorString DXGetErrorStringA
#endif
//--------------------------------------------------------------------------------------
// DXGetErrorDescription has to be modified to return a copy in a buffer rather than
// the original static string.
//--------------------------------------------------------------------------------------
	static void WINAPI DXGetErrorDescriptionW( _In_ HRESULT hr, _Out_cap_( count ) WCHAR * desc, _In_ size_t count );
	static void WINAPI DXGetErrorDescriptionA( _In_ HRESULT hr, _Out_cap_( count ) CHAR * desc, _In_ size_t count );
#ifdef UNICODE
#define DXGetErrorDescription DXGetErrorDescriptionW
#else
#define DXGetErrorDescription DXGetErrorDescriptionA
#endif

#ifdef __cplusplus
}
#endif //__cplusplus



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//--------------------------------------------------------------------------------------
// File: DXErr.cpp
//
// DirectX Error Library
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
//#include "dxerr.h"

#include <stdio.h>
#include <algorithm>

#if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
#include <ddraw.h>
#include <d3d9.h>
//#include <audiodefs.h>
#include <mmeapi.h>
#include <dsound.h>

#define DIRECTINPUT_VERSION 0x800
#include <dinput.h>
#include <dinputd.h>
#endif

//struct DXGI_RGBA;

#include <d3d10_1.h>
#include <d3d11_1.h>

#if !defined(WINAPI_FAMILY) || WINAPI_FAMILY != WINAPI_FAMILY_PHONE_APP
#include <wincodec.h>
#include <d2derr.h>
#include <dwrite.h>
#endif

#define XAUDIO2_E_INVALID_CALL          0x88960001
#define XAUDIO2_E_XMA_DECODER_ERROR     0x88960002
#define XAUDIO2_E_XAPO_CREATION_FAILED  0x88960003
#define XAUDIO2_E_DEVICE_INVALIDATED    0x88960004

#define XAPO_E_FORMAT_UNSUPPORTED MAKE_HRESULT(SEVERITY_ERROR, 0x897, 0x01)

#define DXUTERR_NODIRECT3D              MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0901)
#define DXUTERR_NOCOMPATIBLEDEVICES     MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0902)
#define DXUTERR_MEDIANOTFOUND           MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0903)
#define DXUTERR_NONZEROREFCOUNT         MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0904)
#define DXUTERR_CREATINGDEVICE          MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0905)
#define DXUTERR_RESETTINGDEVICE         MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0906)
#define DXUTERR_CREATINGDEVICEOBJECTS   MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0907)
#define DXUTERR_RESETTINGDEVICEOBJECTS  MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0908)
#define DXUTERR_INCORRECTVERSION        MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x0909)
#define DXUTERR_DEVICEREMOVED           MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x090A)

//-----------------------------------------------------------------------------
#define BUFFER_SIZE 3000

#pragma warning( disable : 6001 6221 )

//--------------------------------------------------------------------------------------
#define  CHK_ERR_W(hrchk, strOut) \
        case hrchk: \
             return L##strOut;

#define  CHK_ERRA_W(hrchk) \
        case hrchk: \
             return L#hrchk;

#define  CHK_ERR_A(hrchk, strOut) \
        case hrchk: \
             return strOut;

#define  CHK_ERRA_A(hrchk) \
        case hrchk: \
             return #hrchk;

#define HRESULT_FROM_WIN32b(x) ((HRESULT)(x) <= 0 ? ((HRESULT)(x)) : ((HRESULT) (((x) & 0x0000FFFF) | (FACILITY_WIN32 << 16) | 0x80000000)))

#define  CHK_ERR_WIN32A_W(hrchk) \
        case HRESULT_FROM_WIN32b(hrchk): \
        case hrchk: \
             return L#hrchk;

#define  CHK_ERR_WIN32_ONLY_W(hrchk, strOut) \
        case HRESULT_FROM_WIN32b(hrchk): \
             return L##strOut;

#define  CHK_ERR_WIN32A_A(hrchk) \
        case HRESULT_FROM_WIN32b(hrchk): \
        case hrchk: \
             return #hrchk;

#define  CHK_ERR_WIN32_ONLY_A(hrchk, strOut) \
        case HRESULT_FROM_WIN32b(hrchk): \
             return strOut;

//-----------------------------------------------------
const WCHAR * WINAPI DXGetErrorStringW( _In_ HRESULT hr )
{
#define CHK_ERRA CHK_ERRA_W
#define CHK_ERR CHK_ERR_W
#define CHK_ERR_WIN32A CHK_ERR_WIN32A_W
#define CHK_ERR_WIN32_ONLY CHK_ERR_WIN32_ONLY_W
#define DX_STR_WRAP(...) L##__VA_ARGS__
#include "DXGetErrorString.inl"
#undef DX_STR_WRAP
#undef CHK_ERR_WIN32A
#undef CHK_ERR_WIN32_ONLY
#undef CHK_ERRA
#undef CHK_ERR
}

const CHAR * WINAPI DXGetErrorStringA( _In_ HRESULT hr )
{
#define CHK_ERRA CHK_ERRA_A
#define CHK_ERR CHK_ERR_A
#define CHK_ERR_WIN32A CHK_ERR_WIN32A_A
#define CHK_ERR_WIN32_ONLY CHK_ERR_WIN32_ONLY_A
#define DX_STR_WRAP(s) s
#include "DXGetErrorString.inl"
#undef DX_STR_WRAP
#undef CHK_ERR_WIN32A
#undef CHK_ERR_WIN32_ONLY
#undef CHK_ERRA
#undef CHK_ERR
}

//--------------------------------------------------------------------------------------
#undef CHK_ERR
#undef CHK_ERRA
#undef HRESULT_FROM_WIN32b
#undef CHK_ERR_WIN32A
#undef CHK_ERR_WIN32_ONLY

#undef CHK_ERRA_W
#define  CHK_ERRA_W(hrchk) \
        case hrchk: \
             wcscpy_s( desc, count, L#hrchk );

#undef CHK_ERR_W
#define  CHK_ERR_W(hrchk, strOut) \
        case hrchk: \
             wcscpy_s( desc, count, L##strOut );

#undef CHK_ERRA_A
#define  CHK_ERRA_A(hrchk) \
        case hrchk: \
             strcpy_s( desc, count, #hrchk );

#undef CHK_ERR_A
#define  CHK_ERR_A(hrchk, strOut) \
        case hrchk: \
             strcpy_s( desc, count, strOut );

//--------------------------------------------------------------------------------------
void WINAPI DXGetErrorDescriptionW( _In_ HRESULT hr, _Out_cap_( count ) WCHAR * desc, _In_ size_t count )
{
#define CHK_ERRA CHK_ERRA_W
#define CHK_ERR CHK_ERR_W
#define DX_FORMATMESSAGE FormatMessageW
#include "DXGetErrorDescription.inl"
#undef DX_FORMATMESSAGE
#undef CHK_ERRA
#undef CHK_ERR
}

void WINAPI DXGetErrorDescriptionA( _In_ HRESULT hr, _Out_cap_( count ) CHAR * desc, _In_ size_t count )
{
#define CHK_ERRA CHK_ERRA_A
#define CHK_ERR CHK_ERR_A
#define DX_FORMATMESSAGE FormatMessageA
#include "DXGetErrorDescription.inl"
#undef DX_FORMATMESSAGE
#undef CHK_ERRA
#undef CHK_ERR
}
