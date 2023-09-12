
function include_demo(name, want_compile_shaders)
	project(name)
	location(name)
	kind 'WindowedApp'
	language 'C++'
	cppdialect 'c++20'
	targetdir ('%{wks.location}/bin/' .. OutputDir)
	objdir ('%{wks.location}/bin-int/' .. OutputDir .. '/%{prj.name}')

	files { '%{prj.location}/src/**.cpp', '%{prj.location}/src/**.h' }
	links { 'Fission' }

	if want_compile_shaders then
		prebuildcommands {
			"cd %{wks.location}/scripts",
			"python compile_shaders.py %{prj.location}"
		}
		prebuildmessage "Compiling Shaders..."
	end

	for library,path in pairs(FissionLinks) do
		links(library)
		if #path ~= 0 then
			libdirs(path)
		end
	end

	includedirs {
	'%{wks.location}/include',
	"%{wks.location}/Fission/%{IncludeDir.vulkan}",
	'%{wks.location}/Fission/vendor',
	'%{wks.location}/resources'
	}

	staticruntime 'On'

	filter 'configurations:Debug'
		defines { 'DEBUG' }
		symbols 'On'

	filter 'configurations:Release'
		defines { 'RELEASE' }
		optimize 'On'

	filter 'configurations:Dist'
		defines { 'DIST' }
		optimize 'Speed'
end

group 'demos'
  include_demo('Emission', true)
  include_demo('Simple Ball')
--  include_demo('Snake')
group ''
