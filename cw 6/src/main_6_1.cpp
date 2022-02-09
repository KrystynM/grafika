#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <vector>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"
#include "Texture.h"

#include <iostream>
#include "SOIL/SOIL.h"
using namespace std;

GLuint programColor;
GLuint programTexture;
GLuint programSkybox;
GLuint textureCubemap;

obj::Model plantModel;
glm::mat4 plantModelMatrix;
GLuint plantTexture;

Core::Shader_Loader shaderLoader;

Core::RenderContext shipContext;
Core::RenderContext bottomContext;
Core::RenderContext fishContext;
Core::RenderContext fishContext2;
Core::RenderContext sharkContext;
Core::RenderContext duckContext;
Core::RenderContext bubbleContext;
Core::RenderContext plantContext;
Core::RenderContext coralContext;
Core::RenderContext kelpContext;

int g = 0;



glm::vec3 fishPositions[100];
float fishSpreadFactor = 100.0;




float old_x, old_y = -1;
float delta_x, delta_y = 0;
glm::quat rotationCamera = glm::quat(1, 0, 0, 0);
glm::quat rotation_y = glm::normalize(glm::angleAxis(209 * 0.03f, glm::vec3(1, 0, 0)));
glm::quat rotation_x = glm::normalize(glm::angleAxis(307 * 0.03f, glm::vec3(0, 1, 0)));
float dy = 0;
float dx = 0;

glm::vec3 cameraPos = glm::vec3(0, 460, 0);
glm::vec3 cameraDir; // Wektor "do przodu" kamery
glm::vec3 cameraSide; // Wektor "w bok" kamery
float cameraAngle = 0;

glm::mat4 cameraMatrix, perspectiveMatrix;

glm::vec3 lightDir = glm::normalize(glm::vec3(-0.4f, -0.5f, -0.2f));
glm::vec3 skyColor = glm::vec3(0.298f, 0.392f, 0.643f);
float fogDensity = 0.08;
float fogGradient = 0.5;

glm::quat rotation = glm::quat(1, 0, 0, 0);

float skyboxSize = 998.0f;

GLuint textureReef;
GLuint textureFish;
GLuint textureFish2;
GLuint textureShark;
GLuint textureDuck;
GLuint textureBubble;
GLuint texturePlant;
GLuint textureCoral;
GLuint textureKelp;

unsigned int skyboxVAO, skyboxVBO;

float skyboxVertices[] =
{
	20.0f, -20.0f,  20.0f,
	 20.0f, 20.0f,  20.0f,
	 20.0f, 20.0f, -20.0f,
	20.0f, -20.0f, -20.0f, 

	-20.0f,  -20.0f,  -20.0f,
	 -20.0f,  20.0f,  -20.0f,
	 -20.0f,  20.0f, 20.0f,    
	-20.0f,  -20.0f, 20.0f,

	-20.0f, 20.0f,  -20.0f,
	 20.0f, 20.0f,  -20.0f,
	 20.0f, 20.0f, 20.0f,
	-20.0f, 20.0f, 20.0f, 

	-20.0f,  -20.0f,  20.0f,
	 20.0f,  -20.0f,  20.0f,
	 20.0f,  -20.0f, -20.0f,
	-20.0f,  -20.0f, -20.0f,


	20.0f, -20.0f,  20.0f,
	-20.0f, -20.0f,  20.0f,
	-20.0f, 20.0f, 20.0f,
	20.0f, 20.0f, 20.0f,

	-20.0f,  -20.0f,  -20.0f,
	 20.0f,  -20.0f,  -20.0f,
	 20.0f,  20.0f, -20.0f,
	-20.0f,  20.0f, -20.0f
};


bool insideSkybox(glm::vec3 nextPosition) {
	return nextPosition.x < skyboxSize&& nextPosition.x > -skyboxSize
		&& nextPosition.y > -skyboxSize && nextPosition.y < skyboxSize
		&& nextPosition.z > -skyboxSize && nextPosition.z < skyboxSize;
}

void drawObjectColor(Core::RenderContext context, glm::mat4 modelMatrix, glm::vec3 color)
{
	GLuint program = programColor;


	glUseProgram(program);

	glUniform1f(glGetUniformLocation(program, "fogDensity"), fogDensity);
	glUniform1f(glGetUniformLocation(program, "fogGradient"), fogGradient);


	glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	glUniform3f(glGetUniformLocation(program, "skyColor"), skyColor.x, skyColor.y, skyColor.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "perspectiveMatrix"), 1, GL_FALSE, (float*)&perspectiveMatrix);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawContext(context);

	glUseProgram(0);
}

