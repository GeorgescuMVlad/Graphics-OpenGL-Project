#version 410 core

in vec3 normal;
in vec4 fragPosEye;
in vec2 fragTexCoords;

out vec4 fColor;

//lighting
in vec4 fragPos;
in vec4 fragPosLightSpace;
uniform	mat3 normalMatrix;
uniform mat3 lightDirMatrix;
uniform	vec3 lightColor;
uniform	vec3 lightDir;
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 64.0f;

vec3 o;

//partea cu lumina
struct SpotLight
		{
			vec3 position;
			vec3 direction;
			float cutOff;
			float outerCutOff;
	
			vec3 ambient;
			vec3 diffuse;
			vec3 specular;
		};

uniform SpotLight felinar;

vec3 computeSpotLight()
		{
			float theta = dot(normalize(felinar.position - fragPos.xyz), normalize(-felinar.direction));
			float epsilon =	felinar.cutOff - felinar.outerCutOff;
			float intensity = clamp((theta - felinar.outerCutOff) / epsilon, 0.0, 1.0) / 9.0;
			vec3 amb = felinar.ambient;
			vec3 diff = felinar.diffuse * intensity;
			vec3 specular = felinar.specular * intensity;
			return amb + diff + specular;
		}


void computeLightComponents()
{		
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(normalMatrix * normal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightDirMatrix * lightDir);	

	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fragPosEye.xyz);
	
	//compute half vector
	vec3 halfVector = normalize(lightDirN + viewDirN);
		
	//compute ambient light
	ambient = ambientStrength * lightColor;
	
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	//compute specular light
	float specCoeff = pow(max(dot(halfVector, normalEye), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;
}

float computeShadow()
{		
	// perform perspective divide
    vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    if(normalizedCoords.z > 1.0f)
        return 0.0f;
    // Transform to [0,1] range
    normalizedCoords = normalizedCoords * 0.5f + 0.5f;
    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;    
    // Get depth of current fragment from light's perspective
    float currentDepth = normalizedCoords.z;
    // Check whether current frag pos is in shadow
    float bias = 0.005f;
    float shadow = currentDepth - bias> closestDepth  ? 1.0f : 0.0f;

    return shadow;	
}

float computeFog()
{
 float fogDensity = 0.0075f;
 float fragmentDistance = length(fragPosEye);
 float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

 return clamp(fogFactor, 0.0f, 1.0f);
}

void main() 
{
	computeLightComponents();
	
	float shadow = computeShadow();
	
	//modulate with diffuse map
	ambient *= 2*vec3(texture(diffuseTexture, fragTexCoords));
	diffuse *= 2*vec3(texture(diffuseTexture, fragTexCoords));
	//modulate woth specular map
	specular *= 2*vec3(texture(specularTexture, fragTexCoords));
	
	//modulate with shadow
	vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular, 1.0f);
	color += computeSpotLight();
    
    //fColor = vec4(color, 1.0f);
    
    float fogFactor = computeFog();
	vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
	vec4 myCol = vec4(color, 1.0f);
	fColor = mix(fogColor, myCol, fogFactor);
	
	//vec4 colorFromTexture = texture(diffuseTexture, interpolatedTexCoords);
	//if(colorFromTexture.a < 0.1)
	//	discard;
	//fColor = colorFromTexture;

}