project 'Fission'
    kind 'SharedLib'
    language 'C++'
    cppdialect "C++latest"

    targetdir ("%{wks.location}/bin/" .. OutputDir)
	objdir ("%{wks.location}/bin-int/" .. OutputDir .. "/%{prj.name}")

    files { "%{prj.location}/src/**.cpp", "%{prj.location}/src/**.h", "%{prj.location}/src/**.hh" }

    -- public headers
    files '../include/**'

    links { "yaml", "freetype" }

    libdirs
    {
        '%{prj.location}/vendor/freetype/' .. OutputDir
    }

	includedirs
	{
        "../include",
        "%{IncludeDir.yaml}",
        "%{IncludeDir.json}",
        '%{IncludeDir.freetype}',
        '../resources'
	}
    
    staticruntime "On"
    defines { 'FISSION_BUILD', 'FISSION_BUILD_DLL=1' }
    
    filter "system:windows"
        files { "resource.h", "Fission.rc" }

        links { 
            'user32',
            'ole32'
        }

        -- Graphics
        links 'd3dcompiler'

        -- HRESULT translation to readable strings
        includedirs '%{prj.location}/vendor/windows/DXErr'
    
        -- Texture processing library for windows
        includedirs '%{prj.location}/vendor/windows/DirectXTex/include'
        libdirs { "%{prj.location}/vendor/windows/DirectXTex/%{cfg.buildcfg}-%{cfg.architecture}" }
        links { 'DirectXTex' }

    filter "configurations:Debug"
        defines { "FISSION_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "FISSION_RELEASE" }
        optimize "On"

    filter "configurations:Dist"
        defines { "FISSION_DIST" }
        optimize "Speed"