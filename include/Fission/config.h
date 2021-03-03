#pragma once
#include "Platform/Platform.h" /*!< Determine Target Platform */

#ifdef FISSION_PLATFORM_WINDOWS
#ifdef FISSION_BUILD_DLL
#define FISSION_API __declspec(dllexport)
#else
#define FISSION_API __declspec(dllimport)
#endif // FISSION_BUILD_DLL
#endif // FISSION_PLATFORM_WINDOWS

#define FISSION_ENGINE "Fission" /*!< Engine Name */

#define FISSION_THREAD_SAFE /*!< Functions that should be thread safe */

/**
 * standard library includes
 */
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

/**
 * External helper library
 */
#include "lazer/lazer.h"

/**
 * convenient type aliases 
 */
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

/**
 * assertions
 */
#include <cassert>
#ifdef FISSION_DEBUG
#define FISSION_ASSERT( expression, msg ) assert( expression && "" msg )
#else
#define FISSION_ASSERT( expression, msg ) ((void)0)
#endif // FISSION_DEBUG

/**
 * macro helpers
 */
#define FISSION_MK_STR(X) #X
#define FISSION_MK_WSTR(X) L#X
#define FISSION_MK_WIDE(X) L##X

