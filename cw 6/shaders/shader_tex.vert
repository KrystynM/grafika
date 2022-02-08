#version 410 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;

layout(location = 2) in vec2 vertexTexCoord;
uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelMatrix;
uniform mat4 perspectiveMatrix;
uniform float fogDensity;
uniform float fogGradient;



out vec3 interpNormal;
out vec2 interpTexCoord;
out float visibility;

void main()
{
	vec4 worldPosition = modelViewProjectionMatrix * vec4(vertexPosition, 1.0);

	gl_Position = modelViewProjectionMatrix * vec4(vertexPosition, 1.0);
	interpNormal = (modelMatrix * vec4(vertexNormal, 0.0)).xyz;
	interpTexCoord = vertexTexCoord;
	vec4 positionRelativeToCamera = perspectiveMatrix * worldPosition;
	float distanceFromCamera = length(positionRelativeToCamera.xyz);
	visibility = exp(-pow((distanceFromCamera * fogDensity), fogGradient));
	visibility = clamp(visibility, 0.0, 1.0);

}
