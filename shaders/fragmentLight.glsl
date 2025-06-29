#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 viewPos;

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

uniform Material material;

struct DirLight {
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform DirLight dirLight;

struct PointLight {
	vec3 position;

	float constant;
	float linear;
	float quadratic;

	vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	
};
#define NR_POINT_LIGHTS 4
uniform PointLight pointLights[NR_POINT_LIGHTS];

struct SpotLight {
	vec3 position;
	vec3 direction;
	float cutOff;
	float outerCutOff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform SpotLight spotLight;

vec3 CalcPhongLighting(vec3 lightDirection,vec3 normal, vec3 viewDir, vec3 ambient, vec3 diffuse, vec3 specular);
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);


void main()
{
	vec3 norm = normalize(Normal);
	vec3 viewDir = normalize(viewPos - FragPos);

	vec3 result = CalcDirLight(dirLight, norm, viewDir);

	for(int i = 0; i < NR_POINT_LIGHTS; i++)
		result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
	result += CalcSpotLight(spotLight, norm, FragPos, viewDir);

	FragColor = vec4(result, 1.0);
}  

vec3 ApplyPhong(vec3 lightDir, vec3 normal, vec3 viewDir, vec3 lightAmbient, vec3 lightDiffuse, vec3 lightSpecular, float attenuation, float intensity)
{
	lightDir = normalize(lightDir);
	vec3 reflectDir = reflect(-lightDir, normal);

	// Diffuse
	float diff = max(dot(normal, lightDir), 0.0);

	// Specular
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

	// Combine all
	vec3 ambient  = lightAmbient  * vec3(texture(material.diffuse, TexCoord));
	vec3 diffuse  = lightDiffuse  * diff * vec3(texture(material.diffuse, TexCoord));
	vec3 specular = lightSpecular * spec * vec3(texture(material.specular, TexCoord));

	ambient  *= attenuation;
	diffuse  *= attenuation * intensity;
	specular *= attenuation * intensity;

	return ambient + diffuse + specular;
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir){
	vec3 lightDir = -light.direction;
	return ApplyPhong(lightDir, normal, viewDir, light.ambient, light.diffuse, light.specular, 1.0, 1.0);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
	vec3 lightDir = light.position - fragPos;
	float distance = length(lightDir);
	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

	return ApplyPhong(lightDir, normal, viewDir, light.ambient, light.diffuse, light.specular, attenuation, 1.0);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir){
	vec3 lightDir = normalize(light.position - fragPos);

	//diffuse shading
	float diff = max(dot(normal, lightDir), 0.0);
	//specular shading
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);


	vec3 spotDir = normalize(-light.direction);
	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
	//attenuation
	float distance = length(lightDir);
	float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);


	return ApplyPhong(lightDir, normal, viewDir, light.ambient, light.diffuse, light.specular, attenuation, intensity);
	
}




	//ambient
//	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord)).rgb;
//
//	//diffuse
//	vec3 norm = normalize(Normal);
//	vec3 lightDir = normalize(light.position - FragPos);
//	float diff = max(dot(norm, lightDir), 0.0);
//	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoord)).rgb;
//
//	//specular	
//	vec3 viewDir = normalize(viewPos - FragPos);
//	vec3 reflectDir = reflect(-lightDir, norm);
//	float base = max(dot(viewDir, reflectDir), 0.0);
//	float spec = pow(base, max(material.shininess, 0.001));
//	vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoord));
//
//	//spotlight
//	float theta = dot(lightDir, normalize(-light.direction));
//	float epsilon = light.cutOff - light.outerCutOff;
//	float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
//	diffuse  *= intensity;
//	specular *= intensity;
//
//	//attenuation
//	float distance    = length(light.position - FragPos);
//	float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
//	ambient  *= attenuation;
//	diffuse  *= attenuation;
//	specular *= attenuation;
//
//	vec3 result = ambient + diffuse + specular;
//	FragColor = vec4(result, 1.0);