
function include_demo(name)
  project(name)
  location(name)
  kind 'WindowedApp'
  language 'C++'
  cppdialect 'C++latest'
  targetdir ('%{wks.location}/bin/' .. OutputDir)
  objdir ('%{wks.location}/bin-int/' .. OutputDir .. '/%{prj.name}')

  files { '%{prj.location}/src/**.cpp' }
  links { 'Fission', 'freetype' }

  libdirs
  {
    '%{wks.location}/Fission/vendor/freetype/' .. OutputDir
  }

  includedirs
  {
    '%{wks.location}/include',
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
  include_demo('Simple Ball')
group ''
