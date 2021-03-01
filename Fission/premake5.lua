project 'Fission'
    kind 'SharedLib'
    language 'C++'
    cppdialect "C++latest"

    targetdir ("%{wks.location}/bin/" .. OutputDir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. OutputDir .. "/%{prj.name}")

    files { "%{prj.location}/src/**.cpp", "%{wks.location}/include/**.h" }

    links { "yaml" }

	includedirs
	{
        '%{wks.location}/include',
        "%{IncludeDir.yaml}",
        "%{IncludeDir.lazerlib}",
        '%{prj.location}/resources'
	}
    
    staticruntime "On"
    defines 'FISSION_BUILD_DLL'
    
    filter "system:windows"
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
        optimize "On"