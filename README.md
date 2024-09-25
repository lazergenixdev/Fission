# Fission [![License](https://img.shields.io/github/license/lazergenixdev/Fission?color=dodgerblue&style=plastic)](https://github.com/lazergenixdev/Fission/blob/master/LICENSE) [![GitHub commits since latest release (by date including pre-releases)](https://img.shields.io/github/commits-since/lazergenixdev/Fission/latest/master?include_prereleases&style=plastic)](https://github.com/lazergenixdev/Fission/commits/master)
*Fission* is an open source "cross-platform" game engine designed for a game that is still in development :)

# Dependencies
Fission uses the following dependencies:
| | |
| --------------------------------------------------- | ----------------------- |
| [Vulkan](https://www.vulkan.org/)                   | Graphics API            |
| [VMA](https://gpuopen.com/vulkan-memory-allocator/) | Vulkan Memory Allocator |
| [GLM](https://github.com/g-truc/glm/)               | OpenGL Math Library     |
| [stb](https://github.com/nothings/stb)              | stb library             |
| [fmt](https://fmt.dev/)                             | C++ Formatting Library  |

Most of which are header only and do not need any setup.

# Building from Source
| Platform | Is Supported ?        |
| -------- | --------------------- |
| Windows  | ✔ Yes                 |
| Linux    | ✔ Yes (needs testing) |
| Andriod  | ✔ Yes                 |
| MacOS    | ❌ No time soon       |
| IOS      | ❌ No time soon       |

## 0. Prerequisite programs/libs to have already installed
| Program/Lib | Target Platform | Why |
| --- | --- | --- |
| [git](https://git-scm.com/) | all | for source version control |
| [python](https://python.org/) | all | for compiling shaders (optional) |
| [premake](https://premake.github.io/) | all | for generating the build configuration |
| Vulkan SDK | all | Graphics API |
| Visual Studio | Windows | recommended |
| Android Studio | Android | recommended (optional) |

## 1. Clone the repository 
```sh
git clone https://github.com/lazergenixdev/Fission
cd Fission
```
## 2. Generate Build Files
### Windows
```sh
premake5 vs2022
```
### Android
```sh
premake5 android-studio
```
### Linux
```sh
premake5 gmake2
```
## 3. Open build
```sh
cd build
```
# Features 🌟
(WIP)
# RoadMap 🗓
(WIP)
# Development 🛠
(WIP)
