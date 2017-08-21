#include "Surface2D.h"

struct Surface2D::Impl: MessageLogger
{
	Texture texture;
	
	Impl(VerboseToggle verboseToggle): texture(VERBOSE_OFF)
	{
		verbose = verboseToggle;
		tag = "Surface2D";
	}
};

Surface2D::Surface2D(VerboseToggle verboseToggle)
{
	impl = shared_ptr<Impl>(new Impl(verboseToggle));
}

Texture Surface2D::getTexture()
{
	return impl->texture;
}

void Surface2D::draw()
{
	impl->texture.draw();
}

