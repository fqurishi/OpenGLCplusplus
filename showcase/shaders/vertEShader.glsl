#version 430

layout (location=0) in vec4 vertPos;
layout (location=1) in vec4 vertNormal;
layout (location=2) in vec2 tCoord;

out vec2 tc;
out vec3 varyingNormal, varyingLightDir, varyingHalfVector, varyingVertPos; 
out vec4 shadow_coord;

struct PositionalLight
{	vec4 ambient, diffuse, specular;
	vec3 position;
};
struct Material
{	vec4 ambient, diffuse, specular;   
	float shininess;
};

uniform vec4 globalAmbient;
uniform PositionalLight light;
uniform Material material;
uniform mat4 mv_matrix;
uniform mat4 proj_matrix;
uniform mat4 norm_matrix;
uniform mat4 shadowMVP;
layout (binding=0) uniform sampler2DShadow shadowTex;
layout (binding=1) uniform sampler2D s;
layout (binding=4) uniform samplerCube t;

void main(void)
{	
	varyingVertPos = (mv_matrix * vertPos).xyz;
        
	//get a vector from the vertex to the light and output it to the rasterizer for interpolation
	varyingLightDir = light.position - varyingVertPos;

	//get a vertex normal vector in eye space and output it to the rasterizer for interpolation
	varyingNormal = (norm_matrix * vertNormal).xyz;
	
	// calculate the half vector (L+V)
	varyingHalfVector = (varyingLightDir-varyingVertPos).xyz;
	
	shadow_coord = shadowMVP * vertPos;
	
	gl_Position = proj_matrix * mv_matrix * vertPos;
	tc = tCoord;
}
