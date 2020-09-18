#include <string.h>
#include <stdbool.h>
#include "wayland.h"
#include "config.h"
#include "util.h"
#include "shm.h"

//Local globals
static struct wayland_state wayland;
static uint32_t output = UINT32_MAX;
static int cur_x = -1, cur_y = -1;
static int button = 0;
static bool run_display = true;
static uint32_t layer = ZWLR_LAYER_SHELL_V1_LAYER_TOP;
static bool keyboard_interactive = false;

/* Listeners and handles */

//Layer_shell listener

static void layer_surface_configure(void *data, struct zwlr_layer_surface_v1 *surface, uint32_t serial, uint32_t w, uint32_t h)
{
	zwlr_layer_surface_v1_ack_configure(surface, serial);
}

static void layer_surface_closed(void *data, struct zwlr_layer_surface_v1 *surface)
{
	zwlr_layer_surface_v1_destroy(surface);
	wl_surface_destroy(wayland.wl_surface);
	run_display = false;
}

struct zwlr_layer_surface_v1_listener layer_surface_listener = {
	.configure = layer_surface_configure,
	.closed = layer_surface_closed,
};

//Pointer listeners and events
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

//Seat listener
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

/* Wayland intialization, freeing, and main loop for drawing and input. */
void draw()
{

}

int init_wayland(void)
{
	//Some variables for setting up our layer_surface
	char *namespace = "herbew";
	int exclusive_zone = height;

	//Connect to the wayland server
	wayland.display = wl_display_connect(NULL);
	if (!wayland.display)
		die("Failed to create display\n");

	//Set up our registry for getting events
	struct wl_registry *registry = wl_display_get_registry(wayland.display);
	wl_registry_add_listener(registry, &registry_listener, NULL);
	wl_display_roundtrip(wayland.display);
	
	if (!wayland.compositor)
		die("No compositor available\n");

	if (!wayland.shm)
		die("Shared memory buffer not available\n");

	if (!wayland.layer_shell)
		die("Layer shell not available\n");
	
	//Cursor image for being able to see the cursor over our notifcations
	wayland.cursor_theme = wl_cursor_theme_load(NULL, 16, wayland.shm);
	//struct wl_cursor *cursor = wl_cursor_theme_get_cursor(wayland.cursor_theme, "left ptr");
	//wayland.cursor_image = cursor->images[0];
	wayland.cursor_surface = wl_compositor_create_surface(wayland.compositor);
	
	//Our surfaces for our notifications
	wayland.wl_surface = wl_compositor_create_surface(wayland.compositor);
	wayland.layer_surface = zwlr_layer_shell_v1_get_layer_surface(wayland.layer_shell, wayland.wl_surface, wayland.wl_output, layer, namespace);

	//Configure how layer surface acts
	zwlr_layer_surface_v1_set_size(wayland.layer_surface, width, height);
	zwlr_layer_surface_v1_set_anchor(wayland.layer_surface, anchor);
	zwlr_layer_surface_v1_set_exclusive_zone(wayland.layer_surface, exclusive_zone);
	zwlr_layer_surface_v1_set_margin(wayland.layer_surface, margin_top, margin_right, margin_bottom, margin_left);
	zwlr_layer_surface_v1_set_keyboard_interactivity(wayland.layer_surface, keyboard_interactive);
	zwlr_layer_surface_v1_add_listener(wayland.layer_surface, &layer_surface_listener, wayland.layer_surface);

	return 1;
}

int free_wayland(void)
{

return 1;
}
