#version 430

in vec3 vertEyeSpacePos;
in vec2 tc;
in vec3 varyingNormal, varyingLightDir, varyingVertPos, varyingHalfVec;
in vec3 varyingTangent;
in vec4 shadow_coord;
out vec4 fragColor;
 
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
layout (binding=6) uniform sampler2D n; // for normal map

vec3 CalcBumpedNormal()
{
	vec3 Normal = normalize(varyingNormal);
	vec3 Tangent = normalize(varyingTangent);
	Tangent = normalize(Tangent - dot(Tangent, Normal) * Normal);
	vec3 Bitangent = cross(Tangent, Normal);
	vec3 BumpMapNormal = texture(n,tc).xyz;
	BumpMapNormal = BumpMapNormal * 2.0 - 1.0;
	mat3 TBN = mat3(Tangent, Bitangent, Normal);
	vec3 NewNormal = TBN * BumpMapNormal;
	NewNormal = normalize(NewNormal);
	return NewNormal;
}

float lookup(float x, float y)
{  	float t = textureProj(shadowTex, shadow_coord + vec4(x * 0.001 * shadow_coord.w,
                                                         y * 0.001 * shadow_coord.w,
                                                         -0.01, 0.0));
	return t;
}



void main(void)
{	float shadowFactor=0.0;
	vec3 L = normalize(varyingLightDir);
	vec3 N = CalcBumpedNormal();
	vec3 V = normalize(-varyingVertPos);
	vec3 H = normalize(varyingHalfVec);
	vec4 textureColor = texture(s, tc);
	float dist = length(vertEyeSpacePos.xyz);
	float swidth = 2.5;
	vec2 o = mod(floor(gl_FragCoord.xy), 2.0) * swidth;
	shadowFactor += lookup(-1.5*swidth + o.x,  1.5*swidth - o.y);
	shadowFactor += lookup(-1.5*swidth + o.x, -0.5*swidth - o.y);
	shadowFactor += lookup( 0.5*swidth + o.x,  1.5*swidth - o.y);
	shadowFactor += lookup( 0.5*swidth + o.x, -0.5*swidth - o.y);
	shadowFactor = shadowFactor / 4.0;
	
	// get the angle between the light and surface normal:
	float cosTheta = dot(L,N);
	
	// compute light reflection vector, with respect N:
	vec3 R = normalize(reflect(-L, N));
	
	// angle between the view vector and reflected light:
	float cosPhi = dot(V,R);


	vec4 shadowColor = globalAmbient * material.ambient
				+ light.ambient * material.ambient;
	
	vec4 lightedColor = light.diffuse * material.diffuse * max(cosTheta,0.0)
				+ light.specular * material.specular
				* pow(max(cosPhi,0.0),material.shininess);


	fragColor = 0.5 * texture(s,tc)
		+
		0.5 * (fragColor = vec4((shadowColor.xyz + shadowFactor*(lightedColor.xyz)),1.0));


	
}