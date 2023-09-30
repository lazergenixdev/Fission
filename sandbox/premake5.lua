fission_project 'sandbox'

files { "%{prj.location}/src/**.cpp" }

prebuild_shader_compile("%{prj.location}")

includedirs {
	'%{prj.location}/src'
}
