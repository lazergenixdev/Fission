# Fission [![License](https://img.shields.io/github/license/lazergenixdev/Fission?color=dodgerblue&style=plastic)](https://github.com/lazergenixdev/Fission/blob/master/LICENSE) [![GitHub commits since latest release (by date including pre-releases)](https://img.shields.io/github/commits-since/lazergenixdev/Fission/latest/master?include_prereleases&style=plastic)](https://github.com/lazergenixdev/Fission/commits/master)
***Fission*** is an open source cross-platform game framework designed for a game that is still in development :)

## Platforms Supported
- Windows `x86_64`
- MacOS `arm64`
- Android `armv7` `arm64`
- ~~Linux~~ (planned)

# Building 🏗️
## Prerequisites
- `Windows` [Visual Studio](https://visualstudio.microsoft.com/downloads/),
            [clang](https://github.com/llvm/llvm-project/releases),
            [cmake](https://cmake.org/download/),
            [Ninja](https://ninja-build.org/) (Android only)
- `MacOS` [Xcode](https://apps.apple.com/us/app/xcode/id497799835), cmake

The build program is built with the Clang C compiler, and requires cmake to build it's dependencies.
You can check that you have clang installed by running `clang --version` in your terminal and making sure you have a somewhat recent version.

To build the build program, simply run one of the setup scripts provided (it will look like `tools/setup-[platform]`).
## How to use build program
Once you have the build program built, to build the project, simply run the build program from the project directory.
```bash
./build
```
Additionally, you can pass options to the build program:
 - `all`: Compile all example programs along with Fission
 - `debug`: Compile with debug symbols
 - `android`: Set target OS to Android

## Binaries 📦
After a build, Fission and all other libraries you will need to link your application with, will be in `bin/[target_os]/`.
Additionally, all the includes you will need will be in `include/`.
