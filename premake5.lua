
workspace 'Fission'
	architecture "x86_64"

	configurations { 'Debug', 'Release', 'Dist' }

	
    flags { 'MultiProcessorCompile', 'MFC' }
    defines '_CRT_SECURE_NO_WARNINGS'
	
	startproject 'sandbox'

	
    OutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
	
    IncludeDir = {}
    IncludeDir["yaml"] = "%{wks.location}/Fission/vendor/yaml-cpp/include"
    IncludeDir["json"] = "%{wks.location}/Fission/vendor/json/single_include"
    IncludeDir["freetype"] = '%{wks.location}/Fission/vendor/freetype/include'


group "Dependencies"
    include "Fission/vendor/yaml-cpp"
group ""

include 'Fission'
include 'sandbox'

include 'demos'