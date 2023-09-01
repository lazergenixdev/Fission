// https://www.codebooklet.com/2022/08/13/ComputerGraphics/Fullscreen-Triangle/
// Vulkan GLSL vertex shader << vkCmdDraw(_,_,3,1,0,0);
#version 450

const vec2[3] full_screen_triangle = vec2[3]
(
    vec2(3.0, 1.0),
    vec2(-1.0, 1.0),
    vec2(-1.0, -3.0)
);

layout (location = 0) out vec2 uv;

void main()
{
	uv = (full_screen_triangle[gl_VertexIndex] + vec2(1.0)) * vec2(0.5);
   gl_Position = vec4(full_screen_triangle[gl_VertexIndex], 0.0, 1.0);
}