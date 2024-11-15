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

in vec3 fs_pos;
in vec2 fs_uv;
in vec3 fs_nrm;

out vec4 out_color;

void main() 
{	
	OBJ_ATTRIBUTES m = material;

	float lightPower = light_list[0].attribs.x;
	vec3 normal = normalize(fs_nrm);
	vec3 lightAmbient = vec3(0.25f, 0.25f, 0.35f);
	vec3 lightColor = light_list[0].color.xyz;
	vec3 lightDirection = normalize(light_list[0].direction.xyz);
	float lightRatio = clamp(dot(-lightDirection, normal), 0.0f, 1.0f);
	vec3 lightResult = clamp(lightRatio * lightColor, 0.0f, 1.0f) * lightPower;
	out_color = vec4(m.Kd * (lightResult + lightAmbient), m.d);

	//vec3 directLight = dot(-normalize(sun_direction.xyz), normalize(fs_nrm)) * sun_color.xyz;
	//vec3 ambientLight = m.Kd * ambient_light.xyz;
	
	//// specular
	//vec3 toCam = normalize(camera_pos.xyz - fs_pos);
	//vec3 toLight = -normalize(sun_direction.xyz);
	//vec3 reflect_vec = normalize(reflect(-toLight, normalize(fs_nrm)));
	//vec3 specular = pow(max(dot(toCam, reflect_vec), 0.0f), m.Ns + 0.000001f) * m.Ks;
	//// end specular

	//vec3 result = clamp(directLight + ambientLight, 0.0f, 1.0f);
	//gl_FragColor = vec4(result * m.Kd + specular, m.d); 
}