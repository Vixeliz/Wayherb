#include <GLES2/gl2.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include "assert.h"
#include <errno.h>
#include <stdarg.h>
#include <fcntl.h>
#include <semaphore.h>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <wayland-egl.h>
#include <wlr/render/egl.h>
#include <wlr/util/log.h>

#include "wlr-layer-shell-unstable-v1.h"
#include "xdg-shell.h"

#include "config.h"

static struct wl_display *display;
static struct wl_output *wl_output;
static struct wl_compositor *compositor;
static struct wl_seat *seat;
static struct wl_shm *shm;
static struct wl_pointer *pointer;
static struct wl_keyboard *keyboard;
static struct xdg_wm_base *xdg_wm_base;
static struct zwlr_layer_shell_v1 *layer_shell;

static uint32_t output = UINT32_MAX;
struct zwlr_layer_surface_v1 *layer_surface;
struct wl_surface *wl_surface;
struct wlr_egl egl;
struct wl_egl_window *egl_window;
struct wlr_egl_surface *egl_surface;
struct wl_callback *frame_callback;

static uint32_t layer = ZWLR_LAYER_SHELL_V1_LAYER_TOP;
static uint32_t anchor = 0;
static uint32_t width = 450;
static uint32_t height = 150;
static int32_t margin_top = 0;
static double alpha = 1.0;
static bool run_display = true;
static bool keyboard_interactive = false;
static int cur_x = -1, cur_y = -1;
struct wl_cursor_image *cursor_image;
struct wl_surface *cursor_surface, *input_surface;

static void draw(void);

static void draw(void) {
	eglMakeCurrent(egl.display, egl_surface, egl_surface, egl.context);

	glViewport(0, 0, width, height);
	glClearColor(0.25, 0.25, 0.25, alpha);
	glClear(GL_COLOR_BUFFER_BIT);

	if (cur_x != -1 && cur_y != -1) {
		glEnable(GL_SCISSOR_TEST);
		glScissor(cur_x, height - cur_y, 5, 5);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);
	}

	eglSwapBuffers(egl.display, egl_surface);
}

static void layer_surface_configure(void *data, struct zwlr_layer_surface_v1 *surface, uint32_t serial, uint32_t w, uint32_t h)
{
	width = w;
	height = h;
	if (egl_window) {
		wl_egl_window_resize(egl_window, width, height, 0, 0);
	}
	zwlr_layer_surface_v1_ack_configure(surface, serial);
}

static void layer_surface_closed(void *data, struct zwlr_layer_surface_v1 *surface)
{
	//wlr_egl_destroy_surface(&egl, egl_surface);
	wl_egl_window_destroy(egl_window);
	zwlr_layer_surface_v1_destroy(surface);
	wl_surface_destroy(wl_surface);
	run_display = false;
}

struct zwlr_layer_surface_v1_listener layer_surface_listener = {
	.configure = layer_surface_configure,
	.closed = layer_surface_closed,
};

static void wl_pointer_enter(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t surface_x, wl_fixed_t surface_y)
{
	struct wl_cursor_image *image;
	image = cursor_image;
	
	wl_surface_attach(cursor_surface, wl_cursor_image_get_buffer(image), 0, 0);
	wl_surface_damage(cursor_surface, 1, 0, image->width, image->height);
	wl_surface_commit(cursor_surface);
	wl_pointer_set_cursor(wl_pointer, serial, cursor_surface, image->hotspot_x, image->hotspot_y);
	input_surface = surface;
}

static void wl_pointer_leave(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface)
{
	cur_x = cur_y = -1;
}

static void wl_pointer_button(void *data, struct wl_pointer *wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
 //Todo implement right click dismissing
}



static void wl_pointer_motion(void *data, struct wl_pointer *wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y)
{
	cur_x = wl_fixed_to_int(surface_x);
	cur_y = wl_fixed_to_int(surface_y);
}

