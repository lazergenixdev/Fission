IncludeDir = {}
IncludeDir["yaml"] = "%{wks.location}/Fission/vendor/yaml-cpp/include"
IncludeDir["json"] = "%{wks.location}/Fission/vendor/json/single_include"
IncludeDir["freetype"] = '%{wks.location}/Fission/vendor/freetype/include'

if _FISSION_EXTERNAL then
	IncludeDir["yaml"] =     'vendor/yaml-cpp/include'
	IncludeDir["json"] =     'vendor/json/single_include'
	IncludeDir["freetype"] = 'vendor/freetype/include'
	include 'Fission'
	group "Dependencies"
		include "Fission/vendor/yaml-cpp"
	group ""
else
	workspace 'Fission'
		architecture "x86_64"
		configurations { 'Debug', 'Release', 'Dist' }
		
		OutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

		flags { 'MultiProcessorCompile', 'MFC' }
		defines '_CRT_SECURE_NO_WARNINGS'
		
		startproject 'sandbox'

	group "Dependencies"
		include "Fission/vendor/yaml-cpp"
	group ""

	include 'Fission'
	include 'sandbox'

	include 'demos'
end