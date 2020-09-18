#include <semaphore.h>
#include <signal.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include "util.h"
#include "wayland.h"
#include "config.h"

#define EXIT_ACTION 0
#define EXIT_FAIL 1
#define EXIT_DISMISS 2

//Local variables
int exit_code = EXIT_DISMISS;
int should_exit = 0;

void expire(int sig)
{
	if (sig == SIGUSR2) {
		exit_code = EXIT_ACTION;
		should_exit = 1;
	} else if (sig == SIGUSR1) {
		should_exit = 1;
	} else {
		should_exit = 0;
	}
}

int main(int argc, char *argv[])
{
	if (argc == 1) {
		sem_unlink("/herbew");
		die("Usage: %s body", argv[0]);
	}
	
	struct sigaction act_expire, act_ignore;
	act_expire.sa_handler = expire;
	act_expire.sa_flags = SA_RESTART;
	sigemptyset(&act_expire.sa_mask);
	
	act_ignore.sa_handler = SIG_IGN;
	act_ignore.sa_flags = SA_RESTART;
	sigemptyset(&act_ignore.sa_mask);
	
	sigaction(SIGALRM, &act_expire, 0);
	sigaction(SIGTERM, &act_expire, 0);
	sigaction(SIGINT, &act_expire, 0);
	
	sigaction(SIGUSR1, &act_ignore, 0);
	sigaction(SIGUSR2, &act_ignore, 0);
	
	init_wayland();
	
	sem_t *mutex = sem_open("/herbew", O_CREAT, 0644, 1);
	sem_wait(mutex);
	
	sigaction(SIGUSR1, &act_expire, 0);
	sigaction(SIGUSR2, &act_expire, 0);

	if (duration != 0)
		alarm(duration);

	for (;;) {
		if (should_exit == 0) {
			fprintf(stdout, "This is a test\n");
		} else if (should_exit == 1) {
		break;
		}
	}

	sem_post(mutex);
	sem_close(mutex);
	
	free_wayland();
	return exit_code;
}
