@echo off

:: Script may be run from tools directory ...
if not exist "build.c" (
	:: in that case go up one directory
	cd ../
)

clang.exe build.c -o build.exe
