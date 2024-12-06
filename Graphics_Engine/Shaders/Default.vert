#version 460

in vec3 vertexIn;
in vec4 colorIn;
in vec2 textureIn;

out vec2 textureOut;

out vec4 colorOut;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
	colorOut = colorIn;
	textureOut = textureIn;
	gl_Position = proj * view * model * vec4(vertexIn, 1.0);
}