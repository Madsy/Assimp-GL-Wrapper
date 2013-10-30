#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <assimp/cimport.h>
#include <assimp/scene.h> 
#include <assimp/postprocess.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <cmath>
#include <cstdio>
#include <algorithm>
#include <fstream>
#include <assimp/types.h>
#include "../include/linealg.h"
#include "png_loader.h"
#include "glstuff.h"
#include "scene.h"
//#define GL33
//#define FULLSCREEN

static const int width = 1280;   
static const int height = 720;

GLFWwindow* window;

class SimpleRenderer : public AnimRenderer
{
public:
	SimpleRenderer()
	{
		shader = createShaderProgram();
		projection = perspective(90.0f, 16.0f/9.0f, 1.0f, 100.0f);
		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, width, height);
		glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
		std::vector<unsigned int> texData;
		unsigned int texWidth, texHeight;
		if(!LoadImagePNG("data/checker.png", texData, texWidth, texHeight)){
			printf("Couldn't load image checker.png.\n");
			texWidth = 256;
			texHeight = 256;
			texData.resize(texWidth * texHeight);
			std::fill(texData.begin(), texData.end(), 0xFFFFFFFF);
		}
		glGenTextures(1, &texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &texData[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	
	void draw()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		bindUniformSampler(shader, "sampler0", GL_TEXTURE0);
		int loc = glGetUniformLocation(shader, "projection");
		if(loc != -1)
			glUniformMatrix4fv(loc, 1, GL_TRUE, projection.c_ptr());
		drawAllObjects(shader);
	}
private:
	GLuint shader;
	GLuint texture;
	Matrix4f projection;
};
	
static GLFWwindow* initWindow(GLFWwindow* shared, bool visible)
{
	GLFWwindow* win;
#ifdef GL33
 	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#endif
	if(visible)
		glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
	else 
		glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
	
#ifdef FULLSCREEN
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();	
    win = glfwCreateWindow(width, height, "Optimus example", monitor, shared);
#else
    win = glfwCreateWindow(width, height, "Optimus example", 0, shared);
#endif
	return win;	
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if ((key == GLFW_KEY_ESCAPE || key == GLFW_KEY_ENTER)
		&& action == GLFW_PRESS){		
        glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

int main(int argc, char* argv[])
{
#if 1	
	if(!glfwInit()){
		printf("Failed to initialize glfw\n");		
		return 0;
	}
	//main window
	window = initWindow(0, true);

	if(!window){
		glfwTerminate();
		printf("Failed to create glfw windows\n");
		return 0;		
	}
	glfwSetKeyCallback(window, key_callback);	
	glfwMakeContextCurrent(window);
	
	if(glewInit() /* || !createFrameBuffer() */ ){
		printf("Failed to init GL\n");
		glfwDestroyWindow(window);
		glfwTerminate();
		return 0;
	}

	//Scene scene("data/pandoras_box2.x");
	//Scene scene("data/test.dae");
	Scene scene("data/pandoras_box3.dae");
	#if 1
	aiVector3D trans(0.0f, 0.0f, -2.0f);
	aiMatrix4x4 camera;
	aiMatrix4x4::Translation(trans, camera);
	//AnimGLData* animation = scene.createAnimation("ArmatureAction", camera);
	AnimGLData* animation = scene.createAnimation(0, camera);
	if(!animation){
		printf("Couldn't find animation ArmatureAction.\n");
		glfwDestroyWindow(window);
		glfwTerminate();
		return 0;
	}
	
	AnimRenderer* renderer = new SimpleRenderer;
	animation->addRenderer(renderer);
	#endif


    while(!glfwWindowShouldClose(window)){
		glfwPollEvents();
		float t = glfwGetTime();
		animation->render(t);
		glfwSwapBuffers(window);
		if(t*32.0f >= 190.0f)
			glfwSetTime(0.0f);
	}
    glfwDestroyWindow(window);
	glfwTerminate();
#endif
	return 0;
}
