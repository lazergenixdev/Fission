project 'Fission'
    kind 'StaticLib'
    language 'C++'
    cppdialect "c++20"

	function add_link(name)
		FISSION_LINKS[name] = ""
	end

    targetdir ("%{wks.location}/bin/" .. output_location)
	objdir ("%{wks.location}/bin-int/" .. output_location .. "/%{prj.name}")

    files { "%{prj.location}/src/**.cpp", "%{prj.location}/src/**.h", "%{prj.location}/src/**.hh" }

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

		add_link("user32")
		add_link("gdi32")

        -- HRESULT translation to readable strings
        includedirs '%{prj.location}/vendor/windows/DXErr'
    
        -- Texture processing library for windows
        includedirs '%{prj.location}/vendor/windows/DirectXTex/include'

    filter "configurations:Debug"
        defines { "FISSION_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "FISSION_RELEASE" }
        optimize "On"

    filter "configurations:Dist"
        defines { "FISSION_DIST" }
        optimize "Speed"
		
		