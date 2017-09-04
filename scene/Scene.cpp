#include "Scene.h"

// change to toggle debug statements on and off
#define debug debug_on

Scene Scene::instance;

struct Scene::Impl: InputInterface
{
	vector<weak_ptr<WindowInterface>> windows;
	
	void pointerMotion(V2d newPos)
	{
		debug("pointer moved to " + to_string(newPos));
	}
};

void Scene::setup()
{
	impl = make_shared<Impl>();
	assert(instance.impl == nullptr);
	instance.impl = impl;
}

void Scene::addWindow(weak_ptr<WindowInterface> window)
{
	debug("adding window to scene");
	assert(window.lock() != nullptr);
	assert(impl);
	impl->windows.push_back(window);
}

weak_ptr<InputInterface> Scene::getInputInterface()
{
	assert(impl);
	return impl;
}

void Scene::draw()
{
	assert(impl);
	for (auto i: impl->windows)
	{
		if (auto window = i.lock())
		{
			window->texture.draw();
		}
	}
}
