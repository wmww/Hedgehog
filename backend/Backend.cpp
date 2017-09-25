#include "Backend.h"

#include <xkbcommon/xkbcommon-x11.h>

// change to toggle debug statements on and off
#define debug debug_off

unique_ptr<Backend> Backend::instance;

unique_ptr<Backend> makeX11GLXBackend(V2i dim);
unique_ptr<Backend> makeX11EGLBackend(V2i dim);
unique_ptr<Backend> makeDRMBackend();

Backend::Backend()
{
	// TODO: don't leak keymap and context memory
	struct xkb_rule_names rules;
	// all these environment vars are empty on my setup
	rules.rules = getenv("XKB_DEFAULT_RULES");
	rules.model = getenv("XKB_DEFAULT_MODEL");
	rules.layout = getenv("XKB_DEFAULT_LAYOUT");
	rules.variant = getenv("XKB_DEFAULT_VARIANT");
	rules.options = getenv("XKB_DEFAULT_OPTIONS");
	struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	ASSERT_ELSE(context, return);
	xkb_keymap * keymap = xkb_map_new_from_names(context, &rules, XKB_KEYMAP_COMPILE_NO_FLAGS);
	ASSERT_THEN(keymap)
	{
		keymapString = xkb_keymap_get_as_string(keymap, XKB_KEYMAP_FORMAT_TEXT_V1);
	}
	if (keymapString == "")
		warning("keymap string is empty");
}

void Backend::setup(Type type)
{
	ASSERT_ELSE(instance == nullptr, return);
	static const V2i defaultDim = V2i(800, 800);
	switch (type)
	{
	case GLX:
		instance = makeX11GLXBackend(defaultDim);
		break;
	case EGL:
		instance = makeX11EGLBackend(defaultDim);
		break;
	case DRM:
		instance = makeDRMBackend();
		break;
	}
	ASSERT(instance);
}
