#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <semaphore.h>
#include <wayland-client.h>
#include <wayland-server.h>
#include <wayland-client-protocol.h>
#include <wayland-egl.h>

#include "wlr-layer-shell-unstable-v1.h"

#include "config.h"

//Wayland registry handling functions
struct wl_display *display = NULL;
struct wl_compositor *compositor = NULL;
struct wl_surface *surface;
struct zwlr_layer_shell_v1 *layer_shell;

void global_add(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
	//Bind compositor to our registry and our shell
	printf("Got a registry event for %s name %d\n", interface, name);
	if (strcmp(interface, "wl_compositor") == 0) {
		compositor = wl_registry_bind(registry,
				name,
				&wl_compositor_interface,
				1);
	} else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
		layer_shell = wl_registry_bind(registry,
				name,
				&zwlr_layer_shell_v1_interface,
				1);
	}

}

void global_remove(void *data, struct wl_registry *registry, uint32_t name)
{
	printf("Got a registry losing event for %d\n", name);
}

struct wl_registry_listener registry_listener = {
	.global = global_add,
	.global_remove = global_remove
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

	//Connect to the wayland server	
	display = wl_display_connect(NULL);
	if (!display) {
		die("Couldn't connect to wayland display");
	}

	printf("Connected to wayland display\n");
	
	//Get the registry for accessing event's from the wayland display
	struct wl_registry *registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, NULL);

	wl_display_dispatch(display);
	wl_display_roundtrip(display);

	if(!compositor) {
		die("Couldn't connect to compositor\n");
	} else {
		fprintf(stdout, "Connected to compositor\n");
	}

	surface = wl_compositor_create_surface(compositor);
	if(!surface) {
		die("Couldn't create a surface\n");
	} else {
		fprintf(stdout, "Created a surface!\n");
	}
	
	sem_t *mutex = sem_open("/herbew", O_CREAT, 0644, 1);
	sem_wait(mutex);
	
	sigaction(SIGUSR1, &act_expire, 0);
	sigaction(SIGUSR2, &act_expire, 0);

	if(duration != 0) {
		alarm(duration);
	}

	//for(;;) {
	//}

	sem_post(mutex);
	sem_close(mutex);

	wl_display_disconnect(display);
	printf("disconnected from wayland display\n");

	exit(EXIT_SUCCESS);
}
