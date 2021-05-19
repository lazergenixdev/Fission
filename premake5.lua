workspace 'Fission'

	architecture "x86_64"

	configurations { 'Debug', 'Release', 'Dist' }

	
    flags { 'MultiProcessorCompile', 'MFC' }
    defines '_CRT_SECURE_NO_WARNINGS'
	
	startproject 'sandbox'

	
    OutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
	
    IncludeDir = {}
    IncludeDir["yaml"] = "%{wks.location}/Fission/vendor/yaml-cpp/include"
    IncludeDir["lazerlib"] = "%{wks.location}/Fission/vendor/lazerlib"
    IncludeDir["freetype"] = '%{wks.location}/Fission/vendor/freetype/include'


group "Dependencies"
    include "Fission/vendor/yaml-cpp"
    include "Fission/vendor/lazerlib"
group ""

include 'Fission'
include 'sandbox'