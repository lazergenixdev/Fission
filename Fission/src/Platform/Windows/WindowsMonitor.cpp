#include "WindowsMonitor.h"
#include "../Monitor.h"
#include <Fission/Core/Console.hh>
#include <Fission/Base/Exception.h>

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

WindowsMonitor::WindowsMonitor( HMONITOR hMonitor, const string & Name, int index )
	: m_hMonitor(hMonitor), m_Name(Name), m_Index(index)
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

const char * WindowsMonitor::GetName() const
{
    return m_Name.c_str();
}

int WindowsMonitor::GetIndex() const
{
    return m_Index;
}

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

	EnumDisplaySettingsW( m_DeviceName.c_str(), 0, &devmode );
	devmode.dmPelsWidth = pMode->resolution.w;
	devmode.dmPelsHeight = pMode->resolution.h;
	devmode.dmDisplayFrequency = pMode->refresh_rate;
	devmode.dmBitsPerPel = 32;
	devmode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;

	return ( ChangeDisplaySettingsExW( m_DeviceName.c_str(), &devmode, NULL, CDS_FULLSCREEN, 0 ) == DISP_CHANGE_SUCCESSFUL );
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

} // namespace Fission::Platform

struct MonitorEnumData
{
	DISPLAYCONFIG_PATH_INFO * pathArray;

	UINT32 index;
	UINT32 pathArrayCount;
};

BOOL CALLBACK Fission::Platform::WindowsMonitor::MonitorEnumCallback( HMONITOR hMonitor, HDC, LPRECT, LPARAM pMonitorEnumData )
{
	auto pEnumData = reinterpret_cast<MonitorEnumData *>( pMonitorEnumData );

	// Number of monitors changed after our query, this is a big problem.
	if( pEnumData->index >= pEnumData->pathArrayCount )
		return FALSE;

	DISPLAYCONFIG_TARGET_DEVICE_NAME request;
	request.header.adapterId = pEnumData->pathArray[pEnumData->index].targetInfo.adapterId;
	request.header.id = pEnumData->pathArray[pEnumData->index].targetInfo.id;
	request.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
	request.header.size = sizeof( request );

	LONG result;
	if( (result = DisplayConfigGetDeviceInfo( (DISPLAYCONFIG_DEVICE_INFO_HEADER *)&request )) != ERROR_SUCCESS )
	{
		char error_msg[64];
		sprintf( error_msg, "`DisplayConfigGetDeviceInfo` returned (%X)", result );
		FISSION_THROW( "Monitor Enum Error", .append( error_msg ) );
	}

	auto name_utf16 = utf16_string((char16_t*)request.monitorFriendlyDeviceName);

	s_Monitors.emplace_back( new WindowsMonitor( hMonitor, name_utf16.utf8(), pEnumData->index++ ) );

	// Continue to enumerate more monitors.
	return TRUE;
}

void Fission::Platform::EnumMonitors()
{
	for( auto && mon : WindowsMonitor::s_Monitors ) delete mon;
	WindowsMonitor::s_Monitors.clear();

	UINT32 numPathArrayElements;
	UINT32 numModeInfoArrayElements;
	DISPLAYCONFIG_PATH_INFO * pathArray = NULL;
	DISPLAYCONFIG_MODE_INFO * modeInfoArray = NULL;
	DISPLAYCONFIG_TOPOLOGY_ID currentTopologyId;

	UINT32 count = 3;
	LONG error = 0;

	do
	{
		numPathArrayElements = count;
		numModeInfoArrayElements = count * 2u;
		pathArray = (DISPLAYCONFIG_PATH_INFO *)_aligned_realloc( pathArray, numPathArrayElements * sizeof DISPLAYCONFIG_PATH_INFO, 64u );
		modeInfoArray = (DISPLAYCONFIG_MODE_INFO *)_aligned_realloc( modeInfoArray, numModeInfoArrayElements * sizeof DISPLAYCONFIG_MODE_INFO, 64u );

		if( pathArray == NULL || modeInfoArray == NULL )
			throw std::bad_alloc();

		error = QueryDisplayConfig( QDC_DATABASE_CURRENT, &numPathArrayElements, pathArray, &numModeInfoArrayElements, modeInfoArray, &currentTopologyId );

	} while( error && (count+=3) < 25 );

	if( error == ERROR_INSUFFICIENT_BUFFER )
	{
		// TODO: Fix this function.
	//	Console::Error( "How do you have more than 24 monitors?? You are INSANE." );
		throw std::logic_error( "rip" );
	}

	MonitorEnumData enumData;
	enumData.index = 0;
	enumData.pathArray = pathArray;
	enumData.pathArrayCount = numPathArrayElements;

	// Init list of monitors
	EnumDisplayMonitors( NULL, nullptr, WindowsMonitor::MonitorEnumCallback, (LPARAM)&enumData );

	// Free memory that we are done with.
	_aligned_free( pathArray );
	_aligned_free( modeInfoArray );
}
