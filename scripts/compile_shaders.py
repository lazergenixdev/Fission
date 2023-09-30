#################################################################
# USAGE: python compile_shaders.py path/to/search
#
# Script that recursivly searches for directories named 'shaders'
#     and compiles all '.frag', '.vert' files to c++ .inl files
#################################################################
import sys, os

def main():
	SEARCH_PATH = sys.argv[1]
	VULKAN_SDK  = os.getenv("VULKAN_SDK")
	COMPILER    = os.path.join(VULKAN_SDK, "Bin", "glslc")
	FILE_TO_CPP = os.path.join(".", "file_to_cpp")

	if os.name == "nt":
		COMPILER    += ".exe"
		FILE_TO_CPP += ".exe"

	def has_shaders_name(folders: list[str]):
		for folder in folders:
			if folder == "shaders":
				return True
		return False

	def is_shader_ext(ext: str):
		if   ext == ".frag":
			return True
		elif ext == ".vert":
			return True
		return False

	# find all shaders/ folders
	shaders = {}
	for root, folders, files in os.walk(SEARCH_PATH):
		if has_shaders_name(root.split('\\')):
			print(f"===== [{root}] =====")
			for f in files:
				ext = os.path.splitext(f)[1]
				if is_shader_ext(ext):
					shader_file_path = os.path.join(root, f);
					print(f)
					r = os.system(f"{COMPILER} {shader_file_path}")
					if  r == 0:
						os.system(f"{FILE_TO_CPP} a.spv {shader_file_path}.inl")
					else:
						print(f"error: shader compilation failed with code {r}")

	try:
		os.remove("a.spv")
	except Exception:
		pass

if __name__ == "__main__":
    main()
