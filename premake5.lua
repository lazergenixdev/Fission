output_location = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

if VULKAN_SDK == nil then
	VULKAN_SDK = os.getenv("VULKAN_SDK")
	if VULKAN_SDK == nil then
		error("Must have Vulkan SDK installed")
	end
end

function prebuild_shader_compile(location)
--	prebuildcommands {
--		"cd " .. FISSION_LOCATION .. "/scripts",
--		"python compile_shaders.py " .. location
--	}
	prebuildmessage "Compiling Shaders..."
end

function fission_project(name)
	project (name)
    kind 'WindowedApp'
    language 'C++'
    cppdialect "c++20"

    targetdir ("%{wks.location}/bin/" .. output_location)
	objdir ("%{wks.location}/bin-int/" .. output_location .. "/%{prj.name}")

    links 'Fission'
	
	print("--------------------------------! Active Libraries !--------------------------------")
    for library,path in pairs(FISSION_LINKS) do
		print(library,path)
		links(library)
		if #path ~= 0 then
			libdirs(path)
		end
    end
	print("------------------------------------------------------------------------------------")
	includedirs (FISSION_INCLUDE_DIRS)

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

	filter {}
end

FISSION_INCLUDE_DIRS = {}
FISSION_INCLUDE_DIRS["Fission"]   = '%{FISSION_LOCATION}/include'
FISSION_INCLUDE_DIRS["vulkan"]   = '%{VULKAN_SDK}/include'
FISSION_INCLUDE_DIRS["freetype"] = '%{FISSION_LOCATION}/Fission/vendor/freetype/include'

-- Links needed to compile with Fission Engine
FISSION_LINKS = {}
FISSION_LINKS["freetype"]="%{FISSION_LOCATION}/Fission/vendor/freetype/%{output_location}"
if _TARGET_OS == "windows" then
	FISSION_LINKS["vulkan-1"]="%{VULKAN_SDK}/lib"
else
	FISSION_LINKS["vulkan"]="%{VULKAN_SDK}/lib"
end

if FISSION_EXTERNAL then
	include 'Fission'
else
	workspace 'Fission'
	architecture "x86_64"
	configurations { 'Debug', 'Release', 'Dist' }
	
    if _TARGET_OS == "windows" then
        flags { 'MultiProcessorCompile', 'MFC' }
        defines '_CRT_SECURE_NO_WARNINGS'
    end

	FISSION_LOCATION = '%{wks.location}'
	startproject 'sandbox'
	
	include 'Fission'
	include 'sandbox'
end
