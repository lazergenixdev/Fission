#include <Windows.h>
#include <uxtheme.h>

enum IMMERSIVE_HC_CACHE_MODE
{
    IHCM_USE_CACHED_VALUE,
    IHCM_REFRESH
};
enum WINDOWCOMPOSITIONATTRIB
{
    WCA_UNDEFINED = 0,
    WCA_NCRENDERING_ENABLED = 1,
    WCA_NCRENDERING_POLICY = 2,
    WCA_TRANSITIONS_FORCEDISABLED = 3,
    WCA_ALLOW_NCPAINT = 4,
    WCA_CAPTION_BUTTON_BOUNDS = 5,
    WCA_NONCLIENT_RTL_LAYOUT = 6,
    WCA_FORCE_ICONIC_REPRESENTATION = 7,
    WCA_EXTENDED_FRAME_BOUNDS = 8,
    WCA_HAS_ICONIC_BITMAP = 9,
    WCA_THEME_ATTRIBUTES = 10,
    WCA_NCRENDERING_EXILED = 11,
    WCA_NCADORNMENTINFO = 12,
    WCA_EXCLUDED_FROM_LIVEPREVIEW = 13,
    WCA_VIDEO_OVERLAY_ACTIVE = 14,
    WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
    WCA_DISALLOW_PEEK = 16,
    WCA_CLOAK = 17,
    WCA_CLOAKED = 18,
    WCA_ACCENT_POLICY = 19,
    WCA_FREEZE_REPRESENTATION = 20,
    WCA_EVER_UNCLOAKED = 21,
    WCA_VISUAL_OWNER = 22,
    WCA_HOLOGRAPHIC = 23,
    WCA_EXCLUDED_FROM_DDA = 24,
    WCA_PASSIVEUPDATEMODE = 25,
    WCA_USEDARKMODECOLORS = 26,
    WCA_LAST = 27
};
enum PreferredAppMode
{
    Default,
    AllowDark,
    ForceDark,
    ForceLight,
    Max
};
struct WINDOWCOMPOSITIONATTRIBDATA
{
    WINDOWCOMPOSITIONATTRIB Attrib;
    PVOID pvData;
    SIZE_T cbData;
};

using fnRtlGetNtVersionNumbers = void (WINAPI*)(LPDWORD major, LPDWORD minor, LPDWORD build);
using fnSetWindowCompositionAttribute = BOOL(WINAPI*)(HWND hWnd, WINDOWCOMPOSITIONATTRIBDATA*);
// 1809 17763
using fnShouldAppsUseDarkMode = bool (WINAPI*)(); // ordinal 132
using fnAllowDarkModeForApp = bool (WINAPI*)(bool allow); // ordinal 135, in 1809
using fnFlushMenuThemes = void (WINAPI*)(); // ordinal 136
using fnRefreshImmersiveColorPolicyState = void (WINAPI*)(); // ordinal 104
using fnIsDarkModeAllowedForWindow = bool (WINAPI*)(HWND hWnd); // ordinal 137
using fnGetIsImmersiveColorUsingHighContrast = bool (WINAPI*)(IMMERSIVE_HC_CACHE_MODE mode); // ordinal 106
using fnOpenNcThemeData = HTHEME(WINAPI*)(HWND hWnd, LPCWSTR pszClassList); // ordinal 49
// 1903 18362
using fnSetPreferredAppMode = PreferredAppMode(WINAPI*)(PreferredAppMode appMode); // ordinal 135, in 1903
using fnIsDarkModeAllowedForApp = bool (WINAPI*)(); // ordinal 139
using fnShouldSystemUseDarkMode = bool (WINAPI*)(); // ordinal 138
using fnAllowDarkModeForWindow = bool (WINAPI*)(HWND hWnd, bool allow); // ordinal 133

static fnAllowDarkModeForWindow _AllowDarkModeForWindow = nullptr;
static fnShouldAppsUseDarkMode _ShouldAppsUseDarkMode = nullptr;
static fnIsDarkModeAllowedForWindow _IsDarkModeAllowedForWindow = nullptr;
static fnSetWindowCompositionAttribute _SetWindowCompositionAttribute = nullptr;
static DWORD g_buildNumber = 0;

