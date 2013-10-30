#ifndef GLSTUFF_H
#define GLSTUFF_H

#include <GL/glew.h>
#include <string>
#include <vector>
#include <cmath>
#include <cstdio>
#include <algorithm>
#include <fstream>
#include <assimp/types.h>

/*
extern unsigned int fb;
extern unsigned int rb[2];
*/

std::string readTextFile(const std::string& path);
void printShaderLog(GLuint shader);
void printProgramLog(GLuint program);
GLuint createShader(const std::string& path);
GLuint createShaderProgram();
GLuint createVAO();
GLuint createVBO(const aiVector2D* data, unsigned int len);
GLuint createVBO(const aiVector3D* data, unsigned int len);
GLuint createVBO(const int*        data, unsigned int len);
GLuint createVBO(const unsigned int* data, unsigned int len);
GLuint createVBO(const float* data, unsigned int len);
void bindVAO(GLuint vao);
void bindVBOFloat(GLuint program, const std::string& name, GLuint vbo, int numComponents);
void bindVBOUint(GLuint program, const std::string& name, GLuint vbo, int numComponents);
void bindVBOIndices(GLuint program, GLuint vbo);
void bindUniformMatrix4(GLuint program, const std::string& name, const 
aiMatrix4x4& matrix);
void bindUniformMatrix4Array(GLuint program, const std::string& name, int count, const aiMatrix4x4* matrix);
void bindUniformSampler(GLuint program, const std::string& name, GLuint sampler);
void bindVBOEmpty(GLuint program, const std::string& name);


#endif