void drawObjectTexture(Core::RenderContext context, glm::mat4 modelMatrix, GLuint textureId)
{
	GLuint program = programTexture;

	glUseProgram(program);

	glUniform1f(glGetUniformLocation(program, "fogDensity"), fogDensity);
	glUniform1f(glGetUniformLocation(program, "fogGradient"), fogGradient);

	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	glUniform3f(glGetUniformLocation(program, "skyColor"), skyColor.x, skyColor.y, skyColor.z);
	Core::SetActiveTexture(textureId, "textureSampler", program, 0);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "perspectiveMatrix"), 1, GL_FALSE, (float*)&perspectiveMatrix);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawContext(context);

	glUseProgram(0);
}




void keyboard(unsigned char key, int x, int y)
{
	float timee = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	float angleSpeed = 0.1f;
	float moveSpeed = 0.3f;
	glm::vec3 desiredPosition;

	switch (key)
	{
	case 'z': cameraAngle -= angleSpeed; break;
	case 'x': cameraAngle += angleSpeed; break;
	case 'q': g = 1; break;
	case 'w': desiredPosition = cameraPos + cameraDir * moveSpeed; if (insideSkybox(desiredPosition)) { cameraPos = desiredPosition; } break;
	case 's': desiredPosition = cameraPos - cameraDir * moveSpeed; if (insideSkybox(desiredPosition)) { cameraPos = desiredPosition; } break;
	case 'd': desiredPosition = cameraPos + cameraSide * moveSpeed; if (insideSkybox(desiredPosition)) { cameraPos = desiredPosition; } break;
	case 'a': desiredPosition = cameraPos - cameraSide * moveSpeed; if (insideSkybox(desiredPosition)) { cameraPos = desiredPosition; } break;
	
	
	}
}

void mouse(int x, int y)
{
	if (old_x >= 0) {
		delta_x = x - old_x;
		delta_y = y - old_y;
	}
	old_x = x;
	old_y = y;
}


glm::mat4 createCameraMatrix()
{
	auto rot_y = glm::angleAxis(delta_y * 0.03f, glm::vec3(1, 0, 0));
	auto rot_x = glm::angleAxis(delta_x * 0.03f, glm::vec3(0, 1, 0));

	dy += delta_y;
	dx += delta_x;
	delta_x = 0;
	delta_y = 0;

	rotation_x = glm::normalize(rot_x * rotation_x);
	rotation_y = glm::normalize(rot_y * rotation_y);

	rotationCamera = glm::normalize(rotation_y * rotation_x);

	auto inverse_rot = glm::inverse(rotationCamera);

	cameraDir = inverse_rot * glm::vec3(0, 0, -1);
	glm::vec3 up = glm::vec3(0, 1, 0);
	cameraSide = inverse_rot * glm::vec3(1, 0, 0);

	glm::mat4 cameraTranslation;
	cameraTranslation[3] = glm::vec4(-cameraPos, 1.0f);

	return glm::mat4_cast(rotationCamera) * cameraTranslation;
}






void setupSkybox() {
	glGenVertexArrays(1, &skyboxVAO);
	glBindVertexArray(skyboxVAO);

	glGenBuffers(1, &skyboxVBO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)(sizeof(float) * 0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	textureCubemap = SOIL_load_OGL_cubemap(
		"textures/uw_ft.jpg",
		"textures/uw_bk.jpg",
		"textures/uw_up.jpg",
		"textures/uw_dn.jpg",
		"textures/uw_rt.jpg",
		"textures/uw_lf.jpg",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS
	);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);



}

void renderSkybox(glm::mat4 modelMatrix, GLuint textureId) {
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	glDepthMask(GL_FALSE);
	glEnable(GL_DEPTH_CLAMP);

	GLuint program = programSkybox;

	glUseProgram(program);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);

	glUniform1i(glGetUniformLocation(program, "skybox"), 0);
	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureCubemap);

	glBindVertexArray(skyboxVAO);
	glDrawArrays(GL_QUADS, 0, 24);
	glBindVertexArray(0);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glUseProgram(0);

	glDepthMask(GL_TRUE);
}


