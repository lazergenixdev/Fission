# Fission [![License](https://img.shields.io/github/license/lazergenixdev/Fission?color=dodgerblue&style=plastic)](https://github.com/lazergenixdev/Fission/blob/master/LICENSE) [![GitHub commits since latest release (by date including pre-releases)](https://img.shields.io/github/commits-since/lazergenixdev/Fission/latest/master?include_prereleases&style=plastic)](https://github.com/lazergenixdev/Fission/commits/master)
*Fission* is an open source cross-platform game engine designed for **high efficiency** and **maximum performance**.

## Features ✨
- Native Win32 support
- DirectX11 support
- Included 2D renderer with support for fonts
- Basic Sound support (XAudio2 + Microsoft Media Foundation for loading sounds)
- Runtime Texture Atlas creation utility

# RoadMap 🗓
Version 0.7 will have a complete focus on UI, both for developer debugging and for general use.

### *Fission v0.7.0-alpha.1* (week 28 2021)
### *Fission v0.7.0-alpha.2* (week 29 2021)
### *Fission v0.7.0* (week 30 2021)
- Functional implementation of debug UI
  - windows
  - sliders
  - buttons
  - combo boxes
- [neutron.hpp](https://github.com/lazergenixdev/Fission/blob/master/include/Fission/neutron.hpp), with all features implemented
- user-defined UI Layers

### Planned Features ⛅
- SPIR-V for shader reflection
- OpenGL support
- Better Input System (binding actions to key strokes)
- External and Docking windows for UI

# Development 🛠
Only focusing on the Windows build for now, I am hoping to have Andriod as the next platform supported.

# Getting Started 🚀
```sh
git clone --recursive https://github.com/lazergenixdev/Fission
```
To make a Visual Studio Solution run `GenWindowsProj.bat` in the `scripts` directory
