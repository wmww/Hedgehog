/*
 * Copyright © 2014 Red Hat, Inc.
 * Copyright (c) 2017 Drew DeVault
 * Copyright (c) 2014 Jari Vetoniemi
 * Copyright (c) 2017 William Wold
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

// note to the person trying to figure out the license situation on this file
// I have taken and heavily modified code from wlroots and official libinput examples (both licensed under MIT)
// I think the way you do that is keep one copy of the MIT bit and each copyright line.
// maybe there needs to be a separate license file instead of just the section above? idk.

#include "../../main/util.h"
#include "../../scene/InputInterface.h"

#include <libinput.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <string.h>

// change to toggle debug statements on and off
#define debug debug_on

bool stop = false;

struct {
	//struct wlr_backend backend;

	//struct wlr_session *session;
	struct udev *udev;
	struct udev_monitor *mon;
	struct wl_event_source *udev_event;
	
	struct wl_display *display;

	struct libinput *libinput_context;
	struct wl_event_source *input_event;

	//struct wl_listener session_signal;
	
	vector<int> wlr_device_vec;
	
	//list_t *wlr_device_lists;
} backend_data;

static const struct libinput_interface libinput_impl = {
	.open_restricted = +[](const char *path, int flags, void *_backend) -> int {
		debug("libinput_interface.open_restricted called, opening '" + string(path) + "'");
		int ret = open(path, flags);
		if (ret >= 0)
			return ret;
		else
			return -errno;
	},
	.close_restricted = +[](int fd, void *_backend) {
		debug("libinput_interface.close_restricted called");
		close(fd);
	}
};

static void wlr_libinput_log(struct libinput *libinput_context,
		enum libinput_log_priority priority, const char *fmt, va_list args) {
	char c_str[255];
	snprintf(c_str, 255, fmt, args);
	debug(c_str);
}

/*
static int wlr_libinput_readable(int fd, uint32_t mask, void *_backend) {
	struct wlr_libinput_backend *backend = (wlr_libinput_backend *)_backend;
	if (libinput_dispatch(backend->libinput_context) != 0) {
		debug("Failed to dispatch libinput");
		// TODO: some kind of abort?
		return 0;
	}
	struct libinput_event *event;
	while ((event = libinput_get_event(backend->libinput_context))) {
		//wlr_libinput_event(backend, event);
		debug("wlroots would have sent event here");
		libinput_event_destroy(event);
	}
	return 0;
}
*/

bool libinput_setup() {
	auto backend = &backend_data;
	debug("Initializing libinput");
	backend->udev = udev_new();
	ASSERT(backend->udev);
	backend->libinput_context = libinput_udev_create_context(&libinput_impl, backend, backend->udev);
	if (!backend->libinput_context) {
		debug("Failed to create libinput context");
		return false;
	}

	// TODO: Let user customize seat used
	if (libinput_udev_assign_seat(backend->libinput_context, "seat0") != 0) {
		debug("Failed to assign libinput seat");
		return false;
	}
	
	libinput_log_set_handler(backend->libinput_context, wlr_libinput_log);
	libinput_log_set_priority(backend->libinput_context, LIBINPUT_LOG_PRIORITY_ERROR);

	//int libinput_fd = libinput_get_fd(backend->libinput_context);
	
	/*
	if (backend->wlr_device_vec.size() == 0) {
		wlr_libinput_readable(libinput_fd, WL_EVENT_READABLE, backend);
		if (backend->wlr_device_vec.size() == 0) {
			debug("libinput initialization failed, no input devices");
			debug("Set WLR_LIBINPUT_NO_DEVICES=1 to suppress this check");
			return false;
		}
	}
	*/

	//struct wl_event_loop *event_loop =
	//	wl_display_get_event_loop(backend->display);
	//if (backend->input_event) {
	//	wl_event_source_remove(backend->input_event);
	//}
	//backend->input_event = wl_event_loop_add_fd(event_loop, libinput_fd,
	//		WL_EVENT_READABLE, wlr_libinput_readable, backend);
	//if (!backend->input_event) {
	//	debug("Failed to create input event on event loop");
	//	return false;
	//}
	debug("libinput sucessfully initialized");
	
	// struct pollfd fds;
	// fds.fd = libinput_get_fd(backend->libinput_context);
	// fds.events = POLLIN;
	// fds.revents = 0;

	// handle interrupts
	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_sigaction = +[](int signal, siginfo_t *siginfo, void *userdata) { stop = true; };
	act.sa_flags = SA_SIGINFO;
	if (sigaction(SIGINT, &act, NULL) == -1) {
		warning("Failed to set up signal handling (" + string(strerror(errno)) + ")");
		return false;
	}

	// Handle already-pending device added events
	//if (handle_and_print_events(li))
	//	fprintf(stderr, "Expected device added events on startup but got none. "
	//			"Maybe you don't have the right permissions?\n");

	// while (!stop && poll(&fds, 1, -1) > -1)
	
	return true;
	
	// udev_unref(session->udev);
	// libinput_unref(backend->libinput_context);
}

