# Fission [![License](https://img.shields.io/github/license/lazergenixdev/Fission?color=dodgerblue&style=plastic)](https://github.com/lazergenixdev/Fission/blob/master/LICENSE) [![GitHub commits since latest release (by date including pre-releases)](https://img.shields.io/github/commits-since/lazergenixdev/Fission/latest/master?include_prereleases&style=plastic)](https://github.com/lazergenixdev/Fission/commits/master)
*Fission* is an open source cross-platform game engine designed for **high efficiency** and **maximum performance**.

## Features ✨
- Native Win32 support
- DirectX11 support
- Included 2D renderer with support for fonts
- Basic Sound support (XAudio2 + Microsoft Media Foundation for loading sounds)
- Runtime Texture Atlas creation utility

# RoadMap 🗓
### *Fission-v0.6.0* (week 24 2021)
- update Engine architecture to be more self-contained (+Fission::FEngine)
- Bring back ConstantBuffers as a Graphics Bindable
- Update misleading Fission::System API
### *Fission-v0.6.1* (week 25 2021)
- Fully implement the use of Monitors
- Make it easy to switch resolutions and fullscreen mode at runtime
### *Fission-v0.6.2* (week 27 2021)
- Better console layer (cursor + selection + copy/paste + history)
### *Fission-v0.6.3* (week 28 2021)
- Rewrite of the debug UI
- Dedicated UI Layer?

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
