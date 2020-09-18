#include "wayland.h"
#include "shm.h"

void registry_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{

}

void registry_handle_remove(void *data, struct wl_registry *registry, uint32_t name)
{


}

static const struct wl_registry_listener registry_listener = {
	.global = registry_global,
	.global_remove = registry_handle_remove,
};
