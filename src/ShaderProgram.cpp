#include "../h/ShaderProgram.h"

ShaderProgram::ShaderProgram(string vertFile, string fragFile, bool debug)
{
	Shader vertShader(vertFile, GL_VERTEX_SHADER, debug);
	Shader fragShader(fragFile, GL_FRAGMENT_SHADER, debug);
	
    data = glCreateProgram();
    glAttachShader(data, vertShader);
    glAttachShader(data, fragShader);
    glLinkProgram(data);
    // Check for linking errors
    GLint success;
    glGetProgramiv(data, GL_LINK_STATUS, &success);
    if (!success) {
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << getInfoLog() << std::endl;
    }
}

string ShaderProgram::getInfoLog()
{
	GLchar infoLog[1024];
	glGetProgramInfoLog(data, 1024, NULL, infoLog);
	string out(infoLog);
	return out;
}

ShaderProgram::~ShaderProgram()
{
	//glDeleteProgram(data);
}

ShaderProgram::Shader::Shader(string file, GLenum type, bool debug)
{
	string sourceString;
	if (!loadFile(file, sourceString, debug))
		cout << "vertex shader file failed to load" << endl;
	const char* source=sourceString.c_str();
    data = glCreateShader(type);
    glShaderSource(data, 1, &source, NULL);
    glCompileShader(data);
    // Check for compile time errors
    GLint success;
    glGetShaderiv(data, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        std::cerr << "shader error: " << file << " " << getInfoLog() << std::endl;
    }
}

string ShaderProgram::Shader::getInfoLog()
{
	GLchar infoLog[1024];
	glGetShaderInfoLog(data, 1024, NULL, infoLog);
	string out(infoLog);
	return out;
}

ShaderProgram::Shader::~Shader()
{
	//glDeleteShader(data);
}
