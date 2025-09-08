/** @file profiler.hpp
	@author lazergenixdev@gmail.com

	@brief Implements simple profiler tool that uses Chrome Tracing to display tracing information
	
	This is a single header,
	define PROFILER_IMPLEMENTATION in a .cpp file to use this API
	
	// Example usage:
	#define PROFILER_IMPLEMENTATION
	#include "profiler.hpp"
	
	extern void wait_ms(int milliseconds);
	
	int main() {
		profiler::Session session{100}; // max trace events = 100
		
		{
			profiler::scoped_trace trace{&session, 0, "start"};
			wait_ms(100);
		}
		{
			profiler::scoped_trace trace{&session, 0, "do_work"};
			wait_ms(300);
		}
		{
			profiler::scoped_trace trace{&session, 0, "end"};
			wait_ms(100);
		}
		
		session.save("out.trace");
		
		// this call can be seperate from the application doing the profile
		generate_chrome_tracing_from_file("out.trace");
	}

	This work is released to Public Domain, do whatever you want with it.
*/
#pragma once
#include <cstdint>
#include <memory>
#include <fstream>
#include <unordered_map>
#include <stdexcept>
#include <cstdlib>

// Only windows is supported for now
#define PROFILER_PLATFORM_WINDOWS 1
#undef PROFILER_PLATFORM_LINUX
#undef PROFILER_PLATFORM_MACOS

namespace profiler {

#ifdef PROFILER_PLATFORM_WINDOWS
using int_sync = volatile long;
#define PROFILER_INTERLOCKED_INC(X) _InterlockedIncrement(&X)
#define PROFILER_INTERLOCKED_DEC(X) _InterlockedDecrement(&X)
#endif

#define PROFILER_XRESULTS \
X(Failed, "operation failed") \
X(FailedToOpenFile, "failed to open file") \
X(FailedToCreateFile, "failed to create file") \

enum result {
	result_Success = 0,
#define X(NAME,STRING) result_Error_ ## NAME,
	PROFILER_XRESULTS
#undef X
};

static constexpr char const* error_str(result r) {
	switch(r) {
	case result_Success: return "no error";
#define X(NAME,STRING) case result_Error_ ## NAME: return STRING;
	PROFILER_XRESULTS
#undef X
	default: return "unknown error";
	}
}

// takes a .trace file and creates a chrome tracing JSON file
result generate_chrome_tracing_from_file(char const* in_filename, char const* out_filename = "trace.json");

// 16 bytes per trace event
struct Raw_Trace_Event {
	uint16_t whatid;
	uint16_t threadid;
	uint32_t duration_us;
	uint64_t timestamp_us;
};

struct Trace_Event {
	char const* what;
	uint16_t threadid;
	uint32_t duration_us;
	uint64_t timestamp_us;
};

struct Session
{
public:
	Trace_Event* trace_data;
	int          trace_count;  // total number of traces we can contain
	int_sync     trace_occupied; // how many traces we have

	Session() noexcept:
		trace_data(nullptr), trace_count(0),
		trace_occupied(0), thread_names(nullptr),
		thread_count(0), thread_names_size(0)
	{}
	Session(int max_trace_count) noexcept { init(max_trace_count); }
	~Session() noexcept { uninit(); }

	void init(int max_trace_count) noexcept;
	void uninit() noexcept;

	void insert_event(Trace_Event const& event);
	void name_threads(char const** ptr_thread_name_strings, int count);
	
	// Must not be called while inserting an event!
	void reset() { trace_occupied = 0; }

