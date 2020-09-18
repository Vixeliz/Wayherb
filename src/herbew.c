#include <semaphore.h>
#include <stdlib.h>
#include "util.h"
#include "wayland.h"

int main(int argc, char *argv[])
{
	if (argc == 1) {
		sem_unlink("/herbew");
		die("Usage: %s body", argv[0]);
	}

	exit(EXIT_SUCCESS);

}
