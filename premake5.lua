IncludeDir = {}
IncludeDir["yaml"] 		= "vendor/yaml-cpp/include"
IncludeDir["json"] 		= "vendor/json/single_include"
IncludeDir["freetype"] 	= 'vendor/freetype/include'

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

group "Dependencies"
	include "Fission/vendor/yaml-cpp"
group ""