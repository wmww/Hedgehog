#pragma once

#include "../main/util.h"
#include <wayland-util.h>

template<typename T>
class WlArray
{
public:
	WlArray()
	{
		array = make_unique<wl_array>();
		wl_array_init(&*array);
	}
	
	~WlArray()
	{
		wl_array_release(&*array);
	}
	
	void append(T item)
	{
		T * ptr = (T *)wl_array_add(&*array, sizeof(T));
		*ptr = item;
	}
	
	// returned value should not be stored long term as it will be invalid once this object dies
	wl_array * getRaw()
	{
		return &*array;
	}
	
private:
	unique_ptr<wl_array> array;
};

