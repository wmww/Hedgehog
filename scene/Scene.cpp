#include "Scene.h"
#include "../opengl/RectRenderer.h"

// change to toggle debug statements on and off
#define debug debug_off

Scene Scene::instance;

struct Scene::Impl: InputInterface
{
	vector<weak_ptr<WindowInterface>> windows;
	Texture cursorTexture;
	V2d cursorHotspot;
	RectRenderer renderer;
	
	weak_ptr<WindowInterface> getActiveWindow()
	{
		// TODO: OPTIMIZATION: do this well
		weak_ptr<WindowInterface> active;
		for (auto i: windows)
		{
			if (!i.expired())
				active = i;
		}
		return active;
	}
	
	void pointerMotion(V2d newPos)
	{
		if (auto window = getActiveWindow().lock())
		{
			auto input = window->getInputInterface().lock();
			ASSERT_ELSE(input, return);
			input->pointerMotion(newPos);
		}
	}
	
	void pointerLeave()
	{
		if (auto window = getActiveWindow().lock())
		{
			auto input = window->getInputInterface().lock();
			ASSERT_ELSE(input, return);
			input->pointerLeave();
		}
	}
	
	void pointerClick(uint button, bool down)
	{
		if (auto window = getActiveWindow().lock())
		{
			auto input = window->getInputInterface().lock();
			ASSERT_ELSE(input, return);
			input->pointerClick(button, down);
		}
	}
	
	void keyPress(uint key, bool down)
	{
		if (auto window = getActiveWindow().lock())
		{
			auto input = window->getInputInterface().lock();
			ASSERT_ELSE(input, return);
			input->keyPress(key, down);
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
	ASSERT_ELSE(impl, return);
	impl->windows.push_back(window);
}

void Scene::setCursor(Texture texture, V2d hotspot)
{
	debug("setting cursor");
	ASSERT_ELSE(impl, return);
	//impl->cursorTexture = texture;
	impl->cursorHotspot = hotspot;
}

weak_ptr<InputInterface> Scene::getInputInterface()
{
	ASSERT_ELSE(impl, return weak_ptr<InputInterface>());
	return impl;
}

void Scene::draw()
{
	ASSERT_ELSE(impl, return);
	//warning(to_string(impl->windows.size()) + " windows");
	for (auto i: impl->windows)
	{
		auto window = i.lock();
		if (window && window->texture.isValid())
		{
			impl->renderer.draw(window->texture, V2d(0.25, 0.25), V2d(0.5, 0.5));
			//mpl->renderer.draw(window->texture, V2d(0, 0), V2d(1, 1));
		}
	}
	if (impl->cursorTexture.isValid())
	{
		impl->renderer.draw(impl->cursorTexture, V2d(0, 0), V2d(0.5, 0.5));
	}
}
