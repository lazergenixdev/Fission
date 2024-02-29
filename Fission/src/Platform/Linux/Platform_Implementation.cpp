#include <Fission/Base/String.hpp>
#include <Fission/Platform.hpp>
#include <Fission/Platform/utils.h>
#include <Fission/Core/Display.hh>
#include <Fission/Core/Engine.hh>
#include <Fission/Core/Console.hh>
#include "../../Scene_Key_Parser.hpp"

#include <numeric>
#include <time.h>
#include <sys/utsname.h>

fs::string platform_version;


namespace fs::platform {
	struct Version_Data {
		Version_Data() {
            struct utsname buf;
            uname(&buf);
            platform_version.data  = (c8*)_Version;
			platform_version.count = snprintf(_Version, sizeof(_Version), "%s %s (%s)", buf.sysname, buf.machine, buf.release);
		}

		char _Version[64];
	};
}

fs::platform::Version_Data _platform_version_data{};



namespace fs {
    struct timespec temp;
#define NB 1'000'000'000

    s64 timestamp() {
        clock_gettime(CLOCK_MONOTONIC, &temp);
        return temp.tv_sec * NB + temp.tv_nsec; // <- this is fucking garbage
    }
    double seconds_elasped_and_reset(s64& last) {
        auto current = timestamp();
        auto duration = double(current - last) / 1e9;
        last = current;
        return duration;
    }

    Scene_Key cmdline_to_scene_key(platform::Instance const& instance) {
        Scene_Key key;

        // what the fuck is this?
        std::string temp = std::accumulate(
                std::next(instance.argv.begin()),
                instance.argv.end(),
                std::string{},
                [](std::string l, std::string r) {
                    return l + ' ' + r;
                }
        );
        if (temp.size()) temp.erase(temp.begin());

        string command_line = FS_str_std(temp);

        console::println(command_line);
        for (auto&& arg: instance.argv)
            console::println(FS_str_make(arg, strlen(arg)));

        Scene_Key_Parser parser{command_line};
        parser.parse(key.stream);
        return key;
    }

    void enumerate_displays(std::vector<struct Display>& out) {

    }
}

namespace fs::platform {
    struct Auto_Closing_File {
        FILE* handle = NULL;

        FILE** operator&() { return &handle; }

        ~Auto_Closing_File() {
            if (handle != NULL) fclose(handle);
        }
    };

    void* load_entire_file(path const& filepath, u64* out_file_size) {
        Auto_Closing_File file;
        file.handle = fopen(filepath.c_str(), "rb");

        if (file.handle == NULL) {
            FS_debug_printf("error: could not open file \"%s\"\n", filepath.string().c_str());
            return nullptr;
        }

        fseeko(file.handle, 0, SEEK_END);
        u64 file_size = (u64)ftello(file.handle);
        fseeko(file.handle, 0, SEEK_SET);

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


    path open_file_dialog(char const* _Name, char const* _Extensions) {
        return path();
    }
}
