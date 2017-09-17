#include "RectRenderer.h"
#include "ShaderProgram.h"

// change to toggle debug statements on and off
#define debug debug_off

ShaderProgram shaderProgram;

const string vertShaderCode = "#version 330 core\n"
"layout (location = 0) in vec2 position; "
"layout (location = 1) in vec2 texturePositionIn; "
"out vec2 texturePosition; "
"uniform mat4 transform; "
"void main() "
"{ "
	"gl_Position = transform * vec4(position, 0.0f, 1.0f); "
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

RectRenderer::RectRenderer()
{
	if (shaderProgram.isNull())
	{
		shaderProgram.setupFromCode(vertShaderCode, fragShaderCode);
	}
	
	debug("setting up a VAO");
		
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
		
		glGenVertexArrays(1, &vertexArrayID);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		
		// Bind the Vertex Array Object first, then bind and set vertex buffer(s) and attribute pointer(s).
		glBindVertexArray(vertexArrayID);
		{
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			{
				glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
				
				glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
				glEnableVertexAttribArray(0);
				
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*) (2 * sizeof(GLfloat)));
				glEnableVertexAttribArray(1);
			}
			glBindBuffer(GL_ARRAY_BUFFER, false);
			// Note that this is allowed, the call to glVertexAttribPointer registered VBO as the currently bound vertex buffer object so afterwards we can safely unbind
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			{
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
			}
			//remember: do NOT unbind the EBO, keep it bound to this VAO
		}
		glBindVertexArray(false); // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs)
		
		// this is legal as per https://stackoverflow.com/a/13342549/4327513
		// the buffers will not really be deleted until the vertex array object is deleted
		// its apparently like reference counting, and here we are lowering the reference count
		// I know, OpenGL is fucking stupid
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
}

RectRenderer::~RectRenderer()
{
	debug("~RectRenderer called");
	glDeleteVertexArrays(1, &vertexArrayID);
}

void RectRenderer::draw(Texture texture, V2d pos, V2d size)
{
	shaderProgram.bind();
	{
		GLfloat transform[] = { // X and Y are flipped
			(GLfloat)size.x,			0.0,						0.0,	0.0,
			0.0,						(GLfloat)size.y,			0.0,	0.0,
			0.0,						0.0,						1.0,	0.0,
			GLfloat(pos.x*2+size.x-1),	GLfloat(pos.y*2+size.y-1),	0.0,	1.0,
		};
		shaderProgram.uniformMatrix4fv("transform", transform);
		
		texture.bind();
		{
			glBindVertexArray(vertexArrayID);
			{
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			}
			glBindVertexArray(false);
		}
		texture.unbind();
	}
	shaderProgram.unbind();
}
