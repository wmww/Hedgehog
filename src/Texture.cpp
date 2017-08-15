#include "../h/Texture.h"
#include "../h/ShaderProgram.h"

ShaderProgram shaderProgram(nullptr);

const string vertShaderCode = "#version 330 core "
"layout (location = 0) in vec3 position; "
"layout (location = 1) in vec3 color; "
"layout (location = 2) in vec2 texCoord; "
"out vec3 ourColor; "
"out vec2 TexCoord; "
"void main() "
"{ "
	"gl_Position = vec4(position.x, position.y, position.z, 1.0); "
	"gl_Position = vec4(position, 1.0f); "
    "ourColor = color; "
    "TexCoord = texCoord; "
"} ";

const string fragShaderCode = "#version 330 core"
"in vec3 ourColor; "
"in vec2 TexCoord; "
"out vec4 color; "
"uniform sampler2D ourTexture; "
"void main() "
"{ "
	"color = texture(ourTexture, TexCoord); "
"} ";

struct Texture::Impl: MessageLogger
{
	GLuint squareVAOId;
	GLuint textureId;
	
	Impl(VerboseToggle verboseToggle)
	{
		verbose = verboseToggle;
		tag = "Texture";
		
		setupIfFirstInstance(this);
		
		setupVAO();
		
		setupGlTexture();
	}
	
	void setupVAO()
	{
		status("setting up a VAO");
		
		GLuint VBO, EBO;
		
		GLfloat vertices[] =
		{
			//Positions				Colors					Texture Coords
			1.0f,	1.0f,	0.0f,	0.0f,	1.0f,	0.0f,	1.0f,	0.0f,	// Top Right
			1.0f,	-1.0f,	0.0f,	0.0f,	0.5f,	0.5f,	1.0f,	1.0f,	// Bottom Right
			-1.0f,	-1.0f,	0.0f,	0.0f,	0.0f,	1.0f,	0.0f,	1.0f,	// Bottom Left
			-1.0f,	1.0f,	0.0f,	0.0f,	0.0f,	0.5f,	0.0f,	0.0f,	// Top Left 
		};
		
		GLuint indices[] =
		{
			0, 1, 3,  // First Triangle
			1, 2, 3   // Second Triangle
		};
		
		//glViewport(0, 0, width, height);
		
		glGenVertexArrays(1, &squareVAOId);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		
		// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
		glBindVertexArray(squareVAOId);
		{
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			{
				glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
				
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
				glEnableVertexAttribArray(0);
				
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GL_FLOAT)));
				glEnableVertexAttribArray(1);
				
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6*sizeof(GL_FLOAT)));
				glEnableVertexAttribArray(2);
			}
			glBindBuffer(GL_ARRAY_BUFFER, false); // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			{
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
			}
			//remember: do NOT unbind the EBO, keep it bound to this VAO
		}
		glBindVertexArray(false); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs)
		
		//deleting these objects appears to work, not sure if its supposed to though
		//glDeleteBuffers(1, &VBO);
		//glDeleteBuffers(1, &EBO);
	}
	
	void setupGlTexture()
	{
		status("setting up OpenGL texture");
		
		glGenTextures(1, &textureId);
		
		glBindTexture(GL_TEXTURE_2D, textureId);
		{
			// Set the texture wrapping/filtering options (on the currently bound texture object)
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		glBindTexture(GL_TEXTURE_2D, false);
	}
	
	static void firstInstanceSetup()
	{
		shaderProgram = ShaderProgram::fromCode(vertShaderCode, fragShaderCode, VERBOSE_OFF);
	}
};

Texture::Texture(VerboseToggle verboseToggle)
{
	impl = shared_ptr<Impl>(new Impl(verboseToggle));
}

void Texture::draw()
{
	assert(impl);
	shaderProgram.activete();
	{
		glBindTexture(GL_TEXTURE_2D, impl->textureId);
		{
			glBindVertexArray(impl->squareVAOId);
			{
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			}
			glBindVertexArray(false);
		}
		glBindTexture(GL_TEXTURE_2D, false);
	}
	shaderProgram.deactivate();
}

GLuint Texture::getTextureId()
{
	assert(impl);
	return impl->textureId;
}
