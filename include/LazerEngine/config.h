#pragma once

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "Platform/Platform.h"
#include "lazer/lazer.h"

#ifdef LAZER_PLATFORM_WINDOWS
#ifdef LAZER_ENGINE_BUILD_DLL
#define LAZER_API __declspec(dllexport)
#else
#define LAZER_API __declspec(dllimport)
#endif // LAZER_ENGINE_BUILD_DLL
#endif // LAZER_PLATFORM_WINDOWS

#ifndef interface
#define interface struct
#endif

#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <optional>
#include <string>
#include <sstream>
#include <array>
#include <vector>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <unordered_map>
#include <map>
#include <filesystem>

namespace lazer {

	using uint8 = uint8_t;
	using uint32 = uint32_t;
	using uint16 = uint16_t;
	using uint64 = uint64_t;

	using int8 = int8_t;
	using int16 = int16_t;
	using int32 = int32_t;
	using LZ_Int64 = int64_t;
}


#include <cassert>

#define LAZER_ASSERT( expression, msg ) assert( expression && msg )

#ifdef _DEBUG
#define LAZER_DEBUG 1
#else
#define LAZER_DEBUG 0
#endif

