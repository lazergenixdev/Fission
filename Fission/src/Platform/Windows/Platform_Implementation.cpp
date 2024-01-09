#include <Fission/Base/String.hpp>
#include <Fission/Platform.hpp>
#include <Fission/Core/Display.hh>
#include <Fission/Core/Engine.hh>
#include <Fission/Core/Console.hh>
#include <Fission/Platform/utils.h>
#include <algorithm>
#include <shellapi.h>
#include <commdlg.h>
#include "../../Scene_Key_Parser.hpp"

#pragma comment (lib, "Shell32.lib")
#pragma comment (lib, "Comdlg32.lib")

fs::string platform_version;
extern fs::Engine engine;

namespace fs::platform {
	struct Module {
		inline Module(const char* module_name) { _handle = LoadLibraryExA(module_name, NULL, LOAD_LIBRARY_SEARCH_SYSTEM32); }
		constexpr bool is_null() const noexcept { return _handle == NULL; }
		template <typename F>
		F Get(const char* name) const noexcept { return reinterpret_cast<F>(GetProcAddress(_handle, name)); }
		~Module() { FreeLibrary(_handle); }
	private:
		HMODULE _handle;
	};

	struct Version_Data {
		Version_Data() {
			typedef LONG NTSTATUS;
			typedef NTSTATUS(WINAPI* pfn_RtlGetVersion)(PRTL_OSVERSIONINFOW);

			pfn_RtlGetVersion RtlGetVersion;

			RTL_OSVERSIONINFOW version = { 0 };
			version.dwOSVersionInfoSize = sizeof(version);

			// Since `GetVersion()` depends on your application manifest,
			// we need to import another function to get the real OS version
			auto ntdll = platform::Module("ntdll.dll");
			platform_version.data = (c8*)_Version;

			if (ntdll.is_null())
				goto error;

			// Get function address
			RtlGetVersion = ntdll.Get<pfn_RtlGetVersion>("RtlGetVersion");

			if (RtlGetVersion == nullptr)
				goto error;

			// Retrieve the true windows version for this machine.
			if (0 != RtlGetVersion(&version))
				goto error;

			// Windows, please get your shit together! Why do I have to do your work??
			if (version.dwBuildNumber > 22000) version.dwMajorVersion = 11;

#ifdef FISSION_PLATFORM_WINDOWS32
			BOOL IsRunningOn64 = FALSE;
			if (FALSE == IsWow64Process(GetCurrentProcess(), &IsRunningOn64)) throw FISSION_THROW("Win32 Error", "IsWow64Process() returned false");
			platform_version.count = sprintf_s(_Version, "Windows %i %i-Bit (%u.%u.%u)", version.dwMajorVersion, IsRunningOn64 ? 64 : 32, version.dwMajorVersion, version.dwMinorVersion, version.dwBuildNumber);
#else // FISSION_PLATFORM_WINDOWS64
			platform_version.count = sprintf_s(_Version, "Windows %i 64-Bit (%u.%u.%u)", version.dwMajorVersion, version.dwMajorVersion, version.dwMinorVersion, version.dwBuildNumber);
#endif
			return;

		error:
			platform_version.count = sprintf_s(_Version, "Windows (Unknown Version)");
		}

		char _Version[64];
	};
}

fs::platform::Version_Data _platform_version_data{};

/////////////////////////////////////////////////////////////////////////////////////////
// Timestamps
/////////////////////////////////////////////////////////////////////////////////////////

__FISSION_BEGIN__

struct PerformanceFrequency {
	PerformanceFrequency() {
		QueryPerformanceFrequency(&raw);
		value = (double)raw.QuadPart;
	}
	double value;
	LARGE_INTEGER raw;
} _freq;

s64 timestamp() {
	LARGE_INTEGER o;
	QueryPerformanceCounter(&o);
	return o.QuadPart;
}

double seconds_elasped(s64 start, s64 end) {
	return double(end - start) / _freq.value;
}