void renderScene()
{
	// Aktualizacja macierzy widoku i rzutowania
	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix();
	float timee = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 0.0);



	if (g == 1) { drawObjectTexture(bubbleContext, glm::translate(glm::vec3(0, 460, 0)) * glm::scale(glm::vec3(0.1f)), textureBubble); g = 0; }
	

	//coral objects placement
	drawObjectTexture(coralContext, glm::translate(glm::vec3(4, 452, 17)) * glm::scale(glm::vec3(2.6f)), textureCoral);
	drawObjectTexture(coralContext, glm::translate(glm::vec3(24, 463, 111)) * glm::scale(glm::vec3(3.0f)), textureCoral);
	drawObjectTexture(coralContext, glm::translate(glm::vec3(57, 457, 75)) * glm::scale(glm::vec3(2.0f)), textureCoral);

	//plants objects placement
	drawObjectTexture(plantContext, glm::translate(glm::vec3(-8, 450, 0)) * glm::scale(glm::vec3(1.8f)), texturePlant);
	drawObjectTexture(plantContext, glm::translate(glm::vec3(50, 450, 0)) * glm::scale(glm::vec3(3.8f)), texturePlant);
	drawObjectTexture(plantContext, glm::translate(glm::vec3(41, 452, 8)) * glm::scale(glm::vec3(1.3f)), texturePlant);
	drawObjectTexture(plantContext, glm::translate(glm::vec3(140, 452, 68)) * glm::scale(glm::vec3(4.1f)), texturePlant);
	drawObjectTexture(plantContext, glm::translate(glm::vec3(69, 459, 31)) * glm::scale(glm::vec3(2.1f)), texturePlant);
	drawObjectTexture(plantContext, glm::translate(glm::vec3(71, 458, 32)) * glm::scale(glm::vec3(1.1f)), texturePlant);
	drawObjectTexture(plantContext, glm::translate(glm::vec3(44, 453, 111)) * glm::scale(glm::vec3(4.1f)), texturePlant);
	drawObjectTexture(plantContext, glm::translate(glm::vec3(-118, 462, 75)) * glm::scale(glm::vec3(4.1f)), texturePlant);
	drawObjectTexture(plantContext, glm::translate(glm::vec3(-77, 451, 43)) * glm::scale(glm::vec3(2.4f)), texturePlant);
	drawObjectTexture(plantContext, glm::translate(glm::vec3(120, 450, -50)) * glm::scale(glm::vec3(3.8f)), texturePlant);
	drawObjectTexture(plantContext, glm::translate(glm::vec3(0, 450, -76)) * glm::scale(glm::vec3(3.8f)), texturePlant);
	drawObjectTexture(plantContext, glm::translate(glm::vec3(10, 457, -36)) * glm::scale(glm::vec3(2.8f)), texturePlant);
	drawObjectTexture(plantContext, glm::translate(glm::vec3(16, 461, -43)) * glm::scale(glm::vec3(1.2f)), texturePlant);


	//kelp objects placement
	drawObjectTexture(kelpContext, glm::translate(glm::vec3(-29, 452, -32)) * glm::scale(glm::vec3(1.9f)), textureKelp);
	drawObjectTexture(kelpContext, glm::translate(glm::vec3(-54, 452, -12)) * glm::scale(glm::vec3(2.9f)), textureKelp);
	drawObjectTexture(kelpContext, glm::translate(glm::vec3(-8, 452, -47)) * glm::scale(glm::vec3(2.2f)), textureKelp);
	drawObjectTexture(kelpContext, glm::translate(glm::vec3(33, 452, -80)) * glm::scale(glm::vec3(4.2f)), textureKelp);
	drawObjectTexture(kelpContext, glm::translate(glm::vec3(-90, 456, 60)) * glm::scale(glm::vec3(4.0f)), textureKelp);
	drawObjectTexture(kelpContext, glm::translate(glm::vec3(-125, 451, 1)) * glm::scale(glm::vec3(4.0f)), textureKelp);



	glm::mat4 shipInitialTransformation = glm::translate(glm::vec3(-0.4f, -0.25f, 0.1f)) * glm::rotate(glm::radians(10.0f), glm::vec3(3, 5, 0)) * glm::rotate(glm::radians(180.0f), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.35f));


	glm::mat4 shipModelMatrix = glm::translate(cameraPos + cameraDir * 0.5f) * glm::mat4_cast(glm::inverse(rotation)) * glm::mat4_cast(glm::inverse(rotationCamera)) * shipInitialTransformation;
	drawObjectTexture(shipContext, shipModelMatrix, textureDuck);

	drawObjectTexture(bottomContext, glm::translate(glm::vec3(0, 438, 0)) * glm::rotate(glm::radians(90.0f), glm::vec3(-1, 0, 0)) * glm::scale(glm::vec3(2.0f) * glm::vec3(0.2f)), textureReef);

	glm::mat4 skyboxModel = glm::scale(glm::mat4(1.0f), glm::vec3(50, 50, 50));
	renderSkybox(skyboxModel, textureCubemap);

	for (int i = 0; i < 50; i++) {
		if (i % 10 == 0) {
			drawObjectTexture(fishContext2, glm::translate(fishPositions[i]) * glm::scale(glm::vec3(0.7f) * glm::vec3(0.7f)) *
				glm::rotate(glm::radians((timee / 4) * 90.f), glm::vec3(0, -1, 0)) *
				glm::rotate(glm::radians(90.f), glm::vec3(0, 1, 0)) *
				glm::translate(glm::vec3(-70, 100, -50)) *
				glm::rotate(glm::radians((timee / 4) * 0.f), glm::vec3(0, 1, 0)) *
				glm::rotate(glm::radians(40.0f), glm::vec3(0, 1, 0)) *
				glm::rotate(glm::radians(180.0f), glm::vec3(0, -1, 0)) *
				glm::eulerAngleY(sin(timee * 10) / 4),
				textureFish2);

		} else if (i % 7 == 0) {
			drawObjectTexture(fishContext, glm::translate(fishPositions[i]) * glm::scale(glm::vec3(0.5f) * glm::vec3(0.5f)) *
				glm::rotate(glm::radians((timee / 4) * 90.f), glm::vec3(0, 1, 0)) *
				glm::rotate(glm::radians(90.f), glm::vec3(0, 1, 0)) *
				glm::translate(glm::vec3(-50 - 3 * i, 100, -70)) *
				glm::rotate(glm::radians((timee / 4) * 0.f), glm::vec3(0, 1, 0)) *
				glm::rotate(glm::radians(40.0f), glm::vec3(0, 1, 0)) *
				glm::eulerAngleY(sin(timee * 10) / 4),
				textureFish);

		} else if (i % 3 == 0) {
			drawObjectTexture(fishContext, glm::translate(fishPositions[i]) * glm::scale(glm::vec3(0.5f) * glm::vec3(0.5f)) *
				glm::rotate(glm::radians((timee / 4) * 90.f), glm::vec3(0, -1, 0)) *
				glm::rotate(glm::radians(90.f), glm::vec3(0, 1, 0)) *
				glm::translate(glm::vec3(-50 - 3 * i, 100, -85)) *
				glm::rotate(glm::radians((timee / 4) * 0.f), glm::vec3(0, 1, 0)) *
				glm::rotate(glm::radians(40.0f), glm::vec3(0, 1, 0)) *
				glm::rotate(glm::radians(180.0f), glm::vec3(0, -1, 0)) *
				glm::eulerAngleY(sin(timee * 10) / 4),
				textureFish);
		} else {
			drawObjectTexture(fishContext2, glm::translate(fishPositions[i]) * glm::scale(glm::vec3(0.25f) * glm::vec3(0.25f)) *
				glm::rotate(glm::radians((timee / 4) * 90.f), glm::vec3(0, 1, 0)) *
				glm::rotate(glm::radians(90.f), glm::vec3(0, 1, 0)) *
				glm::translate(glm::vec3(-50, 100, -fishPositions[i].z)) *
				glm::rotate(glm::radians((timee / 4) * 0.f), glm::vec3(0, 1, 0)) *
				glm::rotate(glm::radians(40.0f), glm::vec3(0, 1, 0)) *
				glm::eulerAngleY(sin(timee * 10) / 4),
				textureFish2);
		}	
	}

	for (int j = 0; j < 2; j++) drawObjectTexture(sharkContext, glm::translate(glm::vec3( 70 * j, 575 + 20 * j, 20 + 50 * j)) *
									glm::rotate(glm::radians((timee / 12) * 90.f), glm::vec3(0, 1, 0)) *
									glm::rotate(glm::radians(90.f), glm::vec3(0, 1, 0)) * glm::translate(glm::vec3(-95, -95, -100)) *
									glm::rotate(glm::radians((timee / 12) * 0.f), glm::vec3(0, 1, 0)) * glm::eulerAngleY(sin(timee * 2) / 4) * 
									glm::scale(glm::vec3(0.3f) * glm::vec3(0.3f)) *
									glm::rotate(glm::radians(40.0f), glm::vec3(0, 1, 0)) *
									glm::rotate(glm::radians(90.0f), glm::vec3(-1, 0, 0)),
									textureShark);

	for (int j = 0; j < 2; j++) drawObjectTexture(sharkContext, glm::translate(glm::vec3(70 * j, 565 + 20 * j, 20 + 50 * j)) *
		glm::rotate(glm::radians((timee / 12) * 90.f), glm::vec3(0, -1, 0)) *
		glm::rotate(glm::radians(90.f), glm::vec3(0, 1, 0)) * glm::translate(glm::vec3(-95, -95, -100)) *
		glm::rotate(glm::radians((timee / 12) * 0.f), glm::vec3(0, 1, 0)) * glm::eulerAngleY(sin(timee * 2) / 4) *
		glm::scale(glm::vec3(0.3f) * glm::vec3(0.3f)) *
		glm::rotate(glm::radians(120.0f), glm::vec3(0, -1, 0)) *
		glm::rotate(glm::radians(90.0f), glm::vec3(-1, 0, 0)),
		textureShark);


	drawObjectTexture(sharkContext, glm::translate(glm::vec3(100, 560, 100)) *
		glm::rotate(glm::radians((timee / 12) * 90.f), glm::vec3(0, 1, 0)) *
		glm::rotate(glm::radians(90.f), glm::vec3(0, 1, 0)) * glm::translate(glm::vec3(-95, -95, -100)) *
		glm::rotate(glm::radians((timee / 12) * 0.f), glm::vec3(0, 1, 0)) * glm::eulerAngleY(sin(timee * 2) / 4) *
		glm::scale(glm::vec3(0.4f) * glm::vec3(0.4f)) *
		glm::rotate(glm::radians(40.0f), glm::vec3(0, 1, 0)) *
		glm::rotate(glm::radians(90.0f), glm::vec3(-1, 0, 0)),
		textureShark);



	skyboxModel = glm::scale(glm::mat4(1.0f), glm::vec3(50, 50, 50));
	renderSkybox(skyboxModel, textureCubemap);

	glutSwapBuffers();
}

