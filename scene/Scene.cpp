#include "Scene.h"

// change to toggle debug statements on and off
#define debug debug_off

Scene Scene::instance;

struct Scene::Impl: InputInterface
{
	vector<weak_ptr<WindowInterface>> windows;
	
	void pointerMotion(V2d newPos)
	{
		if (windows.size() < 1)
		{
			// do nothing
		}
		else
		{
			auto window = windows[windows.size() - 1].lock();
			assert(window);
			auto input = window->getInputInterface().lock();
			if (!input)
			{
				warning("input null in " + FUNC);
				return;
			}
			input->pointerMotion(newPos);
		}
	}
	
	void pointerLeave()
	{
		if (windows.size() < 1)
		{
			// do nothing
		}
		else
		{
			auto window = windows[windows.size() - 1].lock();
			assert(window);
			auto input = window->getInputInterface().lock();
			assert(input);
			input->pointerLeave();
		}
	}
	
	void pointerClick(uint button, bool down)
	{
		if (windows.size() < 1)
		{
			// do nothing
		}
		else
		{
			auto window = windows[windows.size() - 1].lock();
			assert(window);
			auto input = window->getInputInterface().lock();
			assert(input);
			input->pointerClick(button, down);
		}
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
