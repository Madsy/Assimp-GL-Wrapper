#include "glstuff.h"

std::string readTextFile(const std::string& path)
{
	std::ifstream strm(path.c_str());
	if(!strm.is_open()) return "";

	std::string line;
	std::string result;
	while(std::getline(strm, line, '\n')){
		result += line + "\n";
	}
	strm.close();
	return result;
}

void printShaderLog(GLuint shader)
{
	int len;
	char* log = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
	if(len > 0){
		log = new char[len];
		glGetShaderInfoLog(shader, len, 0, log);
	}
	printf("%s\n", log);
	delete [] log;
}

void printProgramLog(GLuint program)
{
	int len;
	char* log = 0;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
	if(len > 0){
		log = new char[len];
		glGetProgramInfoLog(program, len, 0, log);
	}
	printf("%s\n", log);
	delete [] log;
}


GLuint createShaderProgram()
{
	GLuint program = glCreateProgram();
	GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmt = glCreateShader(GL_FRAGMENT_SHADER);

	std::string vertexSrc = readTextFile("assimp_wrapper/shader.vs");
	std::string fragmtSrc = readTextFile("assimp_wrapper/shader.fs");

	const char* str_v = vertexSrc.c_str();
	const char* str_f = fragmtSrc.c_str();
	
	glShaderSource(vertex, 1, &str_v, 0);
	glShaderSource(fragmt, 1, &str_f, 0);

	glCompileShader(vertex);
	glCompileShader(fragmt);
	glAttachShader(program, vertex);
	glAttachShader(program, fragmt);
	glLinkProgram(program);

	int vstatus, fstatus, lstatus;
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &vstatus);
	printf("Vertex shader compile status: %s\n", (vstatus==GL_TRUE)?"true":"false");
	glGetShaderiv(fragmt, GL_COMPILE_STATUS, &fstatus);
	printf("Fragment shader compile status: %s\n", (fstatus==GL_TRUE)?"true":"false");
	glGetProgramiv(program, GL_LINK_STATUS, &lstatus);
	printf("Program link status: %s\n", (lstatus==GL_TRUE)?"true":"false");

	if(!vstatus){
		printf("Vertex shader log: \n");
		printShaderLog(vertex);
	}
	if(!fstatus){
		printf("Fragment shader log: \n");
		printShaderLog(fragmt);
	}
	
	if(!lstatus){
		printf("Program link log: \n");
		printProgramLog(program);
	}
	
	return program;
}

GLuint createVAO()
{
	GLuint vao;
	glGenVertexArrays(1, &vao);
	return vao;
}

GLuint createVBO(const aiVector2D* data, unsigned int len)
{
	GLuint vbo;
	if(!len) return ~0u;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, len * sizeof(aiVector2D),
				 data, GL_STATIC_DRAW);
	return vbo;
}

GLuint createVBO(const aiVector3D* data, unsigned int len)
{
	GLuint vbo;
	if(!len) return ~0u;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, len * sizeof(aiVector3D),
				 data, GL_STATIC_DRAW);
	return vbo;
}

GLuint createVBO(const int* data, unsigned int len)
{
	GLuint vbo;
	if(!len) return ~0u;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, len * sizeof(int),
				 data, GL_STATIC_DRAW);
	return vbo;
}

GLuint createVBO(const unsigned int* data, unsigned int len)
{
	GLuint vbo;
	if(!len) return ~0u;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, len * sizeof(unsigned int),
				 data, GL_STATIC_DRAW);
	return vbo;
}

GLuint createVBO(const float* data, unsigned int len)
{
	GLuint vbo;
	if(!len) return ~0u;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, len * sizeof(float),
				 data, GL_STATIC_DRAW);
	return vbo;
}

void bindVAO(GLuint vao)
{
	if(vao == ~0u){
		printf("Tried to bind invalid VAO.\n");
		return;
	}
	glBindVertexArray(vao);
}

