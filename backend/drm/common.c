/*
 * Copyright (c) 2017 Rob Clark <rclark@redhat.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "drm-common.h"

static struct gbm gbm;

#ifdef HAVE_GBM_MODIFIERS
static int
get_modifiers(uint64_t **mods)
{
	/* Assumed LINEAR is supported everywhere */
	static uint64_t modifiers[] = {DRM_FORMAT_MOD_LINEAR};
	*mods = modifiers;
	return 1;
}
#endif

const struct gbm * init_gbm(int drm_fd, int w, int h, uint64_t modifier)
{
	gbm.dev = gbm_create_device(drm_fd);

#ifndef HAVE_GBM_MODIFIERS
	if (modifier != DRM_FORMAT_MOD_INVALID) {
		fprintf(stderr, "Modifiers requested but support isn't available\n");
		return NULL;
	}
	gbm.surface = gbm_surface_create(gbm.dev, w, h,
			GBM_FORMAT_XRGB8888,
			GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
#else
	uint64_t *mods;
	int count;
	if (modifier != DRM_FORMAT_MOD_INVALID) {
		count = 1;
		mods = &modifier;
	} else {
		count = get_modifiers(&mods);
	}
	gbm.surface = gbm_surface_create_with_modifiers(gbm.dev, w, h,
			GBM_FORMAT_XRGB8888, mods, count);
#endif

	if (!gbm.surface) {
		printf("failed to create gbm surface\n");
		return NULL;
	}

	gbm.width = w;
	gbm.height = h;

	return &gbm;
}

int init_egl(struct egl *egl, const struct gbm *gbm)
{
	EGLint major, minor, n;

	static const EGLint context_attribs[] = {
		EGL_CONTEXT_MAJOR_VERSION_KHR, 3,
		EGL_CONTEXT_MINOR_VERSION_KHR, 3,
		EGL_NONE
	};

	static const EGLint config_attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RED_SIZE, 1,
		EGL_GREEN_SIZE, 1,
		EGL_BLUE_SIZE, 1,
		EGL_ALPHA_SIZE, 0,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
		EGL_NONE
	};

#define get_proc(name) do { \
		egl->name = (void *)eglGetProcAddress(#name); \
	} while (0)

	get_proc(eglGetPlatformDisplayEXT);
	get_proc(eglCreateImageKHR);
	get_proc(eglDestroyImageKHR);
	//get_proc(glEGLImageTargetTexture2DOES);
	get_proc(eglCreateSyncKHR);
	get_proc(eglDestroySyncKHR);
	get_proc(eglWaitSyncKHR);
	get_proc(eglDupNativeFenceFDANDROID);

	if (egl->eglGetPlatformDisplayEXT) {
		egl->display = egl->eglGetPlatformDisplayEXT(EGL_PLATFORM_GBM_KHR,
				gbm->dev, NULL);
	} else {
		egl->display = eglGetDisplay((void *)gbm->dev);
	}

	if (!eglInitialize(egl->display, &major, &minor)) {
		printf("failed to initialize\n");
		return -1;
	}

	printf("Using display %p with EGL version %d.%d\n",
			egl->display, major, minor);

	printf("===================================\n");
	printf("EGL information:\n");
	printf("  version: \"%s\"\n", eglQueryString(egl->display, EGL_VERSION));
	printf("  vendor: \"%s\"\n", eglQueryString(egl->display, EGL_VENDOR));
	printf("  extensions: \"%s\"\n", eglQueryString(egl->display, EGL_EXTENSIONS));
	printf("===================================\n");

	if (!eglBindAPI(EGL_OPENGL_API)) {
		printf("failed to bind api EGL_OPENGL_API\n");
		return -1;
	}

	if (!eglChooseConfig(egl->display, config_attribs, &egl->config, 1, &n) || n != 1) {
		printf("failed to choose config: %d\n", n);
		return -1;
	}

	egl->context = eglCreateContext(egl->display, egl->config,
			EGL_NO_CONTEXT, context_attribs);
	if (egl->context == NULL) {
		printf("failed to create context\n");
		return -1;
	}

	egl->surface = eglCreateWindowSurface(egl->display, egl->config,
			(EGLNativeWindowType)gbm->surface, NULL);
	if (egl->surface == EGL_NO_SURFACE) {
		printf("failed to create egl surface\n");
		return -1;
	}

	/* connect the context to the surface */
	eglMakeCurrent(egl->display, egl->surface, egl->surface, egl->context);

	printf("OpenGL information:\n");
	printf("  version: \"%s\"\n", glGetString(GL_VERSION));
	printf("  shading language version: \"%s\"\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	printf("  vendor: \"%s\"\n", glGetString(GL_VENDOR));
	printf("  renderer: \"%s\"\n", glGetString(GL_RENDERER));
	printf("  extensions: \"%s\"\n", glGetString(GL_EXTENSIONS));
	printf("===================================\n");
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glewInit();
	
	return 0;
}


#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

struct egl egl;
static const struct drm *drm;

int drmSetup()
{
	const char *device = "/dev/dri/card0";
	uint64_t modifier = DRM_FORMAT_MOD_INVALID;
	
	//drm = init_drm_atomic(device);
	drm = init_drm_legacy(device);
	
	if (!drm) {
		printf("failed to initialize DRM\n");
		return -1;
	}

	const struct gbm * ret = init_gbm(drm->fd, drm->mode->hdisplay, drm->mode->vdisplay,
			modifier);
	if (!ret) {
		printf("failed to initialize GBM\n");
		return -1;
	}

	int ret2 = init_egl(&egl, &gbm);
	
	if (ret2 != 0) {
		printf("failed to initialize EGL\n");
		return -1;
	}
	
	glViewport(0, 0, gbm.width, gbm.height);

	// clear the color buffer
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	
	drm_legacy_setup_with_egl(&gbm, &egl);
	//drm->run(&gbm, egl);
	
	return 0;
}

void drmSwapBuffers()
{
	drm_legacy_swap_buffers(&gbm, &egl);
}

void * drmGetEglDisplay()
{
	return egl.display;
}

void * drmGetEglContext()
{
	return egl.context;
}
