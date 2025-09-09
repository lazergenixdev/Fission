# Fission
todo
# Building 🏗️
## Prerequisites
The build program is built with the Clang C compiler, make sure to have clang installed.
You can check that you have clang installe🏗️d by running `clang` in your terminal.
## Windows

In order to compile code for Windows, you will need to have Visual Studio installed.
- [Install Visual Studio](https://visualstudio.microsoft.com/downloads/)

To install Clang:
 - [Install LLVM](https://github.com/llvm/llvm-project/releases) (installer will start with "LLVM-")
 - You may need to restart terminal after installation

Once you have clang installed then you can run `tools/setup-windows.bat` (or simply compile the `build.c` yourself)

## MacOS

You should already have the Apple Clang Compiler installed with Xcode.
- [Install Xcode](https://apps.apple.com/us/app/xcode/id497799835?mt=12)

Then you can run `tools/setup-macos.sh` (or simply compile the `build.c` yourself)

## How to use build program
Then to build the project, simply run the build program from the project directory.
```bash
./build
```
Additionally, you can pass options to the build program:
 - `all`: Compile all example programs along with Fission
 - `debug`: Compile with debug symbols

## Binaries 📦
After a build, Fission and all other libraries you will need to link you application with will be in `bin/[target_os]/`.
Additionally, all the includes you will need will be in `include/`.