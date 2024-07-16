
#version 460

vec3 Positions[6] = {
	vec3(-0.3f, -0.5f, 0.f),
	vec3( 0.3f, -0.5f, 0.f),
	vec3(-0.5f,  0.5f, 0.f),
	vec3(-0.5f,  0.5f, 0.f),
	vec3( 0.3f, -0.5f, 0.f),
	vec3( 0.5f,  0.5f, 0.f),
};

void main()
{
	if (gl_VertexIndex < 6) {
		gl_Position = vec4(Positions[gl_VertexIndex], 1.0f);
	} else {
		gl_Position = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}
}
