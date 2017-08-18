#include "../h/Texture.h"
#include "../h/ShaderProgram.h"
#include <SOIL/SOIL.h>

ShaderProgram shaderProgram(nullptr);

const string vertShaderCode = "#version 330 core\n"
"layout (location = 0) in vec2 position; "
"layout (location = 1) in vec2 texturePositionIn; "
"out vec2 texturePosition; "
"void main() "
"{ "
	"gl_Position = vec4(position, 0.0f, 1.0f); "
    "texturePosition = texturePositionIn; "
"} ";

const string fragShaderCode = "#version 330 core\n"
"in vec2 texturePosition; "
"out vec4 color; "
"uniform sampler2D textureData; "
"void main() "
"{ "
	"color = texture(textureData, texturePosition); "
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
			// position		// texture position
			1.0f,	1.0f,	1.0f,	0.0f,	// Top Right
			1.0f,	-1.0f,	1.0f,	1.0f,	// Bottom Right
			-1.0f,	-1.0f,	0.0f,	1.0f,	// Bottom Left
			-1.0f,	1.0f,	0.0f,	0.0f,	// Top Left 
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
				
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
				glEnableVertexAttribArray(0);
				
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*) (2 * sizeof(GLfloat)));
				glEnableVertexAttribArray(1);
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

void Texture::loadFromImage(string imagePath)
{
	impl->status("loading '" + imagePath + "' into texture...");
	// Load and generate the texture
	int width, height;
	unsigned char* image = SOIL_load_image(imagePath.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
	if (!image)
	{
		impl->important(string() + "image loading error: " + SOIL_last_result());
	}
	
	impl->status("creating texture from image...");
	
	glBindTexture(GL_TEXTURE_2D, impl->textureId);
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentally mess up our texture.
	
	SOIL_free_image_data(image);
}

void Texture::draw()
{
	assert(impl);
	//impl->status("drawing...");
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
