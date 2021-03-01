project 'sandbox'
    kind 'WindowedApp'
    language 'C++'
    cppdialect "C++latest"

    targetdir ("%{wks.location}/bin/" .. OutputDir)
	objdir ("%{wks.location}/bin-int/" .. OutputDir .. "/%{prj.name}")

    files { "%{prj.location}/src/**.cpp" }

    links { 'Fission' }

	includedirs
	{
        '%{wks.location}/include',
        "%{IncludeDir.lazerlib}",
        '%{prj.location}/resources'
	}
    
    staticruntime "On"

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "RELEASE" }
        optimize "On"

    filter "configurations:Dist"
        defines { "DIST" }
        optimize "On"
