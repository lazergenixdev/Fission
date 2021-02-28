workspace 'Fission'

	architecture "x86_64"

	configurations { 'Debug', 'Release', 'Dist' }

	
    flags { 'MultiProcessorCompile', 'MFC' }
	
	startproject 'Fission'

	
    OutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
	
    IncludeDir = {}
    IncludeDir["yaml"] = "%{wks.location}/Fission/vendor/yaml-cpp/include"
    IncludeDir["lazerlib"] = "%{wks.location}/Fission/vendor/lazerlib"

group "Dependencies"
    include "Fission/vendor/yaml-cpp"
    include "Fission/vendor/lazerlib"
group ""

include 'Fission'
