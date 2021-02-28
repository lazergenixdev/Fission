project 'Fission'
    kind 'SharedLib'
    language 'C++'
    cppdialect "C++17"

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

    filter "configurations:Debug"
        defines { "FISSION_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "FISSION_RELEASE" }
        optimize "On"

    filter "configurations:Dist"
        defines { "FISSION_DIST" }
        optimize "On"