
project 'Fission'
    kind 'StaticLib'
    language 'C++'
    cppdialect "c++20"

	function add_link(name)
		fission_links[name] = ""
	end

    targetdir ("%{wks.location}/bin/" .. OutputDir)
	objdir ("%{wks.location}/bin-int/" .. OutputDir .. "/%{prj.name}")

    files { "%{prj.location}/src/**.cpp", "%{prj.location}/src/**.h", "%{prj.location}/src/**.hh" }

    -- public headers
    files '../include/**'

	includedirs
	{
        "../include",
        "%{IncludeDir.yaml}",
        "%{IncludeDir.json}",
        '%{IncludeDir.freetype}',
        '%{IncludeDir.lunasvg}',
        '%{IncludeDir.vulkan}',
        '../resources',
        "vendor",
	}
    
    staticruntime "On"
    defines { 'FISSION_BUILD' }
	
    
    filter "system:windows"
        files { "resource.h", "Fission.rc" }

		add_link("user32")
		add_link("gdi32")

        -- HRESULT translation to readable strings
        includedirs '%{prj.location}/vendor/windows/DXErr'
    
        -- Texture processing library for windows
        includedirs '%{prj.location}/vendor/windows/DirectXTex/include'
    --    libdirs { "%{prj.location}/vendor/windows/DirectXTex/%{cfg.buildcfg}-%{cfg.architecture}" }
    --    links { 'DirectXTex' }

    filter "configurations:Debug"
        defines { "FISSION_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "FISSION_RELEASE" }
        optimize "On"

    filter "configurations:Dist"
        defines { "FISSION_DIST" }
        optimize "Speed"
		
		