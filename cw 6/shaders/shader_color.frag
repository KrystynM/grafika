#version 410 core

uniform vec3 objectColor;
uniform vec3 lightDir;
uniform vec3 skyColor;

in vec3 interpNormal;
in float visibility;


void main()
{
	vec3 normal = normalize(interpNormal);
	float diffuse = max(dot(normal, -lightDir), 0.0);
	gl_FragColor = vec4(objectColor * diffuse, 1.0);
	gl_FragColor = mix(vec4(skyColor, 1.0), gl_FragColor, visibility);
}
