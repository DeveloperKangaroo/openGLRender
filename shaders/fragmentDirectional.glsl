#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture1;
uniform sampler2D texture2;

uniform vec3 viewPos;

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

uniform Material material;

struct Light {
	vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Light light;  

void main()
{
	vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoord)).rgb;

	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(-light.direction);
	
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoord)).rgb;

	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0f);
	vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoord));

	FragColor = vec4(ambient + diffuse + specular, 1.0);

      // FragColor = mix(texture(texture1, TexCoord), texture(texture2,TexCoord), 0.2);
}  