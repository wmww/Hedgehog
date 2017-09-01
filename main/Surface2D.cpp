#include "Surface2D.h"

// change to toggle debug statements on and off
#define debug debug_off

struct Surface2D::Impl
{
	// instance data
	Texture texture;
	
	// class data
	static vector<Surface2D> allSurfaces;
	
	// functions
	Impl()
	{}
};

vector<Surface2D> Surface2D::Impl::allSurfaces;

Surface2D::Surface2D()
{
	impl = make_shared<Impl>();
	
	Impl::allSurfaces.push_back(Surface2D(impl));
}

void Surface2D::draw()
{
	assert(impl);
	impl->texture.draw();
}

void Surface2D::drawAll()
{
	debug("drawing all surfaces");
	
	for (Surface2D i: Impl::allSurfaces)
	{
		i.draw();
	}
}

void Surface2D::setTexture(Texture texture)
{
	assert(impl);
	impl->texture = texture;
}

Texture Surface2D::getTexture()
{
	assert(impl);
	return impl->texture;
}

