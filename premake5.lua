if VULKAN_SDK == nil then
	VULKAN_SDK = os.getenv("VULKAN_SDK")
	if VULKAN_SDK == nil then
		error("Must have Vulkan SDK installed")
	end
end

-- how will this work when using Fission as a dependency?
function prebuild_shader_compile()
	prebuildcommands {
		"cd %{wks.location}/scripts",
		"python compile_shaders.py %{prj.location}"
	}
	prebuildmessage "Compiling Shaders..."
end

IncludeDir = {}
IncludeDir["vulkan"] 	= '%{VULKAN_SDK}/Include'
IncludeDir["freetype"] 	= 'vendor/freetype/include'

FissionLinks = {}
FissionLinks["vulkan-1"]="%{VULKAN_SDK}/Lib"
FissionLinks["freetype"]="%{wks.location}/Fission/vendor/freetype/%{OutputDir}"

if _FISSION_EXTERNAL then
	include 'Fission'
else
	workspace 'Fission'
	architecture "x86_64"
	configurations { 'Debug', 'Release', 'Dist' }
	
	OutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
	
	flags { 'MultiProcessorCompile', 'MFC' }
	defines '_CRT_SECURE_NO_WARNINGS'
	
	startproject 'sandbox'
	
	include 'Fission'
	include 'sandbox'
	include 'demos'
end

-- group "Dependencies"
-- 	include "Fission/vendor/yaml-cpp"
-- group ""