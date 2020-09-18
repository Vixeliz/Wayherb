#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "util.h"

void die(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(EXIT_FAILURE);

}

int os_create_anonymous_file(off_t size)
{
	static const char template[] = "/herbew-shared-XXXXXX";
	const char *path;
	char *name;
	int fd;
	int ret;

	path = getenv("XDG_RUNTIME_DIR");
	if (!path) {
		errno = ENOENT;
		return -1;
	}

	name = malloc(strlen(path) + sizeof(template));
	if (!name)
		return -1;

	strcpy(name, path);
	strcat(name, template);

	fd = create_tmpfile_cloexec(name);
	free(name);

	if (fd < 0)
		return -1;

	ret = posix_fallocate(fd, 0, size);
	if (ret != 0) {
		close(fd);
		errno = ret;
		return -1;
	}
	
	return fd;
}

int create_tmpfile_cloexec(char *tmpname)
{
	int fd;
#ifdef HAVE_MKOSTEMP
	fd = mkostemp(tmpname, O_CLOEXEC);

	if (fd >= 0)
		unlink(tmpname);
#else
	fd = mkstemp(tmpname);

	if (fd >= 0) {
		fd = set_cloexec_or_close(fd);
		unlink(tmpname);
	}
#endif
	return fd;

}

int set_cloexec_or_close(int fd)
{
	long flags;

	if (fd == -1)
		return -1;
	
	flags = fcntl(fd, F_GETFD);

	if (flags == -1)
		goto err;

	if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1)
		goto err;

	return fd;

err:
	close(fd);
	return -1;

}
