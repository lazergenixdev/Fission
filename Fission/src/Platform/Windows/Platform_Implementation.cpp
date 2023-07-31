#include <Fission/Base/String.hpp>
#include <Fission/Platform.hpp>

fs::string platform_version;

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

double seconds_elasped_and_reset(s64& last) {
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	double elapsed = double(now.QuadPart - last) / _freq.value;
	last = now.QuadPart;
	return elapsed;
}

__FISSION_END__