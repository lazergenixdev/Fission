#pragma once

#include "Platform/Platform.h"
#include "lazer/lazer.h"

#define FISSION_ENGINE "Fission"

#ifdef FISSION_PLATFORM_WINDOWS
#ifdef FISSION_BUILD_DLL
#define FISSION_API __declspec(dllexport)
#else
#define FISSION_API __declspec(dllimport)
#endif // FISSION_BUILD_DLL
#endif // FISSION_PLATFORM_WINDOWS

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

namespace Fission {

	using namespace lazer;

	using uInt8 = uint8_t;
	using uInt32 = uint32_t;
	using uInt16 = uint16_t;
	using uInt64 = uint64_t;

	using Int8 = int8_t;
	using Int16 = int16_t;
	using Int32 = int32_t;
	using Int64 = int64_t;

}

#include <cassert>

#define LAZER_ASSERT( expression, msg ) assert( expression && "" msg )

#ifdef DEBUG
#define FISSION_DEBUG
#endif

// reminder that these functions can introduce race conditions
#define FISSION_THREAD_SAFE
