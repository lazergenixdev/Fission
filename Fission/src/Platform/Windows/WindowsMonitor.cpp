#include "WindowsMonitor.h"
#include "../Monitor.h"

namespace Fission
{
	list_view<MonitorPtr> Monitor::GetMonitors()
	{
		using m = Platform::WindowsMonitor;
		return list_view<MonitorPtr>( (MonitorPtr *)( &m::s_Monitors.front() ), (MonitorPtr *)( &m::s_Monitors.back() + 1 ) );
	}
}

namespace Fission::Platform
{

std::vector<WindowsMonitor *> WindowsMonitor::s_Monitors;

inline bool DisplayModesEqual( const DisplayMode & a, const DisplayMode & b )
{
	return ( a.resolution == b.resolution ) && ( a.refresh_rate == b.refresh_rate );
}

WindowsMonitor::WindowsMonitor( HMONITOR hMonitor, const std::wstring & Name )
	: m_hMonitor(hMonitor), m_Name(Name)
{
	BOOL bRetVal;
	DEVMODEW devmode;
	memset( &devmode, 0, sizeof( DEVMODEW ) );
	devmode.dmSize = sizeof( DEVMODEW );

	MONITORINFOEXW mi;
	mi.cbSize = sizeof( mi );
	GetMonitorInfoW( hMonitor, &mi );
	m_DeviceName = mi.szDevice;

	DisplayMode current;
	::EnumDisplaySettingsW( mi.szDevice, ENUM_CURRENT_SETTINGS, &devmode );
	current.refresh_rate = (int)devmode.dmDisplayFrequency;
	current.resolution = { (int)devmode.dmPelsWidth, (int)devmode.dmPelsHeight };

	int currentIdx = -1;
	int iMode = 0, i = 0;
	bRetVal = ::EnumDisplaySettingsW( mi.szDevice, iMode, &devmode );

	for( ; bRetVal ; bRetVal = ::EnumDisplaySettingsW( mi.szDevice, iMode++, &devmode ) )
	{
		if( devmode.dmBitsPerPel != 32 ) continue;
		if( devmode.dmDisplayFrequency < 60 ) continue;

		DisplayMode mode;
		mode.refresh_rate = (int)devmode.dmDisplayFrequency;
		mode.resolution = { (int)devmode.dmPelsWidth, (int)devmode.dmPelsHeight };

		if( !m_SupportedModes.empty() && DisplayModesEqual( mode, m_SupportedModes.back() ) ) continue;

		m_SupportedModes.emplace_back( mode ); ++i;

		if( currentIdx == -1 && DisplayModesEqual( current, mode ) ) currentIdx = i-1;
	}

	m_pCurrentMode = &m_SupportedModes[currentIdx];
}

const wchar_t * WindowsMonitor::GetName() const
{
    return m_Name.c_str();
}

//int WindowsMonitor::GetIndex() const
//{
//    return 0;
//}

const DisplayMode * WindowsMonitor::GetCurrentDisplayMode() const
{
    return m_pCurrentMode;
}

list_view<DisplayMode> WindowsMonitor::GetSupportedDisplayModes()
{
    return list_view<DisplayMode>( (DisplayMode *)( &m_SupportedModes.front() ), (DisplayMode *)( &m_SupportedModes.back() + 1 ) );
}

bool WindowsMonitor::SetDisplayMode( const DisplayMode * pMode )
{
	DEVMODEW devmode;
	memset( &devmode, 0, sizeof( DEVMODEW ) );
	devmode.dmSize = sizeof( DEVMODEW );

	EnumDisplaySettingsW( NULL, 0, &devmode );
	devmode.dmPelsWidth = pMode->resolution.w;
	devmode.dmPelsHeight = pMode->resolution.h;
	devmode.dmDisplayFrequency = pMode->refresh_rate;
	devmode.dmBitsPerPel = 32;
	devmode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;

	return ( ChangeDisplaySettingsW( &devmode, CDS_FULLSCREEN ) == DISP_CHANGE_SUCCESSFUL );
}

bool WindowsMonitor::RevertDisplayMode()
{
	bool changeSuccessful = ( ChangeDisplaySettingsW( nullptr, CDS_RESET ) == DISP_CHANGE_SUCCESSFUL );

	DisplayMode current;
	{
		DEVMODEW devmode;
		memset( &devmode, 0, sizeof( DEVMODEW ) );
		devmode.dmSize = sizeof( DEVMODEW );
		EnumDisplaySettingsW( m_DeviceName.c_str(), ENUM_CURRENT_SETTINGS, &devmode );
		current.refresh_rate = (int)devmode.dmDisplayFrequency;
		current.resolution = { (int)devmode.dmPelsWidth, (int)devmode.dmPelsHeight };
	}
	int currentIdx = -1;

	int iMode = 0;
	for( auto && mode : m_SupportedModes )
	{
		if( DisplayModesEqual( current, mode ) )
		{
			currentIdx = iMode;
			break;
		}
		++iMode;
	}

	if( currentIdx == -1 )
		throw std::logic_error( "Something went very wrong when finding current monitor settings, RIP" );

	m_pCurrentMode = &m_SupportedModes[currentIdx];

	return changeSuccessful;
}

WindowsMonitor::native_handle_type WindowsMonitor::native_handle()
{
    return m_hMonitor;
}

WindowsMonitor * WindowsMonitor::GetMonitorFromHandle( HMONITOR hMonitor )
{
	return nullptr;
}


BOOL CALLBACK WindowsMonitor::MonitorEnumCallback( HMONITOR hMonitor, HDC, LPRECT, LPARAM ppwsNames )
{
	auto & pName = *reinterpret_cast<std::wstring **>( ppwsNames );
	s_Monitors.emplace_back( new WindowsMonitor( hMonitor, *(pName++) ) );
	return TRUE;
}

}

