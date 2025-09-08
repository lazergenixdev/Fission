# Building
## Windows
The build program builds with Clang C compiler, make sure to have clang installed.
 - [Install LLVM](https://github.com/llvm/llvm-project/releases) (installer will start with "LLVM-")

Check that you have clang installed by running `clang.exe` in your terminal (may need to restart terminal).
Once you have clang installed then you can run `tools/setup-windows.bat` (or simply compile the `build.c` yourself)

Then to build the project, run the build program from the project directory.
```bash
./build.exe
```

# Build Example Programs
By default the build program will only build the static library for Fission, so to build the example programs pass "all" to the build program:
```bash
./build all
```