double seconds_elasped_and_reset(s64& last) {
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	double elapsed = double(now.QuadPart - last) / _freq.value;
	last = now.QuadPart;
	return elapsed;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Command Line Stuff
/////////////////////////////////////////////////////////////////////////////////////////

void skip_application_path(string& s) {
	u64 idx = 0;
	while (idx < s.count && s.data[idx] != ' ') ++idx;
	while (idx < s.count && s.data[idx] == ' ') ++idx;
	s = s.substr(idx);
}

Scene_Key cmdline_to_scene_key(platform::Instance) {
	Scene_Key key;
	string_utf16 win32_command_line;
	win32_command_line.data = (c16*)GetCommandLineW();
	win32_command_line.count = strlen(win32_command_line.data);

	// could probably avoid allocation by using stack
	auto temp_buffer = std::make_unique<c8[]>(win32_command_line.count * 3);

	string command_line;
	command_line.data = temp_buffer.get();
	convert_utf16_to_utf8(&command_line, win32_command_line);

	console::println(command_line);
	skip_application_path(command_line);

	Scene_Key_Parser parser{command_line};
	parser.parse(key.stream);
	return key;
}


/////////////////////////////////////////////////////////////////////////////////////////
// Monitors
/////////////////////////////////////////////////////////////////////////////////////////

struct MonitorEnumData
{
	DISPLAYCONFIG_PATH_INFO* pathArray;
	UINT32 index;
	UINT32 pathArrayCount;
	std::vector<Display>* out;
};

BOOL CALLBACK MonitorEnumCallback(HMONITOR hMonitor, HDC, LPRECT, LPARAM pMonitorEnumData)
{
	auto pEnumData = reinterpret_cast<MonitorEnumData*>(pMonitorEnumData);

	// Number of monitors changed after our query, this is a big problem.
	if (pEnumData->index >= pEnumData->pathArrayCount)
		return FALSE;

	DISPLAYCONFIG_TARGET_DEVICE_NAME request;
	request.header.adapterId = pEnumData->pathArray[pEnumData->index].targetInfo.adapterId;
	request.header.id = pEnumData->pathArray[pEnumData->index].targetInfo.id;
	request.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
	request.header.size = sizeof(request);

	LONG result;
	if ((result = DisplayConfigGetDeviceInfo((DISPLAYCONFIG_DEVICE_INFO_HEADER*)&request)) != ERROR_SUCCESS)
	{
		return FALSE;
		//	char error_msg[64];
		//	sprintf(error_msg, "`DisplayConfigGetDeviceInfo` returned (%X)", result);
		//	FISSION_THROW("Monitor Enum Error", .append(error_msg));
	}

	Display display;
	display._handle = hMonitor;

	FS_FOR(64) {
		display.name_buffer[i] = (c8)request.monitorFriendlyDeviceName[i];
	}
	display.name_count = (int)strlen((char*)display.name_buffer);

	MONITORINFOEX mi;
	mi.cbSize = sizeof(mi);
	GetMonitorInfoW(hMonitor, &mi);
	display.rect = rs32::from_win32(mi.rcMonitor);

	if ((mi.dwFlags & MONITORINFOF_PRIMARY) && pEnumData->index != 0) {
		display.index = 0;
		if (pEnumData->out->size())
			(*pEnumData->out)[0].index = pEnumData->index;
	}
	else display.index = pEnumData->index;

	pEnumData->out->emplace_back(display);
	pEnumData->index++;

	// Continue to enumerate more monitors.
	return TRUE;
}

void enumerate_displays(std::vector<struct Display>& out) {
	out.clear();

	UINT32 numPathArrayElements;
	UINT32 numModeInfoArrayElements;
	DISPLAYCONFIG_PATH_INFO* pathArray = NULL;
	DISPLAYCONFIG_MODE_INFO* modeInfoArray = NULL;
	DISPLAYCONFIG_TOPOLOGY_ID currentTopologyId;

	UINT32 count = 3;
	LONG error = 0;

	do {
		numPathArrayElements = count;
		numModeInfoArrayElements = count * 2u;
		pathArray = (DISPLAYCONFIG_PATH_INFO*)_aligned_realloc(pathArray, numPathArrayElements * sizeof DISPLAYCONFIG_PATH_INFO, 64u);
		modeInfoArray = (DISPLAYCONFIG_MODE_INFO*)_aligned_realloc(modeInfoArray, numModeInfoArrayElements * sizeof DISPLAYCONFIG_MODE_INFO, 64u);

		if (pathArray == NULL || modeInfoArray == NULL)
			throw std::bad_alloc();

		error = QueryDisplayConfig(QDC_DATABASE_CURRENT, &numPathArrayElements, pathArray, &numModeInfoArrayElements, modeInfoArray, &currentTopologyId);

	} while (error && (count += 3) < 25); // what kind of while loop is this?

	if (error == ERROR_INSUFFICIENT_BUFFER)
	{
		// TODO: do error handling here
	//	Console::Error( "How do you have more than 24 monitors?? You are INSANE." );
		throw std::logic_error("rip");
	}

	MonitorEnumData enumData;
	enumData.index = 0;
	enumData.pathArray = pathArray;
	enumData.pathArrayCount = numPathArrayElements;
	enumData.out = &out;

	// Init list of monitors
	EnumDisplayMonitors(NULL, nullptr, MonitorEnumCallback, (LPARAM)&enumData);

	// stupid
	std::sort(out.begin(), out.end(), [](Display const& a, Display const& b) { return a.index < b.index; });

	// Free memory that we are done with.
	_aligned_free(pathArray);
	_aligned_free(modeInfoArray);
}


/////////////////////////////////////////////////////////////////////////////////////////
// Utils
/////////////////////////////////////////////////////////////////////////////////////////

struct Auto_Closing_File {
	FILE* handle = NULL;

	FILE** operator&() { return &handle; }

	~Auto_Closing_File() {
		if (handle != NULL) fclose(handle);
	}
};

void* platform::load_entire_file(platform::path const& filepath, u64* out_file_size) {
	Auto_Closing_File file;
	errno_t err = _wfopen_s(&file, filepath.c_str(), L"rb");

	if (err != 0 || file.handle == NULL) {
		FS_debug_printf("error: could not open file \"%s\"\n", filepath.string().c_str());
		return nullptr;
	}

	_fseeki64(file.handle, 0, SEEK_END);
	u64 file_size = (u64)_ftelli64(file.handle);
	_fseeki64(file.handle, 0, SEEK_SET);

	void* file_data = malloc(file_size);
	if (file_data == nullptr) {
		FS_debug_printf("error: malloc failed [size = %llu]\n", file_size);
		return nullptr;
	}

	auto bytes_read = fread(file_data, 1, file_size, file.handle);

	if (bytes_read != file_size) {
		FS_debug_printf("error: failed to read file \"%s\"", filepath.string().c_str());
		return file_data; // TODO: is this ok? I guess this is very rare so.. eh, whatever.
	}

	*out_file_size = file_size;
	return file_data;
}

FISSION_API bool platform::dump_to_file(path const& filepath, void* data, u64 size)
{
	Auto_Closing_File file;
	errno_t err = _wfopen_s(&file, filepath.c_str(), L"wb");

	if (err != 0 || file.handle == NULL) {
		FS_debug_printf("error: could not open file \"%s\"\n", filepath.string().c_str());
		return true;
	}

	u64 bytes_written = fwrite(data, 1, size, file.handle);

	if (bytes_written != size) {
		return true;
	}

	return false;
}

void win32_str_copy(WCHAR* dst, int& offset, char const* src) {
	while (*src != 0) dst[offset++] = *src++;
}

platform::path platform::open_file_dialog(char const* _Name, char const* _Extensions)
{
	WCHAR opened_path[MAX_PATH] = {};

	OPENFILENAMEW of = {};
	of.lStructSize = sizeof(of);
	of.hwndOwner = NULL;
	of.lpstrFile = opened_path;
	of.nMaxFile = sizeof(opened_path);

	WCHAR filter[128] = L"All (*.*)\0*.*\0";
	int count = 14;
	win32_str_copy(filter, count, _Name);
	filter[count++] = L' ';
	filter[count++] = L'(';
	win32_str_copy(filter, count, _Extensions);
	filter[count++] = L')';
	filter[count++] = L'\0';
	win32_str_copy(filter, count, _Extensions);
	filter[count++] = L'\0';
	filter[count++] = L'\0';

	of.lpstrFilter = filter;
	of.nFilterIndex = 2;

	of.lpstrFileTitle = NULL;
	of.nMaxFileTitle = 0;
	
	of.lpstrInitialDir = NULL;
	of.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	GetOpenFileNameW(&of);

	return platform::path(opened_path);
}

__FISSION_END__