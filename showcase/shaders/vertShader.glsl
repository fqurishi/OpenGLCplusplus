#version 430

layout (location=0) in vec3 vertPos;
layout (location=1) in vec3 vertNormal;
layout (location=2) in vec2 tCoord;


out vec2 tc;
out vec3 vertEyeSpacePos;
out vec3 varyingNormal, varyingLightDir, varyingVertPos, varyingHalfVector;
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
layout (binding=5) uniform sampler2D h;	// for height map

void main(void)
{	
	// height-mapped vertex
	vec4 p = vec4(vertPos,1.0) + vec4((vertNormal*((texture2D(h,tCoord).r)/5.0f)),1.0f);

	//output the vertex position to the rasterizer for interpolation
	varyingVertPos = (mv_matrix * vec4(vertPos,1.0)).xyz;
        
	//get a vector from the vertex to the light and output it to the rasterizer for interpolation
	varyingLightDir = light.position - varyingVertPos;

	//get a vertex normal vector in eye space and output it to the rasterizer for interpolation
	varyingNormal = (norm_matrix * vec4(vertNormal,1.0)).xyz;
	
	// calculate the half vector (L+V)
	varyingHalfVector = (varyingLightDir-varyingVertPos).xyz;
	
	shadow_coord = shadowMVP * vec4(vertPos,1.0);
	
	// compute vertex position in eye space (without perspective)
	vertEyeSpacePos = (mv_matrix * p).xyz;
	gl_Position = proj_matrix * mv_matrix * vec4(vertPos,1.0);
	tc = tCoord;

}
