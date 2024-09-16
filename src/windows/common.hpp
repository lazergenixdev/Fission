#pragma once
#include <Fission/base/string.hpp>
#include <Fission/platform.hpp>
#include <cstdio>
#include <stringapiset.h>

enum {
    _WM_USER_START = WM_USER,
    WM_USER_SHOW_DIALOG,
    WM_USER_DESTROY,
};

/// @note: use LocalFree to free result
inline auto win32_to_wide_string(fs::string const& str) -> WCHAR* {
  int count = MultiByteToWideChar(CP_UTF8, 0, (char const*)str.data,
                                         (int)str.count, nullptr, 0);
  WCHAR* wstr = (WCHAR*)LocalAlloc(0, (count + 1) * sizeof(WCHAR));
  MultiByteToWideChar(CP_UTF8, 0, (char const*)str.data, (int)str.count, wstr,
                      count);
  wstr[count] = '\0';
    return wstr;
}

/// @note: use LocalFree to free result
inline auto win32_to_utf8_string(fs::string_utf16 const& wstr) -> char* {
    int count = WideCharToMultiByte(CP_UTF8, 0, (WCHAR*)wstr.data,
                                         (int)wstr.count, nullptr, 0, 0, 0);
  char* str = (char*)LocalAlloc(0, (count + 1) * sizeof(char));
  WideCharToMultiByte(CP_UTF8, 0, (WCHAR*)wstr.data,
                                  (int)wstr.count, str, count, 0, 0);
    str[count] = '\0';
    return str;
}

/// @note: use LocalFree to free result
inline auto win32_error_string(DWORD error_code) -> WCHAR* { 
    WCHAR* w_error_string;

    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, error_code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &w_error_string,
        0, NULL );

    return w_error_string;
}

inline void report_error(fs::string const& function_name, fs::string const& message)
{
  auto error_code = GetLastError();
  auto w_title = win32_to_wide_string(message);
  auto w_description = win32_error_string(error_code);
  auto w_function_name = win32_to_wide_string(function_name);
  WCHAR* w_buffer =
      (WCHAR*)LocalAlloc(0, (lstrlen(w_description) + 128) * sizeof(WCHAR));

  {
      auto description =
          win32_to_utf8_string({(fs::u64)lstrlenW(w_description), (fs::c16*)w_description});

      { // Remove all newlines
          char* dst = description;
          char* src = description;
          while (*src != 0) {
            if (*src == '\r' || *src == '\n')
              src++;
            else
              *dst++ = *src++;
          }
          *dst = '\0';
      }

      int count = sprintf_s(
          (char*)w_buffer, LocalSize(w_buffer), "%s failed with error %i: %s",
               function_name.str().c_str(), (int)error_code, description);

      fs::log::error({w_buffer, (fs::u64)count});
      fs::log::error(message);

      LocalFree(description);
  }

  swprintf_s(w_buffer, LocalSize(w_buffer) / sizeof(WCHAR),
             L"%s failed with error %i:\n\n%s", w_function_name,
             (int)error_code, w_description);

  MessageBoxW(NULL, w_buffer, w_title, MB_ICONERROR | MB_OK);

  LocalFree(w_title);
  LocalFree(w_description);
  LocalFree(w_function_name);
  LocalFree(w_buffer);
}