#ifndef WAYLAND_H
#define WAYLAND_H

#include <wayland-client.h>
#include "wlr-layer-shell-unstable-v1.h"

struct wayland {
	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_compositor *compositor;
	struct wl_shm *shm;
	struct zwlr_layer_shell_v1 *layer_shell;
	struct zwlr_layer_surface_v1 *layer_surface;
	struct wl_output *output;
	struct wl_surface *surface;
};

int init_wayland(struct wayland);
int free_wayland(struct wayland);

#endif
