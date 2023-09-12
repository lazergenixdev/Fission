project 'sandbox'
    kind 'WindowedApp'
    language 'C++'
    cppdialect "c++20"

    targetdir ("%{wks.location}/bin/" .. OutputDir)
	objdir ("%{wks.location}/bin-int/" .. OutputDir .. "/%{prj.name}")

    files { "%{prj.location}/src/**.cpp" }

    links { 'Fission' }
	
	-- compile shaders
	prebuildcommands {
		"cd %{wks.location}/scripts",
		"python compile_shaders.py %{prj.location}"
	}
	prebuildmessage "Compiling Shaders..."

	print("--------------------------------! Active Libraries !--------------------------------")
    for library,path in pairs(FissionLinks) do
		print(library,path)
		links(library)
		if #path ~= 0 then
			libdirs(path)
		end
    end
	print("------------------------------------------------------------------------------------")
	
	includedirs {
        '%{wks.location}/include',
        "%{wks.location}/Fission/%{IncludeDir.vulkan}",
        "%{wks.location}/Fission/%{IncludeDir.freetype}",
        '%{wks.location}/Fission/vendor',
		'%{prj.location}/src'
	}
    
    staticruntime "On"

    filter "configurations:Debug"
        defines "DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "RELEASE"
        optimize "On"

    filter "configurations:Dist"
        defines "DIST"
        optimize "Speed"
