#version 330 // GLSL 3.30

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

layout (std140, row_major) uniform MESH_DATA
{
	mat4 world_matrix;
	OBJ_ATTRIBUTES material;
};

layout (std140, row_major) uniform SCENE_DATA
{
	mat4 view_matrix;
	mat4 projection_matrix;
	vec4 camera_pos;
};

in vec3 fs_pos;
in vec2 fs_uv;
in vec3 fs_nrm;

void main() 
{	
	OBJ_ATTRIBUTES m = material;

	vec3 normal = normalize(fs_nrm);
	vec3 lightAmbient = vec3(0.2f, 0.2f, 0.3f);
	vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
	vec3 lightDirection = normalize(vec3(-1.0f, -1.0f, 1.0f));
	float lightRatio = clamp(dot(-lightDirection, normal), 0, 1);
	vec3 lightResult = lightRatio * lightColor;
	gl_FragColor = clamp(vec4(m.Kd * (lightResult + lightAmbient), m.d), 0, 1);

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