import sys, os

COLOR_RED = "\033[1;31m"
COLOR_RESET = "\033[0m"

def output(X): print(X, end="")

shader_compiler = "C:\\VulkanSDK\\1.3.250.1\\Bin\\glslc.exe"
shader_options  = "-O"

def panic(failure_reason: str):
	print(failure_reason)
	exit(1)

def compile(shader: str):
	output(COLOR_RED)
	r = os.system(f"{shader_compiler} {shader_options} {shader}")

	if r != 0:
		panic(f"error: shader compilation failed with code {r}")
	else:
		output(COLOR_RESET)
		print(f"compiled shader: \"{shader}\"")
	
	r = os.system(f".\\file_to_cpp.exe a.spv {shader}.inl")

	if r != 0:
		panic(f"error: file_to_cpp failed to generate .inl file [exit_code={r}]")
	else:
		print(f"generated shader include: \"{shader}.inl\"")

	return False

def main():
	for v in sys.argv[1:]:
		compile(v)
	
	os.remove("a.spv")

if __name__ == "__main__":
    main()