#include <tchar.h>
#include <initguid.h>
#include <wmistr.h>

DEFINE_GUID( WmiMonitorID_GUID, 0x671a8285, 0x4edb, 0x4cae, 0x99, 0xfe, 0x69, 0xa1, 0x5c, 0x48, 0xc0, 0xbc );

typedef struct WmiMonitorID {
	USHORT ProductCodeID[16];
	USHORT SerialNumberID[16];
	USHORT ManufacturerName[16];
	UCHAR WeekOfManufacture;
	USHORT YearOfManufacture;
	USHORT UserFriendlyNameLength;
	USHORT UserFriendlyName[1];
} WmiMonitorID, * PWmiMonitorID;

#define OFFSET_TO_PTR(Base, Offset) ((PBYTE)((PBYTE)Base + Offset))

typedef HRESULT( WINAPI * WOB ) ( IN LPGUID lpGUID, IN DWORD nAccess, OUT PVOID );
WOB WmiOpenBlock;
typedef HRESULT( WINAPI * WQAD ) ( IN LONG_PTR hWMIHandle, ULONG * nBufferSize, OUT UCHAR * pBuffer );
WQAD WmiQueryAllData;
typedef HRESULT( WINAPI * WCB ) ( IN LONG_PTR );
WCB WmiCloseBlock;

void Fission::Platform::EnumMonitors()
{
	WindowsMonitor::s_Monitors.clear();

	std::vector<std::wstring> monitor_names;

	// Get all the monitor names
	HRESULT hr = E_FAIL;
	LONG_PTR hWmiHandle = 0;
	PWmiMonitorID MonitorID = nullptr;

	HINSTANCE hDLL = LoadLibraryA( "Advapi32.dll" );

	if( hDLL == NULL ) throw 0x45;

	WmiOpenBlock = (WOB)GetProcAddress( hDLL, "WmiOpenBlock" );
	WmiQueryAllData = (WQAD)GetProcAddress( hDLL, "WmiQueryAllDataW" );
	WmiCloseBlock = (WCB)GetProcAddress( hDLL, "WmiCloseBlock" );

	if( WmiOpenBlock != NULL && WmiQueryAllData && WmiCloseBlock )
	{
		WCHAR pszDeviceId[256] = L"";
		hr = WmiOpenBlock( (LPGUID)&WmiMonitorID_GUID, GENERIC_READ, &hWmiHandle );
		if( hr == ERROR_SUCCESS )
		{
			ULONG nBufferSize = 0;
			UCHAR * pAllDataBuffer = 0;
			UCHAR * ptr = 0;
			PWNODE_ALL_DATA pWmiAllData;
			hr = WmiQueryAllData( hWmiHandle, &nBufferSize, 0 );
			if( hr == ERROR_INSUFFICIENT_BUFFER )
			{
				pAllDataBuffer = (UCHAR *)malloc( nBufferSize );
				ptr = pAllDataBuffer;
				hr = WmiQueryAllData( hWmiHandle, &nBufferSize, pAllDataBuffer );
				if( hr == ERROR_SUCCESS )
				{
					while( 1 )
					{
						pWmiAllData = (PWNODE_ALL_DATA)pAllDataBuffer;
						if( pWmiAllData->WnodeHeader.Flags & WNODE_FLAG_FIXED_INSTANCE_SIZE )
							MonitorID = (PWmiMonitorID)&pAllDataBuffer[pWmiAllData->DataBlockOffset];
						else
							MonitorID = (PWmiMonitorID)&pAllDataBuffer[pWmiAllData->OffsetInstanceDataAndLength[0].OffsetInstanceData];

						ULONG nOffset = 0;
						WCHAR * pwsInstanceName = 0;
						nOffset = (ULONG)pAllDataBuffer[pWmiAllData->OffsetInstanceNameOffsets];
						pwsInstanceName = (WCHAR *)OFFSET_TO_PTR( pWmiAllData, nOffset + sizeof( USHORT ) );

						WCHAR * pwsUserFriendlyName = (WCHAR *)MonitorID->UserFriendlyName;
						monitor_names.emplace_back( pwsUserFriendlyName );

						if( !pWmiAllData->WnodeHeader.Linkage )
							break;
						pAllDataBuffer += pWmiAllData->WnodeHeader.Linkage;
					}
				}
				free( ptr );
			}
			WmiCloseBlock( hWmiHandle );
		}
	}

	FreeLibrary( hDLL );

	// Init list of monitors
	std::wstring * pStartName = &monitor_names.front();
	EnumDisplayMonitors( NULL, nullptr, WindowsMonitor::MonitorEnumCallback, (LPARAM)&pStartName );
}
