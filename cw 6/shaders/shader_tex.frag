#version 410 core

uniform sampler2D textureSampler;
uniform vec3 lightDir;
uniform vec3 skyColor;

in vec3 interpNormal;
in vec2 interpTexCoord;
in float visibility;

void main()
{
	vec2 modifiedTexCoord = vec2(interpTexCoord.x, 1.0 - interpTexCoord.y); // Poprawka dla tekstur Ziemi, ktore bez tego wyswietlaja sie 'do gory nogami'
	vec4 rgba = texture2D(textureSampler, modifiedTexCoord).rgba;
	if(rgba.a < 0.1) discard;
	vec3 normal = normalize(interpNormal);
	float diffuse = max(dot(normal, -lightDir), 0.0);
	gl_FragColor = rgba * diffuse;
	gl_FragColor = mix(vec4(skyColor, 1.0), gl_FragColor, visibility);
}
