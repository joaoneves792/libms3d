#version 450

#define MAX_LIGHTS 10

in vec3 position_worldspace;
in vec2 texture_coord_from_vshader;
in vec3 normal_cameraspace;
in vec3 eyeDirection_cameraspace;
in vec3 lightDirection_cameraspace[MAX_LIGHTS];

out vec4 out_color;

uniform sampler2D texture_sampler;
uniform mat4 Model;
uniform mat4 View;
uniform vec3 lightPosition_worldspace[MAX_LIGHTS];
uniform int lightsEnabled[MAX_LIGHTS];

uniform vec4 ambient;
uniform vec4 diffuse;
uniform vec4 specular;
uniform vec4 emissive;

uniform float shininess;
uniform float transparency;

void main() {
	//For now these are constant
	vec3 lightColor = vec3(1, 1, 1);
	float lightPower = 9000.0f;

	//Material properties
	vec3 diffuseCoef = diffuse.xyz / vec3(255, 255, 255);
	vec3 matDiffuse = (texture(texture_sampler, texture_coord_from_vshader).rgb * diffuse.xyz);
	vec3 ambientCoef = ambient.xyz / vec3(255, 255, 255);
	vec3 matAmbient = ambientCoef * matDiffuse;
	vec3 matSpecular = specular.xyz;

	out_color.rgb = vec3(0,0,0);//Very important! start with black

	int i;
	for(i=0; i<MAX_LIGHTS; i++){
		if(lightsEnabled[i] == 1){
			float distance_to_light = length( lightPosition_worldspace[i].xyz - position_worldspace );

			vec3 n = normalize( normal_cameraspace );
			vec3 l = normalize( lightDirection_cameraspace[i] );
			float cosTheta = clamp( dot(n, l), 0, 1);

			vec3 E = normalize(eyeDirection_cameraspace);
			vec3 R = reflect(-l, n);
			float cosAlpha = clamp( dot(E,R), 0,1);

			out_color.rgb += matAmbient + 
				matDiffuse * lightColor * lightPower * cosTheta / (distance_to_light*distance_to_light) +
				matSpecular * (shininess/4.0) * lightColor * lightPower * pow(cosAlpha, 5) / (distance_to_light*distance_to_light)+
				emissive.xyz;
		}
		
	}
	
	out_color.a = texture(texture_sampler, texture_coord_from_vshader).a - transparency;
}