void loadModelToContext(std::string path, Core::RenderContext& context)
{
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	context.initFromAssimpMesh(scene->mMeshes[0]);
}



void init()
{
	srand(time(0));
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_FOG);
	programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	programSkybox = shaderLoader.CreateProgram("shaders/shader_skybox.vert", "shaders/shader_skybox.frag");
	loadModelToContext("models/boat.obj", shipContext);
	loadModelToContext("models/bottom.obj", bottomContext);
	loadModelToContext("models/nemo.obj", fishContext);
	loadModelToContext("models/fish.obj", fishContext2);
	loadModelToContext("models/shark.obj", sharkContext);
	loadModelToContext("models/duck.obj", duckContext);
	loadModelToContext("models/bubble.obj", bubbleContext);
	loadModelToContext("models/seaweed.obj", plantContext);
	loadModelToContext("models/coral.obj", coralContext);
	loadModelToContext("models/kelp.obj", kelpContext);


	setupSkybox();
	textureFish = Core::LoadTexture("textures/color.jpg");
	textureReef = Core::LoadTexture("textures/ground.jpg");
	textureFish2 = Core::LoadTexture("textures/fish.png");
	textureShark = Core::LoadTexture("textures/shark.jpg");
	textureDuck = Core::LoadTexture("textures/boat.png");
	textureBubble = Core::LoadTexture("textures/bubble.jpg");
	texturePlant = Core::LoadTexture("textures/plant.png");
	textureCoral = Core::LoadTexture("textures/coral.png");
	textureKelp = Core::LoadTexture("textures/kelp.png");

	//textureFish = Core::LoadTexture("textures/xd.jpg");
	for (int i = 0; i < 100; i++) fishPositions[i] = glm::vec3(rand()%100,(rand()%50) + 430, rand()% 100);
}

void shutdown()
{
	shaderLoader.DeleteProgram(programColor);
	shaderLoader.DeleteProgram(programTexture);
}

void idle()
{
	glutPostRedisplay();
}

int main(int argc, char ** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1280, 720);
	glutCreateWindow("graficzka.exe");
	glewInit();

	init();
	glutKeyboardFunc(keyboard);
	glutPassiveMotionFunc(mouse);
	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);

	glutMainLoop();

	shutdown();

	return 0;
}
