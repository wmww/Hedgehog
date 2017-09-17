#pragma once

#include "../main/util.h"

// there is almost no cost to creating a texture, initialization is lazy and done on first load
class Texture
{
public:
	Texture() {}
	
	void setupEmpty();
	void loadFromImage(string filepath);
	void loadFromData(void * data, V2i dim);
	void loadFromEGLImage(void * image, V2i dim);
	
	void bind();
	void unbind();
	
	GLuint getTextureId();
	inline bool isNull() { return impl == nullptr; };
	inline bool isValid() { return !isNull(); };
	
private:
	struct Impl;
	shared_ptr<Impl> impl;
};

