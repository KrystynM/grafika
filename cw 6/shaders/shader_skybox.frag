uniform samplerCube skybox;
in vec3 TexCoords;

void main() {
	gl_FragColor = texture(skybox, TexCoords);
}