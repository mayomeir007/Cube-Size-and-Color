#version 460

in vec4 colorOut;
out vec4 fragColor;
in vec2 textureOut;

uniform bool isTextured;
uniform sampler2D textureImage;

void main()
{
	if(isTextured)
	{
		fragColor = texture(textureImage, textureOut);
	}

	else
	{
		fragColor = colorOut;
	}
}