void libinput_destroy()
{
	libinput_unref(backend_data.libinput_context);
	// udev_unref(session->udev);
}

void libinput_check_events(InputInterface * interface)
{
	struct libinput_event *ev;
	auto backend = &backend_data;

	libinput_dispatch(backend->libinput_context);
	while ((ev = libinput_get_event(backend->libinput_context))) {
		//print_event_header(ev);
		
		switch (libinput_event_get_type(ev)) {
		case LIBINPUT_EVENT_NONE:
			debug("LIBINPUT_EVENT_NONE");
			abort();
		case LIBINPUT_EVENT_DEVICE_ADDED:
			debug("LIBINPUT_EVENT_DEVICE_ADDED");
			//print_device_notify(ev);
			//tools_device_apply_config(libinput_event_get_device(ev), &options);
			break;
		case LIBINPUT_EVENT_DEVICE_REMOVED:
			debug("LIBINPUT_EVENT_DEVICE_REMOVED");
			//print_device_notify(ev);
			//tools_device_apply_config(libinput_event_get_device(ev), &options);
			break;
		case LIBINPUT_EVENT_KEYBOARD_KEY:
			debug("LIBINPUT_EVENT_KEYBOARD_KEY");
			//print_key_event(li, ev);
			break;
		case LIBINPUT_EVENT_POINTER_MOTION:
			debug("LIBINPUT_EVENT_POINTER_MOTION");
			//print_motion_event(ev);
			break;
		case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
			debug("LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE");
			//print_absmotion_event(ev);
			break;
		case LIBINPUT_EVENT_POINTER_BUTTON:
			debug("LIBINPUT_EVENT_POINTER_BUTTON");
			//print_pointer_button_event(ev);
			break;
		case LIBINPUT_EVENT_POINTER_AXIS:
			debug("LIBINPUT_EVENT_POINTER_AXIS");
			//print_pointer_axis_event(ev);
			break;
		/*case LIBINPUT_EVENT_TOUCH_DOWN:
			//print_touch_event_with_coords(ev);
			break;
		case LIBINPUT_EVENT_TOUCH_MOTION:
			print_touch_event_with_coords(ev);
			break;
		case LIBINPUT_EVENT_TOUCH_UP:
			print_touch_event_without_coords(ev);
			break;
		case LIBINPUT_EVENT_TOUCH_CANCEL:
			print_touch_event_without_coords(ev);
			break;
		case LIBINPUT_EVENT_TOUCH_FRAME:
			print_touch_event_without_coords(ev);
			break;
		case LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN:
			print_gesture_event_without_coords(ev);
			break;
		case LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE:
			print_gesture_event_with_coords(ev);
			break;
		case LIBINPUT_EVENT_GESTURE_SWIPE_END:
			print_gesture_event_without_coords(ev);
			break;
		case LIBINPUT_EVENT_GESTURE_PINCH_BEGIN:
			print_gesture_event_without_coords(ev);
			break;
		case LIBINPUT_EVENT_GESTURE_PINCH_UPDATE:
			print_gesture_event_with_coords(ev);
			break;
		case LIBINPUT_EVENT_GESTURE_PINCH_END:
			print_gesture_event_without_coords(ev);
			break;
		case LIBINPUT_EVENT_TABLET_TOOL_AXIS:
			print_tablet_axis_event(ev);
			break;
		case LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY:
			print_proximity_event(ev);
			break;
		case LIBINPUT_EVENT_TABLET_TOOL_TIP:
			print_tablet_tip_event(ev);
			break;
		case LIBINPUT_EVENT_TABLET_TOOL_BUTTON:
			print_tablet_button_event(ev);
			break;
		case LIBINPUT_EVENT_TABLET_PAD_BUTTON:
			print_tablet_pad_button_event(ev);
			break;
		case LIBINPUT_EVENT_TABLET_PAD_RING:
			print_tablet_pad_ring_event(ev);
			break;
		case LIBINPUT_EVENT_TABLET_PAD_STRIP:
			print_tablet_pad_strip_event(ev);
			break;
		case LIBINPUT_EVENT_SWITCH_TOGGLE:
			print_switch_event(ev);
			break;*/
		default:
			debug("other libinput event");
			break;
		}

		libinput_event_destroy(ev);
		libinput_dispatch(backend->libinput_context);
	}
}

