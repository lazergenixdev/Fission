# Fission [![License](https://img.shields.io/github/license/lazergenixdev/Fission?color=dodgerblue&style=plastic)](https://github.com/lazergenixdev/Fission/blob/master/LICENSE)
### High performance game engine
Game Engine I am developing on for a specific game that I am working on, but thought that some others could also find some use in this project :)
### Why this game engine ?
I developing this engine to fully take advantage of the hardware on most modern machines to make a competitively **Fast**, **Efficient**, and **Stable** engine with no compromises. To do this I will be using the native toolsets for each platform and implement most graphics APIs; all abstracted away from the user of *Fission* to make game development easier than coding everything from scratch, but with all of the benefits.
### Development
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
- Basic Sound support (mp3,wav,...)

# RoadMap
### *Fission-v0.5.0*
- Implement more window functionality
- Add Child Windows (preparation for UI system)
- Add scene system
- Add loading screen functionality (fission logo?)
### *Fission-v0.6.0*
- Add general purpose UI system
### *Fission-v0.6.1*
- Add opengl support
## Future releases
- new platforms will be implemented after the official version 1.0 is released and all base features are available
