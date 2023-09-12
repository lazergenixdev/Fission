# Fission [![License](https://img.shields.io/github/license/lazergenixdev/Fission?color=dodgerblue&style=plastic)](https://github.com/lazergenixdev/Fission/blob/master/LICENSE) [![GitHub commits since latest release (by date including pre-releases)](https://img.shields.io/github/commits-since/lazergenixdev/Fission/latest/master?include_prereleases&style=plastic)](https://github.com/lazergenixdev/Fission/commits/master)
*Fission* is an open source "cross-platform" game engine designed for a game that is still in development :)

# Building from Source
| Platform | Is Supported ? |
| --- | --- |
| Windows | ✔ Yes |
| Linux | ❌ WIP |
| Andriod | ❌ No |
| MacOS | ❌ No |
| IOS | ❌ No |

note: There will very likely not be another platform supported until my game is mostly done

## 0. Prerequisite programs/libs to have already installed
| Program/Lib | Platform | Why |
| --- | --- | --- |
| [git](https://git-scm.com/) | all | for source version control |
| [python](https://python.org/) | all | for compiling shaders (optional) |
| [premake](https://premake.github.io/) | all | for generating the build configuration (optional^) |
| Vulkan SDK | all | Graphics API |
| Visual Studio | Windows | dont know |

^ *premake binary is included in `vendor/`*
## 1. Clone the repository 
```sh
git clone https://github.com/lazergenixdev/Fission
```
## 2. Generate Build Configuration
### Windows
To make a Visual Studio Solution, run command:
```sh
premake5 vs2022
```
or run `setup_windows.bat` in `scripts/` to use the included premake binary
### Linux (WIP)

# Features 🌟
(WIP)
# RoadMap 🗓
(WIP)
# Development 🛠
(WIP)