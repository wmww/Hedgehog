#include "ShaderProgram.h"

// change to toggle debug statements on and off
#define debug debug_off

struct SingleShader::Impl
{
	GLuint shaderId = 0;
};

SingleShader::SingleShader()
{
	impl = shared_ptr<Impl>(new Impl);
}

SingleShader SingleShader::fromFile(string file, GLenum type)
{
	string code;
	
	if (!loadFile(file, code))
	{
		warning("shader file '" + file + "' failed to load");
	}
	
	return fromCode(code, type);
}

SingleShader SingleShader::fromCode(string code, GLenum type)
{
	SingleShader shader;
	debug("compiling " + typeToString(type) + "...");

	const char* codeCharPtr=code.c_str();
	shader.impl->shaderId = glCreateShader(type);
	glShaderSource(shader.impl->shaderId, 1, &codeCharPtr, 0);
	glCompileShader(shader.impl->shaderId);
	// Check for compile time errors
	GLint success;
	glGetShaderiv(shader.impl->shaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		warning("shader failed to compile:\n" + shader.getInfoLog());
	}
	
	return shader;
}

GLuint SingleShader::getShaderId()
{
	return impl->shaderId;
}

string SingleShader::getInfoLog()
{
	GLchar infoLog[1024];
	glGetShaderInfoLog(impl->shaderId, 1024, nullptr, infoLog);
	string out(infoLog);
	return out;
}

string SingleShader::typeToString(GLenum type)
{
	switch (type)
	{
		case GL_VERTEX_SHADER:
			return "vertex shader";
		case GL_FRAGMENT_SHADER:
			return "fragment shader";
		default:
			warning("unknown shader type");
			return "unknown shader type";
	}
}

struct ShaderProgram::Impl
{
	GLuint programId = 0;
	
	~Impl()
	{
		glDeleteProgram(programId);
	}
};

ShaderProgram::ShaderProgram()
{
	impl = make_shared<Impl>();
}

ShaderProgram::ShaderProgram(shared_ptr<Impl> implIn)
{
	impl = implIn;
}

ShaderProgram ShaderProgram::fromFiles(string vertFile, string fragFile)
{
	return ShaderProgram::fromShaders(
		SingleShader::fromFile(vertFile, GL_VERTEX_SHADER),
		SingleShader::fromFile(fragFile, GL_FRAGMENT_SHADER)
	);
}

ShaderProgram ShaderProgram::fromCode(string vertCode, string fragCode)
{
	return ShaderProgram::fromShaders(
		SingleShader::fromCode(vertCode, GL_VERTEX_SHADER),
		SingleShader::fromCode(fragCode, GL_FRAGMENT_SHADER)
	);
}

ShaderProgram ShaderProgram::fromShaders(SingleShader vertShader, SingleShader fragShader)
{
	ShaderProgram program;
	
	debug("creating shader program...");
	
	program.impl->programId = glCreateProgram();
	
	glAttachShader(program.impl->programId, vertShader.getShaderId());
	glAttachShader(program.impl->programId, fragShader.getShaderId());
	glLinkProgram(program.impl->programId);
	// Check for linking errors
	GLint success;
	glGetProgramiv(program.impl->programId, GL_LINK_STATUS, &success);
	if (!success) {
		warning("shader linking failed:\n" + program.getInfoLog());
	}
	
	return program;
}

void ShaderProgram::activete()
{
	assert(impl);
	
	glUseProgram(impl->programId);
}

void ShaderProgram::deactivate()
{
	glUseProgram(false);
}

string ShaderProgram::getInfoLog()
{
	assert(impl);
	GLchar infoLog[1024];
	glGetProgramInfoLog(impl->programId, 1024, nullptr, infoLog);
	string out(infoLog);
	return out;
}

GLuint ShaderProgram::getProgramId()
{
	assert(impl);
	return impl->programId;
}

/*
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
*/