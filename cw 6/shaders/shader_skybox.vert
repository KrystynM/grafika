layout (location = 0) in vec3 vertexPosition;

out vec3 TexCoords;

uniform mat4 modelViewProjectionMatrix;

void main(){
	TexCoords = vertexPosition;
	gl_Position = modelViewProjectionMatrix * vec4(vertexPosition, 1.0);
}