static void wl_pointer_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value)
{
//Not going to implement
}

static void wl_pointer_frame(void *data, struct wl_pointer *wl_pointer)
{
//Not going to implement
}

static void wl_pointer_axis_source(void *data, struct wl_pointer *wl_pointer, uint32_t axis_source)
{
//Not going to implement
}


static void wl_pointer_axis_stop(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis)
{
//Not going to implement
}

static void wl_pointer_axis_discrete(void *data, struct wl_pointer *wl_pointer, uint32_t axis, int32_t discrete)
{
//Not going to implement
}

struct wl_pointer_listener pointer_listener = {
	.enter = wl_pointer_enter,
	.leave = wl_pointer_leave,
	.motion = wl_pointer_motion,
	.button = wl_pointer_button,
	.axis = wl_pointer_axis,
	.frame = wl_pointer_frame,
	.axis_source = wl_pointer_axis_source,
	.axis_stop = wl_pointer_axis_stop,
	.axis_discrete = wl_pointer_axis_discrete,
};

static void wl_keyboard_keymap(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t format, int32_t fd, uint32_t size) {
	// Not needed
}

static void wl_keyboard_enter(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t serial, struct wl_surface *surface, struct wl_array *keys) {
	//dont need
}

static void wl_keyboard_leave(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t serial, struct wl_surface *surface) {
	//dont need
}

static void wl_keyboard_key(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
	//Dont need
}

static void wl_keyboard_modifiers(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched,
		uint32_t mods_locked, uint32_t group) {
	// Not needed
}

static void wl_keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard,
		int32_t rate, int32_t delay) {
	// Not needed
}

static struct wl_keyboard_listener keyboard_listener = {
	.keymap = wl_keyboard_keymap,
	.enter = wl_keyboard_enter,
	.leave = wl_keyboard_leave,
	.key = wl_keyboard_key,
	.modifiers = wl_keyboard_modifiers,
	.repeat_info = wl_keyboard_repeat_info,
};

static void seat_handle_capabilities(void *data, struct wl_seat *wl_seat, enum wl_seat_capability caps)
{
	if ((caps & WL_SEAT_CAPABILITY_POINTER)) {
		pointer = wl_seat_get_pointer(wl_seat);
		wl_pointer_add_listener(pointer, &pointer_listener, NULL);
	}
	if ((caps & WL_SEAT_CAPABILITY_KEYBOARD)) {
		keyboard = wl_seat_get_keyboard(wl_seat);
		wl_keyboard_add_listener(keyboard, &keyboard_listener, NULL);
	}
}

static void seat_handle_name(void *data, struct wl_seat *wl_seat, const char *name)
{
	//Uneeded
}

const struct wl_seat_listener seat_listener = {
	.capabilities = seat_handle_capabilities,
	.name = seat_handle_name,
};

//Handle wayland registry and assigning
static void handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
	if (strcmp(interface, wl_compositor_interface.name) == 0) {
		compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 1);
	} else if (strcmp(interface, wl_shm_interface.name) == 0) {
		shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
	} else if (strcmp(interface, "wl_output") == 0) {
		if (output != UINT32_MAX) {
			if (!wl_output) {
				wl_output = wl_registry_bind(registry, name, &wl_output_interface, 1);
			} else {
				output--;
			}
		}
	} else if (strcmp(interface, wl_seat_interface.name) == 0) {
		seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
		wl_seat_add_listener(seat, &seat_listener, NULL);
	} else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
		layer_shell = wl_registry_bind(registry, name, &zwlr_layer_shell_v1_interface, 1);
	} else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
		xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
	}
}

static void handle_global_remove(void *data, struct wl_registry * registry, uint32_t name)
{
	//Don't worry about this for now
}

static const struct wl_registry_listener registry_listener = {
	.global = handle_global,
	.global_remove = handle_global_remove,
};

static void die(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(EXIT_FAILURE);

}