bool IsHighContrast()
{
    HIGHCONTRASTW highContrast = {sizeof(highContrast)};
    if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(highContrast), &highContrast, FALSE))
        return bool(highContrast.dwFlags & HCF_HIGHCONTRASTON);
    return false;
}

// Interface made to look like just regular windows API functions
void EnableDarkModeAPI() {
    fnAllowDarkModeForApp _AllowDarkModeForApp = nullptr;
    fnFlushMenuThemes _FlushMenuThemes = nullptr;
    fnRefreshImmersiveColorPolicyState _RefreshImmersiveColorPolicyState = nullptr;
    fnGetIsImmersiveColorUsingHighContrast _GetIsImmersiveColorUsingHighContrast = nullptr;
    fnOpenNcThemeData _OpenNcThemeData = nullptr;
    // 1903 18362
    fnShouldSystemUseDarkMode _ShouldSystemUseDarkMode = nullptr;
    fnSetPreferredAppMode _SetPreferredAppMode = nullptr;

    bool g_darkModeSupported = false;
    bool g_darkModeEnabled = false;


    auto AllowDarkModeForApp = [&](bool allow)
    {
        if (_AllowDarkModeForApp)
            _AllowDarkModeForApp(allow);
        else if (_SetPreferredAppMode)
            _SetPreferredAppMode(allow ? AllowDark : Default);
    };

    HMODULE hUxtheme = LoadLibraryExW(L"uxtheme.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (hUxtheme)
    {
        _OpenNcThemeData = reinterpret_cast<fnOpenNcThemeData>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(49)));
        _RefreshImmersiveColorPolicyState = reinterpret_cast<fnRefreshImmersiveColorPolicyState>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(104)));
        _GetIsImmersiveColorUsingHighContrast = reinterpret_cast<fnGetIsImmersiveColorUsingHighContrast>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(106)));
        _ShouldAppsUseDarkMode = reinterpret_cast<fnShouldAppsUseDarkMode>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(132)));
        _AllowDarkModeForWindow = reinterpret_cast<fnAllowDarkModeForWindow>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(133)));

        auto ord135 = GetProcAddress(hUxtheme, MAKEINTRESOURCEA(135));
        if (g_buildNumber < 18362)
            _AllowDarkModeForApp = reinterpret_cast<fnAllowDarkModeForApp>(ord135);
        else
            _SetPreferredAppMode = reinterpret_cast<fnSetPreferredAppMode>(ord135);

        //_FlushMenuThemes = reinterpret_cast<fnFlushMenuThemes>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(136)));
        _IsDarkModeAllowedForWindow = reinterpret_cast<fnIsDarkModeAllowedForWindow>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(137)));
        _SetWindowCompositionAttribute = reinterpret_cast<fnSetWindowCompositionAttribute>(GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetWindowCompositionAttribute"));

        if (_OpenNcThemeData &&
            _RefreshImmersiveColorPolicyState &&
            _ShouldAppsUseDarkMode &&
            _AllowDarkModeForWindow &&
            (_AllowDarkModeForApp || _SetPreferredAppMode) &&
            //_FlushMenuThemes &&
            _IsDarkModeAllowedForWindow)
        {
            g_darkModeSupported = true;

            AllowDarkModeForApp(true);
            _RefreshImmersiveColorPolicyState();

            g_darkModeEnabled = _ShouldAppsUseDarkMode() && !IsHighContrast();

            //FixDarkScrollBar();
        }
    }
}

void SetDarkModeForWindow(HWND hwnd) {
    _AllowDarkModeForWindow(hwnd, true);

    // Window should use dark mode when: ( _IsDarkModeAllowedForWindow(hWnd) && _ShouldAppsUseDarkMode() && !IsHighContrast() )
    // however, _IsDarkModeAllowedForWindow only returns true in debug builds.
    // I'm not sure of why this is the case, this could be looked into further.
    BOOL dark = !IsHighContrast();

    if (_SetWindowCompositionAttribute)
    {
        WINDOWCOMPOSITIONATTRIBDATA data = {WCA_USEDARKMODECOLORS, &dark, sizeof(dark)};
        _SetWindowCompositionAttribute(hwnd, &data);
    }
}
