#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>

void die(const char *format, ...);
int os_create_anonymous_file(off_t size);
int create_tmpfile_cloexec(char *tmpname);
int set_cloexec_or_close(int fd);

#endif 