void expire(int sig)
{
	fprintf(stdout, "Notifcation Success\n");
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	//Handle arguments
	if (argc == 1) {
		sem_unlink("/herbew");
		die("Usage: %s body", argv[0]);
	}

	struct sigaction act_expire, act_ignore;

	act_expire.sa_handler = expire;
	act_expire.sa_flags = SA_RESTART;
	sigemptyset(&act_expire.sa_mask);

	act_ignore.sa_handler = SIG_IGN;
	act_ignore.sa_flags = 0;
	sigemptyset(&act_ignore.sa_mask);

	sigaction(SIGALRM, &act_expire, 0);
	sigaction(SIGTERM, &act_expire, 0);
	sigaction(SIGINT, &act_expire, 0);

	sigaction(SIGUSR1, &act_ignore, 0);
	sigaction(SIGUSR2, &act_ignore, 0);
	
	//Init wayland stuff
	char *namespace = "herbew";
	int exclusive_zone = height;
	int32_t margin_right = 0, margin_bottom = 0, margin_left = 0;
	bool found;
	anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP + ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;

	display = wl_display_connect(NULL);
	if (!display) {
		die("Failed to create display\n");
	}
	
	struct wl_registry *registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, NULL);
	wl_display_roundtrip(display);

	if (!compositor) {
		die("No compositor available\n");
	}

	if (!shm) {
		die("Shared memory buffer not available\n");
	}

	if (!layer_shell) {
		die("No layer shell available\n");
	}

	struct wl_cursor_theme *cursor_theme = wl_cursor_theme_load(NULL, 16, shm);
	assert(cursor_theme);
	struct wl_cursor *cursor = wl_cursor_theme_get_cursor(cursor_theme, "left_ptr");
	assert(cursor);
	cursor_image = cursor->images[0];
	
	cursor = wl_cursor_theme_get_cursor(cursor_theme, "left_ptr");
	assert(cursor);
	
	cursor_surface = wl_compositor_create_surface(compositor);
	assert(cursor_surface);

	EGLint attribs[] = { EGL_ALPHA_SIZE, 8, EGL_NONE };
	wlr_egl_init(&egl, EGL_PLATFORM_WAYLAND_EXT, display, attribs, WL_SHM_FORMAT_ARGB8888);

	wl_surface = wl_compositor_create_surface(compositor);
	assert(wl_surface);

	layer_surface = zwlr_layer_shell_v1_get_layer_surface(layer_shell, wl_surface, wl_output, layer, namespace);
	assert(layer_surface);

	zwlr_layer_surface_v1_set_size(layer_surface, width, height);
	zwlr_layer_surface_v1_set_anchor(layer_surface, anchor);
	zwlr_layer_surface_v1_set_exclusive_zone(layer_surface, exclusive_zone);
	zwlr_layer_surface_v1_set_margin(layer_surface, margin_top, margin_right, margin_bottom, margin_left);
	zwlr_layer_surface_v1_set_keyboard_interactivity(layer_surface, keyboard_interactive);
	zwlr_layer_surface_v1_add_listener(layer_surface, &layer_surface_listener, layer_surface);
	wl_surface_commit(wl_surface);
	wl_display_roundtrip(display);

	egl_window = wl_egl_window_create(wl_surface, width, height);
	assert(egl_window);
	egl_surface = wlr_egl_create_surface(&egl, egl_window);
	assert(egl_surface);

	wl_display_roundtrip(display);
	draw();

	sigaction(SIGUSR1, &act_expire, 0);
	sigaction(SIGUSR2, &act_expire, 0);

	sem_t *mutex = sem_open("/herbew", O_CREAT, 0644, 1);
	sem_wait(mutex);

	if(duration != 0) {
		alarm(duration);
	}

	for(;;) {
		//Draw Here
	}

	sem_post(mutex);
	sem_close(mutex);

	wl_cursor_theme_destroy(cursor_theme);
	exit(EXIT_SUCCESS);
}
