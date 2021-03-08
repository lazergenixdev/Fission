# Fission [![License](https://img.shields.io/github/license/lazergenixdev/Fission?color=dodgerblue&style=plastic)](https://github.com/lazergenixdev/Fission/blob/master/LICENSE)
### High performance game engine
Game Engine I am developing on for a specific game that I am working on, but thought that some others could also find some use in this project :)
\
\
This Engine mainly focuses on 2D graphics, but the rendering API is not limited to 2D (however some concepts are missing, such as depth stencils).
If you really want 3D graphics, then you would have to query the `Graphics` object for its native handle.
\
\
Only focusing on the Windows build for now, but after I got the main features stable I will move to work on an Andriod build as the next platform.

## Getting Started
```sh
git clone --recursive https://github.com/lazergenixdev/Fission
```
To make a Visual Studio Solution run `GenWindowsProj.bat` in the `scripts` directory

## Features
- Native Win32 support
- DirectX11 support
- Included 2D renderer with support for fonts
- Runtime Texture Atlas creation utility
- ImGui integration for debugging

## Short-Term Goals
- OpenGL support
- (not buggy) Fullscreen functionality + monitor picking
- Sound (mp3,wav,ogg)
- Application Icon/Cursor loading from Memory or File
- UI System
- Complete 2D renderer

## Long-Term Goals
### Platform support for:
- Mac
- IOS
- Andriod
- Linux ?
### Native Rendering APIs:
- DirectX 11 and 12
- Vulkan
- OpenGL
### Engine
- Simple + Fast 2D Physics library
- Powerful UI System