void bindVBOFloat(GLuint program, const std::string& name, GLuint vbo, int numComponents)
{
	int loc = glGetAttribLocation(program, name.c_str());
	if(loc == -1){
		//printf("Didn't find vbo named %s\n", name.c_str());
		return;
	}
	
	glEnableVertexAttribArray(loc);
	if(vbo == ~0u){
		printf("Tried to bind invalid Float VBO with name %s.\n", name.c_str());
		glDisableVertexAttribArray(loc);
		return;
	}
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(loc, numComponents, GL_FLOAT, GL_FALSE, 0, 0);
}

void bindVBOUint(GLuint program, const std::string& name, GLuint vbo, int numComponents)
{
	int loc = glGetAttribLocation(program, name.c_str());
	if(loc == -1){
		//printf("Didn't find vbo named %s\n", name.c_str());
		return;
	}
	
	glEnableVertexAttribArray(loc);
	if(vbo == ~0u){
		printf("Tried to bind invalid Uint VBO with name %s.\n", name.c_str());
		glDisableVertexAttribArray(loc);
		return;
	}	
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribIPointer(loc, numComponents, GL_UNSIGNED_INT, 0, 0);
}

void bindVBOIndices(GLuint program, GLuint vbo)
{
	if(vbo == ~0u){
		printf("Tried to bind invalid element index VBO.\n");
		return;	
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo);
}

void bindUniformMatrix4(GLuint program, const std::string& name, const 
aiMatrix4x4& matrix)
{
	int loc = glGetUniformLocation(program, name.c_str());
	if(loc == -1){
		//printf("Didn't find uniform named %s\n", name.c_str());
		return;
	}
	glUniformMatrix4fv(loc, 1, GL_TRUE, matrix[0]);
}

void bindUniformMatrix4Array(GLuint program, const std::string& name, int count, const aiMatrix4x4* matrix)
{
	int loc = glGetUniformLocation(program, name.c_str());
	if(loc == -1){
		//printf("Didn't find uniform named %s\n", name.c_str());
		return;
	}
	glUniformMatrix4fv(loc, count, GL_TRUE, (*matrix)[0]);
}

void bindUniformSampler(GLuint program, const std::string& name, GLuint sampler)
{
	int loc = glGetUniformLocation(program, name.c_str());
	if(loc == -1){
		//printf("Didn't find uniform sampler named %s\n", name.c_str());
		return;
	}
	glUniform1i(loc, sampler - GL_TEXTURE0);
}

void bindVBOEmpty(GLuint program, const std::string& name)
{
	int loc = glGetAttribLocation(program, name.c_str());
	if(loc == -1){
		printf("Didn't find vbo named %s\n", name.c_str());
		return;
	}
	glVertexAttrib4f(loc, 0.0f, 0.0f, 0.0f, 0.0f);
}

#if 0
bool checkFrameBuffer(GLuint fbuffer)
{
	bool isFB = glIsFramebuffer(fbuffer);
	bool isCA = glIsRenderbuffer(rb[0]);
	bool isDSA = glIsRenderbuffer(rb[1]);
	bool isComplete = false;
	if(isFB){
		glBindFramebuffer(GL_FRAMEBUFFER, fbuffer);		
		isComplete = (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	}

	return isFB && isCA &&isDSA && isComplete;	
}


static bool createFrameBuffer() //for worker thread
{
	bool ret;
	
	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glGenRenderbuffers(2, rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb[0]);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 2, GL_RGBA8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, rb[1]);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 2, GL_DEPTH24_STENCIL8, width, height);

	glBindRenderbuffer(GL_RENDERBUFFER, rb[0]);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb[0]);
	glBindRenderbuffer(GL_RENDERBUFFER, rb[1]);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rb[1]);

	if(!(ret = checkFrameBuffer(fb))){
		glDeleteRenderbuffers(2, rb);
		glDeleteFramebuffers(1,  &fb);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	return ret;	
}
#endif
