#include "ShaderProgram.h"

// change to toggle debug statements on and off
#define debug debug_off

struct ShaderProgram::Impl
{
	GLuint programId = 0;
	
	struct SingleShader
	{
		GLuint shaderId = 0;
		
		static SingleShader fromFile(string file, GLenum type);
		static SingleShader fromCode(string code, GLenum type);
		
		SingleShader() {}
		~SingleShader();
		
		string getInfoLog()
		{
			GLchar infoLog[1024];
			glGetShaderInfoLog(shaderId, 1024, nullptr, infoLog);
			return infoLog;
		}
		
		static string typeToString(GLenum type)
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
	};
	
	Impl(SingleShader vertShader, SingleShader fragShader)
	{
		debug("creating shader program");
		
		programId = glCreateProgram();
		
		glAttachShader(programId, vertShader.shaderId);
		glAttachShader(programId, fragShader.shaderId);
		glLinkProgram(programId);
		// Check for linking errors
		GLint success;
		glGetProgramiv(programId, GL_LINK_STATUS, &success);
		if (!success) {
			warning("shader linking failed:\n" + getInfoLog());
		}
	}
	
	~Impl()
	{
		glDeleteProgram(programId);
	}
	
	string getInfoLog()
	{
		GLchar infoLog[1024];
		glGetProgramInfoLog(programId, 1024, nullptr, infoLog);
		return infoLog;
	}
};

ShaderProgram::Impl::SingleShader ShaderProgram::Impl::SingleShader::fromFile(string file, GLenum type)
{
	string code;
	
	if (!loadFile(file, code))
	{
		warning("shader file '" + file + "' failed to load");
	}
	
	return fromCode(code, type);
}

ShaderProgram::Impl::SingleShader ShaderProgram::Impl::SingleShader::fromCode(string code, GLenum type)
{
	SingleShader shader;
	debug("compiling " + typeToString(type) + "...");

	const char* codeCharPtr=code.c_str();
	shader.shaderId = glCreateShader(type);
	glShaderSource(shader.shaderId, 1, &codeCharPtr, 0);
	glCompileShader(shader.shaderId);
	// Check for compile time errors
	GLint success;
	glGetShaderiv(shader.shaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		warning("shader failed to compile:\n" + shader.getInfoLog());
	}
	
	return shader;
}

ShaderProgram::Impl::SingleShader::~SingleShader()
{
	glDeleteShader(shaderId);
}

void ShaderProgram::setupFromFiles(string vertFile, string fragFile)
{
	ASSERT(!impl);
	impl = make_shared<Impl>(
		ShaderProgram::Impl::SingleShader::fromFile(vertFile, GL_VERTEX_SHADER),
		ShaderProgram::Impl::SingleShader::fromFile(fragFile, GL_FRAGMENT_SHADER)
	);
}

void ShaderProgram::setupFromCode(string vertCode, string fragCode)
{
	ASSERT(!impl);
	impl = make_shared<Impl>(
		ShaderProgram::Impl::SingleShader::fromCode(vertCode, GL_VERTEX_SHADER),
		ShaderProgram::Impl::SingleShader::fromCode(fragCode, GL_FRAGMENT_SHADER)
	);
}

void ShaderProgram::bind()
{
	ASSERT_ELSE(impl, return);
	
	glUseProgram(impl->programId);
}

void ShaderProgram::unbind()
{
	glUseProgram(false);
}

void ShaderProgram::uniformMatrix4fv(string name, GLfloat * data)
{
	ASSERT_ELSE(impl, return);
	GLint loc = glGetUniformLocation(impl->programId, name.c_str());
	if (loc == -1)
	{
		warning("unknown shader uniform name '" + name + "' given");
		return;
	}
	glUniformMatrix4fv(loc, 1, GL_FALSE, data);
}
