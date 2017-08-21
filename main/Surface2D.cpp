#include "Surface2D.h"

struct Surface2D::Impl: MessageLogger
{
	Texture texture;
	
	static vector<Surface2D> allSurfaces;
	
	Impl(VerboseToggle verboseToggle): texture(VERBOSE_OFF)
	{
		verbose = verboseToggle;
		tag = "Surface2D";
	}
};

vector<Surface2D> Surface2D::Impl::allSurfaces;

Surface2D::Surface2D(VerboseToggle verboseToggle)
{
	impl = shared_ptr<Impl>(new Impl(verboseToggle));
	Impl::allSurfaces.push_back(*this);
}

Texture Surface2D::getTexture()
{
	return impl->texture;
}

void Surface2D::draw()
{
	impl->texture.draw();
}

void Surface2D::drawAll()
{
	for (Surface2D i: Impl::allSurfaces)
	{
		i.draw();
	}
}

