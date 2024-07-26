
#version 460

layout(location = 0) out vec4 FragColor;
layout(location = 1) out uvec4 FragCustomStencil;

void main()
{
	FragColor = vec4(0.0f, 1.0f, 0.4999f, 1.0f) ;
	FragCustomStencil.r = 1;
}
