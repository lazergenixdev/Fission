project 'Fission'
    kind 'StaticLib'
    language 'C++'
    cppdialect "c++20"

    os_links = {
        windows = function ()
            FISSION_LINKS["user32"] = ""
            FISSION_LINKS["gdi32"] = ""
        end;

        linux = function ()
            FISSION_LINKS["xcb"] = ""
            FISSION_LINKS["xcb-keysyms"] = ""
        end;
    }

    os_links[_TARGET_OS]()

    targetdir ("%{wks.location}/bin/" .. output_location)
	objdir ("%{wks.location}/bin-int/" .. output_location .. "/%{prj.name}")

    files { "%{prj.location}/src/*.cpp", "%{prj.location}/src/*.h" }

    -- public headers
    files '../include/**'

	includedirs (FISSION_INCLUDE_DIRS)

	includedirs {
        '../resources',
        "vendor",
	}
    
    staticruntime "On"
    defines { 'FISSION_BUILD' }
	
	if os.isfile("/dev/easter_eggs.hpp") and os.isfile("/dev/easter_eggs_setup.inl") then
		defines "FS_INCLUDE_EASTER_EGGS"
	end
    
    filter "system:windows"
        files { "resource.h", "Fission.rc" }
        files { "%{prj.location}/src/Platform/Windows/**.cpp" }

        -- HRESULT translation to readable strings
        includedirs '%{prj.location}/vendor/windows/DXErr'
    
        -- Texture processing library for windows
        includedirs '%{prj.location}/vendor/windows/DirectXTex/include'

    filter "system:linux"
        files { "%{prj.location}/src/Platform/Linux/**.cpp" }

    filter { "system:linux", "action:gmake" }
        buildoptions { "-march=native" }

    filter "configurations:Debug"
        defines { "FISSION_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "FISSION_RELEASE" }
        optimize "On"

    filter "configurations:Dist"
        defines { "FISSION_DIST" }
        optimize "Speed"
		
		
