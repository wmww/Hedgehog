#include "Surface2D.h"

struct Surface2D::Impl
{
	Texture texture;
	
	static vector<Surface2D> allSurfaces;
};

vector<Surface2D> Surface2D::Impl::allSurfaces;

Surface2D::Surface2D()
{
	impl = shared_ptr<Impl>(new Impl);
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

