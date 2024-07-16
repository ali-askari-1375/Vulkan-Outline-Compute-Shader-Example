
#version 460

layout(location = 0) out vec4 FragColor;
layout(location = 1) out uint FragCustomStencil;

void main()
{
	FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f) ;
	FragCustomStencil = 45;
}
