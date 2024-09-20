
VULKAN_SDK = os.getenv("VULKAN_SDK")
if VULKAN_SDK == nil then
    error("Must have Vulkan SDK installed")
end

if _ACTION == 'android-studio' then
	gradleversion 'com.android.tools.build:gradle:8.6.0'
	gradlewrapper {
		'distributionUrl=https://services.gradle.org/distributions/gradle-8.10.1-bin.zip'
	}
    gradleproperties {
        'android.useAndroidX=true',
    }
	assetpacks { ['pack'] = 'install-time', }
	androidnamespace (settings.namespace)
	androidminsdkversion '34'
	androidsdkversion '34'
    -- fuck you android!
    androidbuildsettings {
        [[configurations.all {
            resolutionStrategy {
                force 'org.jetbrains.kotlin:kotlin-stdlib:1.8.22'
                force 'org.jetbrains.kotlin:kotlin-stdlib-jdk8:1.8.22'
                force 'org.jetbrains.kotlin:kotlin-stdlib-jdk7:1.8.22'
            }
        }]],
    }
    --androidabis {
    --    'armeabi-v7a',
    --    'arm64-v8a',
    --    'x86',
    --    'x86_64'
    --}
elseif _ACTION == 'vs2022' then
    architecture 'x86_64'
end

-- Example:
--   settings = {
--       target_location = '%{wks.location}/bin/%{cfg.system}-%{cfg.buildcfg}'
--       build_location = '%{wks.location}/build/' .. _ACTION,
--       namespace = 'com.lazergenixdev.Example',
--       title = 'Android Vulkan Example',
--       fission_location = 'some/path/Fission' -- Default: '%{wks.location}/Fission'
--   }
if not settings then
	error 'Fission: "settings" not defined!'
end
if not settings.build_location then
	error 'Fission: "settings.build_location" not defined!'
end
if not settings.namespace then
	error 'Fission: "settings.namespace" not defined!'
end
if not settings.title then
	error 'Fission: "settings.title" not defined!'
end
if not settings.target_location then
	error 'Fission: "settings.target_location" not defined!'
end
settings.fission_location = path.translate(path.getdirectory(_SCRIPT), '/')
printf ('Fission location "%s"', settings.fission_location)

location (settings.build_location)

defines { ('__TITLE__=\"%s\"'):format(settings.title) }

project 'Fission'

kind 'StaticLib'

targetdir (settings.target_location)

includedirs { 'include', 'src' }
includedirs { VULKAN_SDK .. '/Include' }
files { 'include/**' }
files { 'src/*.cpp', 'src/*.hpp' }

if _ACTION == 'android-studio' then

    files { 'src/android/*.cpp', }
    links { 'log', 'android', 'vulkan' }

	-- #define __ANDROID_NAMESPACE__  some.given.namespace
    defines { ('__ANDROID_NAMESPACE__=%s'):format(settings.namespace:gsub('%.', '_')) }

else -- Windows
    buildoptions { '/utf-8' }
    files { 'src/windows/**' }
	libdirs { '%{VULKAN_SDK}/Lib' }
	links {
        'vulkan-1',
	}
end

fission = function ()
	links { 'Fission' }
    includedirs { '%{settings.fission_location}/include' }
    includedirs { VULKAN_SDK .. '/Include' }
    buildoptions { '/utf-8' }
end

android = function (info)
    if _ACTION ~= 'android-studio' then return end;

    -- Vulkan Validation Layers
    term.pushColor (term.blue)
    print 'Copying Vulkan Validation Layers...'
    local src = '%[' .. settings.fission_location .. '/src/android/jniLibs/]'
    local dst = '%[' .. settings.build_location .. '/%{prj.name}/src/main/jniLibs]'
    local cmd = '{COPYDIR} ' .. src .. ' ' .. dst
    --prebuildcommands (cmd)
    
    local cmd = os.translateCommands ( '{COPYDIR} ' .. settings.fission_location .. '/src/android/jniLibs/ ../' .. settings.build_location .. '/' .. info.name .. '/src/main/jniLibs' )
    os.execute (cmd)
    term.popColor ()

	-- Generate Java Activity
    term.pushColor (term.blue)
    print 'Generating Activity.java...'
    os.execute (
        string.format (
            "python %s/scripts/template.py %s/src/android/MainActivity.template.java \"{'name': '%s', 'namespace': '%s'}\" -O \"%s\"",
            settings.fission_location,
            settings.fission_location,
            info.name,
            settings.namespace,
            '../' .. settings.build_location .. '/' .. info.name .. '/src/java/'
        )
    )
    term.popColor ()
    
	-- Generate Resource Files
    term.pushColor (term.blue)
        print 'Generating strings.xml...'
        os.mkdir ('../' .. settings.build_location .. '/' .. info.name .. '/res/values/')
        io.writefile (
            '../' .. settings.build_location .. '/' .. info.name .. '/res/values/strings.xml',
            '<resources><string name="title">' .. settings.title .. '</string></resources>'
        )
        files ('../' .. settings.build_location .. '/' .. info.name .. '/res/values/*')
    term.popColor ()
    
    files { '../' .. settings.build_location .. '/' .. info.name .. '/src/java/*' }
    files { settings.fission_location .. '/src/android/AndroidManifest.xml' }
    files { settings.fission_location .. '/src/android/res/**' }

    --buildoptions { '-std=c++17' }
    androiddependencies {
        'androidx.appcompat:appcompat:1.7.0'
    }
    assetpackdependencies { 'pack' }
    
    assetdirs {
        settings.fission_location .. "/resources",    
    }
end