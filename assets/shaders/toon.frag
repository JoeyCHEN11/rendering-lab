#version 330 core
out vec4 FragColor;

// object materials (colors)
struct Material
{
	sampler2D texture_diffuse1;
	sampler2D texture_diffuse2;
	sampler2D texture_diffuse3;
	sampler2D texture_diffuse4;
	sampler2D texture_specular1;
	sampler2D texture_specular2;

	float shininess;
};

// lights
struct DirLight{
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

struct SpotLight{
	vec3 position;
	vec3 direction;
	float cutOff; // cosine of cutoff angle
	float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

const int NR_DIRECT_LIGHTS = 4;
const int NR_POINT_LIGHTS = 4;
const int NR_SPOT_LIGHTS = 4;

uniform vec3 viewPos;
uniform int directLightCount;
uniform DirLight dirLight[NR_DIRECT_LIGHTS];
uniform int pointLightCount;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform int spotLightCount;
uniform SpotLight spotLight[NR_SPOT_LIGHTS];
uniform Material material;
uniform float toonThreshold;
uniform float toonSmoothness;


vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
	vec3 result = vec3(0.0);
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 norm = normalize(Normal);

	for (int i = 0; i < directLightCount; i++){
		result += CalcDirLight(dirLight[i], norm, viewDir); 
	}

	for (int i = 0; i < pointLightCount; i++)
	{
		result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
	}
	
	for (int i = 0; i < spotLightCount; i++){
		result += CalcSpotLight(spotLight[i], norm, FragPos, viewDir);
	}

	FragColor = vec4(result, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
	vec3 diffColor = texture(material.texture_diffuse1, TexCoords).rgb;
	vec3 specColor = texture(material.texture_specular1, TexCoords).rgb;

	vec3 lightDir = normalize(-light.direction);
	vec3 reflectDir = reflect(-lightDir, normal);
	float diff = max(dot(normal, lightDir), 0.0);
	float spec = pow(max(dot(reflectDir, viewDir), 0.0), material.shininess);

	// ambient
	vec3 ambient = light.ambient * diffColor;

	// diffuse
	// blend light-dark edge
	float diffIntensity = smoothstep(toonThreshold - toonSmoothness, toonThreshold + toonSmoothness, diff);
	vec3 diffuse = light.diffuse * diffColor * diffIntensity;

	// specular
	//float specIntensity = smoothstep(0.005, 0.01, spec);
	//vec3 specular = light.specular * specColor * specIntensity ;

	return ambient + diffuse; // + specular

}
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 diffColor = texture(material.texture_diffuse1, TexCoords).rgb;
	vec3 specColor = texture(material.texture_specular1, TexCoords).rgb;
	vec3 lightDir = normalize(light.position - fragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 reflectDir = reflect(-lightDir, normal);
	//float spec = pow(max(dot(reflectDir, viewDir), 0.0), material.shininess);

	// attenuation
	float dist = distance(light.position, fragPos);
	float attenuation = 1.0 / (light.constant + 
							   light.linear * dist + 
							   light.quadratic * (dist * dist));

	// ambient
	vec3 ambient = light.ambient * diffColor;

	// diffuse
	float diffIntensity = smoothstep(toonThreshold - toonSmoothness, toonThreshold + toonSmoothness, diff);
	vec3 diffuse = light.diffuse * diffIntensity * diffColor;

	// specular
	//float specIntensity = smoothstep(0.005, 0.01, spec);
	//vec3 specular = light.specular * specIntensity * specColor;

	ambient  *= attenuation; 
	diffuse  *= attenuation;
	//specular *= attenuation;

	return ambient + diffuse; // + specular

}
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - fragPos);
	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

	// attenuation
	float dist = distance(light.position, fragPos);
	float attenuation = 1.0 / (light.constant + 
							   light.linear * dist + 
							   light.quadratic * (dist * dist));

	vec3 diffColor = texture(material.texture_diffuse1, TexCoords).rgb;
	vec3 specColor = texture(material.texture_specular1, TexCoords).rgb;
	vec3 reflectDir = reflect(-lightDir, normal);

	float diff = max(dot(normal, lightDir), 0.0);
	float spec = pow(max(dot(reflectDir, viewDir), 0.0), material.shininess);

	// ambient
	vec3 ambient = light.ambient * diffColor;

	// diffuse
	float diffIntensity = smoothstep(toonThreshold - toonSmoothness, toonThreshold + toonSmoothness, diff);
	vec3 diffuse = light.diffuse * diffIntensity * diffColor;

	// specular
	//float specIntensity = smoothstep(0.005, 0.01, spec);
	//vec3 specular = light.specular * specIntensity * specColor;

	ambient *= intensity * attenuation;
	diffuse  *= intensity * attenuation;
	//specular *= intensity * attenuation;

	return ambient + diffuse; //specular

}