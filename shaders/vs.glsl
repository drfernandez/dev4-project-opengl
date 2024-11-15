#version 420 // GLSL 4.20

struct OBJ_ATTRIBUTES
{
	vec3		Kd; // diffuse reflectivity
	float		d; // dissolve (transparency) 
	vec3		Ks; // specular reflectivity
	float		Ns; // specular exponent
	vec3		Ka; // ambient reflectivity
	float		sharpness; // local reflection map sharpness
	vec3		Tf; // transmission filter
	float		Ni; // optical density (index of refraction)
	vec3		Ke; // emissive reflectivity
	uint		illum; // illumination model
};

struct LIGHT
{
	vec4 position;
	vec4 color;
	vec4 direction;
	vec4 attribs;
};

layout (std140, row_major) uniform;

layout (binding = 0) uniform MESH_DATA
{
	mat4 world_matrix;
	OBJ_ATTRIBUTES material;
};

layout (binding = 1) uniform SCENE_DATA
{
	mat4 view_matrix;
	mat4 projection_matrix;
	vec4 camera_pos;
};

layout (binding = 2) uniform LIGHT_DATA
{
	LIGHT light_list[100];
};

layout (location = 0) in vec3 in_pos; 
layout (location = 1) in vec3 in_uvw; 
layout (location = 2) in vec3 in_nrm; 

out vec3 fs_pos;
out vec2 fs_uv;
out vec3 fs_nrm;

void main()
{
	vec4 p = vec4(in_pos, 1) * world_matrix;
	vec4 n = vec4(in_nrm, 0) * world_matrix;
	vec4 posH = p * view_matrix * projection_matrix;
	fs_pos = p.xyz;
	fs_uv = in_uvw.xy;
	fs_nrm = n.xyz;
	gl_Position = posH; 
}