#include <string.h>
#include "wayland.h"
#include "shm.h"

//Local globals
static struct wayland_state wayland;
static uint32_t output = UINT32_MAX;
static int cur_x = -1, cur_y = -1;
static int button = 0;

//Listeners and handles
static void wl_pointer_enter(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t surface_x, wl_fixed_t surface_y)
{
	struct wl_cursor_image *image;
	image = wayland.cursor_image;

	wl_surface_attach(wayland.cursor_surface, wl_cursor_image_get_buffer(image), 0, 0);
	wl_surface_damage(wayland.cursor_surface, 1, 0, image->width, image->height);
	wl_pointer_set_cursor(wl_pointer, serial, wayland.cursor_surface, image->hotspot_x, image->hotspot_y);
	wayland.input_surface = surface;
}

static void wl_pointer_leave(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface)
{
	cur_x = cur_y = -1;
	button = 0;
}	

static void wl_pointer_motion(void *data, struct wl_pointer *wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y)
{
	cur_x = wl_fixed_to_int(surface_x);
	cur_y = wl_fixed_to_int(surface_y);
}

static void wl_pointer_button(void *data, struct wl_pointer *wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{

}

struct wl_pointer_listener pointer_listener = {
	.enter = wl_pointer_enter,
	.leave = wl_pointer_leave,
	.motion = wl_pointer_motion,
	.button = wl_pointer_button,
};

static void seat_handle_capabilities(void *data, struct wl_seat *wl_seat, enum wl_seat_capability caps)
{
	if((caps & WL_SEAT_CAPABILITY_POINTER)) {
		wayland.pointer = wl_seat_get_pointer(wl_seat);
		wl_pointer_add_listener(wayland.pointer, &pointer_listener, NULL);
	}
}

const struct wl_seat_listener seat_listener = {
	.capabilities = seat_handle_capabilities,
};

//Registry
void registry_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{

	if (strcmp(interface, wl_compositor_interface.name) == 0) {
		wayland.compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 1);
	} else if (strcmp(interface, wl_shm_interface.name) == 0) {
		wayland.shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
	} else if (strcmp(interface, "wl_output") == 0) {
		if (output != UINT32_MAX) {
			if (!wayland.wl_output) {
				wayland.wl_output = wl_registry_bind(registry, name, &wl_output_interface, 1);
			} else {
				output --;
			}
		}
	} else if (strcmp(interface, wl_seat_interface.name) == 0) {
		wayland.seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
		wl_seat_add_listener(wayland.seat, &seat_listener, NULL);
	} else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
		wayland.layer_shell = wl_registry_bind(registry, name, &zwlr_layer_shell_v1_interface, 1);
	}

}

void registry_handle_remove(void *data, struct wl_registry *registry, uint32_t name)
{


}

static const struct wl_registry_listener registry_listener = {
	.global = registry_global,
	.global_remove = registry_handle_remove,
};