	// Generate a .trace file
	// Must not be called while inserting an event!
	void save(char const* filename);
private:
	char* thread_names;
	int thread_count;
	int thread_names_size;
};


#ifdef PROFILER_IMPLEMENTATION

#define _aligned64_mallocT(COUNT,TYPE) (TYPE*)_aligned_malloc(COUNT*sizeof(TYPE), 64)
#define _read_into(STREAM,VAR)         STREAM.read((char*)&VAR, sizeof(VAR))
#define _write_from(STREAM,VAR)        STREAM.write((char*)&VAR, sizeof(VAR))
#define _u64_to_str(BUFFER, VALUE)     _ui64toa_s((unsigned long long)VALUE, BUFFER, std::size(BUFFER), 10)

inline result generate_chrome_tracing_from_file(char const* in_filename, char const* out_filename)
{
	using f = std::ios;

	auto is = std::ifstream( in_filename, f::binary);
	if( !is.is_open() ) {
		return result_Error_FailedToOpenFile;
	}
	is.exceptions(f::badbit | f::failbit);
	
	auto os = std::ofstream(out_filename, f::binary);
	if( !os.is_open() ) {
		return result_Error_FailedToCreateFile;
	}
	os.exceptions(f::badbit | f::failbit);


	try {
		uint32_t count;
		_read_into(is, count);

		// this is truly something special :(
		std::vector<std::string> functions;
		functions.reserve(count);
		std::string tmp;
		while (functions.size() < count) {
			char ch = (char)is.get();
			if (ch == '\0') {
				functions.emplace_back(std::move(tmp));
			}
			else tmp.push_back(ch);
		}

		uint32_t thread_count;
		_read_into(is, thread_count);

		std::vector<std::string> thread_names;
		thread_names.reserve(thread_count);
		while (thread_names.size() < thread_count) {
			char ch = (char)is.get();
			if (ch == '\0') {
				thread_names.emplace_back(std::move(tmp));
			}
			else tmp.push_back(ch);
		}

		uint32_t event_count;
		_read_into(is, event_count);

		os << R"({"otherData":{},"traceEvents":[)";

		char buffer[72]; // big buffer just in case
		Raw_Trace_Event trace;
		for (uint32_t k = 0;;) {
			_read_into(is, trace);
			/*
			{
				"cat": "function",
				"dur": 600,
				"name": "void main()",
				"ph": "X",
				"pid": 0,
				"tid": "Main Thread",
				"ts": 834734345
			},
			*/
			os << R"({"cat":"function","dur":)";

			// duration
			_u64_to_str(buffer, trace.duration_us);
			os << buffer;

			os << R"(,"name":")";

			// name
			os << functions[trace.whatid];

			os << R"(","ph":"X","pid":0,"tid":)";

			// thread id
			if (trace.threadid < thread_count) {
				os << '"' << thread_names[trace.threadid] << '"';
			}
			else {
				_u64_to_str(buffer, trace.threadid);
				os << buffer;
			}

			os << R"(,"ts":)";

			// timestamp
			_u64_to_str(buffer, trace.timestamp_us);
			os << buffer;

			os << '}';
			if (++k < event_count) {
				os << ',';
			}
			else break;
		}
		os << R"(]})";
	}

	catch (...) {
		// sorry if this happens to you...
		return result_Error_Failed;
	}

	return result_Success;
}

void Session::init(int max_trace_count) noexcept
{
	// TODO: make this allocation more portable?
	trace_data = _aligned64_mallocT(max_trace_count, Trace_Event);
	trace_count = trace_data ? max_trace_count : 0;
	trace_occupied = 0;

	thread_names = nullptr;
	thread_count = 0;
}

void Session::uninit() noexcept
{
	if (trace_data) {
		trace_count = 0;
		_aligned_free(trace_data);
		trace_data = nullptr;
	}
	if (thread_names) {
		thread_count = 0;
		free(thread_names);
		thread_names = nullptr;
	}
}

void Session::insert_event(Trace_Event const& event)
{
	auto next = PROFILER_INTERLOCKED_INC(trace_occupied) - 1;
	if (next >= trace_count) {
		PROFILER_INTERLOCKED_DEC(trace_occupied);
		return;
	}
	trace_data[next] = event;
}

namespace helper {
	void strcpy(char*& dst, char const* src) {
		for (;;) {
			*dst++ = *src;
			if (*src++ == 0) break;
		}
	}
	template <typename T> struct buffer_view {
		T const* data;
		size_t count;

		buffer_view(T const* data, size_t count): data(data), count(count) {}
		T const* begin() const { return data; }
		T const* end  () const { return data + count; }
	};
}

void Session::name_threads(char const** ptr_thread_name_strings, int count)
{
	size_t total_size = 0;
	for (int i = 0; i < count; ++i) {
		total_size += strlen(ptr_thread_name_strings[i]) + 1;
	}
	thread_names_size = (int)total_size;

	auto names = (char*)malloc(total_size);
	if (names == nullptr) throw std::bad_alloc();
	thread_names = names;
	thread_count = count;

	for (int i = 0; i < count; ++i) {
		helper::strcpy(names, ptr_thread_name_strings[i]);
	}
}

