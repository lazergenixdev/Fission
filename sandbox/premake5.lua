fission_project 'sandbox'

files { "%{prj.location}/src/**.cpp" }

prebuild_shader_compile()

includedirs {
	'%{prj.location}/src'
}
