#include "../h/ShaderProgram.h"

struct SingleShader::Impl: MessageLogger
{
	GLuint shaderId = 0;
};

SingleShader::SingleShader()
{
	impl = shared_ptr<Impl>(new Impl);
	impl->tag = "SingleShader";
}

SingleShader SingleShader::fromFile(string file, GLenum type, VerboseToggle verbose)
{
	string code;
	
	if (!loadFile(file, code, verbose))
	{
		MessageLogger::show("error: shader file '" + file + "' failed to load");
	}
	
	return fromCode(code, type, verbose);
}

SingleShader SingleShader::fromCode(string code, GLenum type, VerboseToggle verbose)
{
	SingleShader shader;
	shader.impl->verbose = verbose;
	shader.impl->status("compiling " + typeToString(type) + "...");

	const char* codeCharPtr=code.c_str();
	shader.impl->shaderId = glCreateShader(type);
	glShaderSource(shader.impl->shaderId, 1, &codeCharPtr, 0);
	glCompileShader(shader.impl->shaderId);
	// Check for compile time errors
	GLint success;
	glGetShaderiv(shader.impl->shaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		shader.impl->important("shader failed to compile:\n\n" + shader.getInfoLog() + "\n");
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
			return "unknown shader type";
	}
}

struct ShaderProgram::Impl: MessageLogger
{
	GLuint programId = 0;
	
	~Impl()
	{
		glDeleteProgram(programId);
	}
};

ShaderProgram::ShaderProgram()
{
	impl = shared_ptr<Impl>(new Impl);
	impl->tag = "ShaderProgram";
}

ShaderProgram::ShaderProgram(shared_ptr<Impl> implIn)
{
	impl = implIn;
}

ShaderProgram ShaderProgram::fromFiles(string vertFile, string fragFile, VerboseToggle verbose)
{
	return ShaderProgram::fromShaders(
		SingleShader::fromFile(vertFile, GL_VERTEX_SHADER, verbose),
		SingleShader::fromFile(fragFile, GL_FRAGMENT_SHADER, verbose),
		verbose
	);
}

ShaderProgram ShaderProgram::fromCode(string vertCode, string fragCode, VerboseToggle verbose)
{
	return ShaderProgram::fromShaders(
		SingleShader::fromCode(vertCode, GL_VERTEX_SHADER, verbose),
		SingleShader::fromCode(fragCode, GL_FRAGMENT_SHADER, verbose),
		verbose
	);
}

ShaderProgram ShaderProgram::fromShaders(SingleShader vertShader, SingleShader fragShader, VerboseToggle verbose)
{
	ShaderProgram program;
	program.impl->verbose = verbose;
	
	program.impl->status("creating shader program...");
	
	program.impl->programId = glCreateProgram();
	
	glAttachShader(program.impl->programId, vertShader.getShaderId());
	glAttachShader(program.impl->programId, fragShader.getShaderId());
	glLinkProgram(program.impl->programId);
	// Check for linking errors
	GLint success;
	glGetProgramiv(program.impl->programId, GL_LINK_STATUS, &success);
	if (!success) {
		program.impl->important("shader linking failed:\n\n" + program.getInfoLog() + "\n");
	}
	
	program.impl->status("info log: " + program.getInfoLog());

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