void Session::save(char const* filename)
{
	using f = std::ios;

	// create hash table for our "function names"
	std::unordered_map<char const*, int> map;
	std::vector<char const*> what_array;
	{
		what_array.reserve(32);
		map.reserve(32);
		int count = 0;
		for (auto&& trace : helper::buffer_view(trace_data, trace_occupied)) {
			auto it = map.find(trace.what);
			if (it == map.end()) {
				map.insert({ trace.what, count++ });
				what_array.emplace_back(trace.what);
			}
		}
	}

	auto temp = _aligned64_mallocT(trace_occupied, Raw_Trace_Event);
	if (temp == nullptr) throw std::bad_alloc();

	// Fill our raw buffer
	auto pCurrent = temp;
	for (auto&& trace : helper::buffer_view(trace_data, trace_occupied)) {
		pCurrent->whatid = map[trace.what];
		pCurrent->threadid = trace.threadid;
		pCurrent->duration_us = trace.duration_us;
		pCurrent->timestamp_us = trace.timestamp_us;
		++pCurrent;
	}

	// finally write to file
	if (auto os = std::ofstream(filename, f::binary)) {

		// First write our strings
		uint32_t count = (uint32_t)what_array.size();
		_write_from(os, count);
		for (auto&& what : what_array) {
			os << what << '\0';
		}

		// Second write our thread names
		_write_from(os, thread_count);
		if (thread_count > 0) {
			os.write(thread_names, thread_names_size);
		}

		// Third write the raw trace data
		uint32_t event_count = trace_occupied;
		_write_from(os, event_count);
		os.write((char*)temp, event_count * sizeof(Raw_Trace_Event));
	}

	_aligned_free(temp);
}

#undef _aligned64_mallocT
#undef _read_into
#undef _write_from
#undef _u64_to_str

#endif // PROFILER_IMPLEMENTATION

#ifndef PROFILER_NO_SCOPED_TRACE
#define microseconds(TS) ((TS.QuadPart*1000000)/s_Frequency.QuadPart)
// Windows Only (for now...)
struct scoped_trace {
	Session* session;
	char const* what;
	uint16_t threadid;
	LARGE_INTEGER start;

	static inline LARGE_INTEGER s_Frequency = {1};
	static inline LARGE_INTEGER s_PrevStart;
	static inline LARGE_INTEGER s_PrevEnd;

	scoped_trace(profiler::Session* session, int threadid, char const* what) :
		session(session), threadid(threadid), what(what)
	{
	last_resort: // this is my last resort
		QueryPerformanceCounter(&start);

		// Ensure no two start times are the same
		if (microseconds(start) == microseconds(s_PrevStart))
			goto last_resort;
		//	start.QuadPart += s_Frequency.QuadPart / 1'000'000;

		s_PrevStart = start;
	}
	~scoped_trace() {
		LARGE_INTEGER end, ElapsedMicroseconds;
	last_resort:
		QueryPerformanceCounter(&end);

		// Ensure no two end times are the same
		if (microseconds(end) == microseconds(s_PrevEnd))
			goto last_resort;
		//	end.QuadPart += 4*s_Frequency.QuadPart / 1'000'000;

		s_PrevEnd = end;

		ElapsedMicroseconds.QuadPart = end.QuadPart - start.QuadPart;

		// Convert to microseconds
		ElapsedMicroseconds.QuadPart *= 1'000'000;
		ElapsedMicroseconds.QuadPart /= s_Frequency.QuadPart;

		uint32_t duration;
		if (ElapsedMicroseconds.QuadPart > UINT32_MAX) {
			duration = UINT32_MAX;
		}
		else duration = static_cast<uint32_t>(ElapsedMicroseconds.QuadPart);

		session->insert_event({ what, threadid, duration, (unsigned)(microseconds(start)) });
	//	session->insert_event({ what, threadid, duration, (unsigned)(start.QuadPart) });
	}
private:
	static void init() {
		QueryPerformanceFrequency(&s_Frequency);
		s_PrevStart.QuadPart = 0;
		s_PrevEnd.QuadPart = 0;
	}

	struct doinit {
		doinit() { scoped_trace::init(); }
	};
	static inline doinit _doinit;
#undef microseconds
};
#endif // !PROFILER_NO_SCOPED_TRACE

} // namespace profiler