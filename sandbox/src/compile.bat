::C:\VulkanSDK\1.3.250.1\Bin\glslc.exe "solid_color.vert" -o "solid_color.vert.spv" -O
::C:\VulkanSDK\1.3.250.1\Bin\glslc.exe "solid_color.frag" -o "solid_color.frag.spv" -O
::C:\VulkanSDK\1.3.250.1\Bin\glslc.exe "textured_2d.vert" -o "textured_2d.vert.spv" -O
::C:\VulkanSDK\1.3.250.1\Bin\glslc.exe "textured_2d.frag" -o "textured_2d.frag.spv" -O
C:\VulkanSDK\1.3.250.1\Bin\glslc.exe "3d.vert" -o "3d.vert.spv" -O
C:\VulkanSDK\1.3.250.1\Bin\glslc.exe "3d.frag" -o "3d.frag.spv" -O

::file_to_cpp.exe solid_color.vert.spv solid_color.vert.inl
::file_to_cpp.exe solid_color.frag.spv solid_color.frag.inl
::file_to_cpp.exe textured_2d.vert.spv textured_2d.vert.inl
::file_to_cpp.exe textured_2d.frag.spv textured_2d.frag.inl
file_to_cpp.exe 3d.vert.spv 3d.vert.inl
file_to_cpp.exe 3d.frag.spv 3d.frag.inl
