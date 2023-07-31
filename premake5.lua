IncludeDir = {}
IncludeDir["yaml"] 		= "vendor/yaml-cpp/include"
IncludeDir["json"] 		= "vendor/json/single_include"
IncludeDir["freetype"] 	= 'vendor/freetype/include'
IncludeDir["lunasvg"] 	= 'vendor/lunasvg/include'
IncludeDir["vulkan"] 	= 'vendor/vulkan/include'

fission_links = {}
fission_links["vulkan-1"]="%{wks.location}/Fission/vendor/vulkan"
fission_links["freetype"]="%{wks.location}/Fission/vendor/freetype/%{OutputDir}"

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