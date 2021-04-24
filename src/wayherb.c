#include <semaphore.h>
#include <signal.h>
#include <fcntl.h>
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
	} else if (sig == SIGALRM) {
		should_exit = 1;
	} else {
                should_exit = 0;
        }
}

void time_set (struct timespec *t, uint64_t seconds, uint64_t nanosec) {
  t->tv_sec = seconds;
  t->tv_nsec = nanosec;
}

void time_elapsed (struct timespec *c, const struct timespec *a, const struct timespec *b) {
  c->tv_sec = a->tv_sec - b->tv_sec;
  if (b->tv_nsec > a->tv_nsec)
    {
      c->tv_sec--;
      c->tv_nsec = 1000000000;
      c->tv_nsec += a->tv_nsec;
      c->tv_nsec -= b->tv_nsec;
    }
  else
    {
      c->tv_nsec = a->tv_nsec - b->tv_nsec;
    }
}

bool time_lessthan(const struct timespec *a, const struct timespec *b) {
        return a->tv_sec == b->tv_sec ?
            a->tv_nsec < b->tv_nsec :
            a->tv_sec < b->tv_sec;
}

int main(int argc, char *argv[])
{
/*
	if (argc == 1) {
		sem_unlink("/wayherb");
		die("Usage: %s body", argv[0]);
	}
	
	if (argc > 2) {
		sem_unlink("/wayherb");
		die("Currently wayherb only allows one argument however this will change\n");
	}
*/
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
	
	//sem_t *mutex = sem_open("/wayherb", O_CREAT, 0644, 1);
	//sem_wait(mutex);
	
	sigaction(SIGUSR1, &act_expire, 0);
	sigaction(SIGUSR2, &act_expire, 0);
	
        struct timespec last_frame;
        struct timespec current_frame;
        struct timespec frame_delta;
        struct timespec sleep_time;
        struct timespec limit;

        memset(&last_frame, 0, sizeof(struct timespec));
        memset(&current_frame, 0, sizeof(struct timespec));
        memset(&frame_delta, 0, sizeof(struct timespec));
        memset(&sleep_time, 0, sizeof(struct timespec));
        memset(&limit, 0, sizeof(struct timespec));

        time_set(&limit, 0, 40000000);

        clock_settime(CLOCK_MONOTONIC_RAW, &last_frame);
        current_frame = last_frame;

        init_wayland(argc, argv);

	if (duration != 0)
		alarm(duration);
	
	
	for (;;) {
            while(should_exit == 0) {
                draw();

                last_frame = current_frame;
                clock_settime(CLOCK_MONOTONIC_RAW, &current_frame);
                time_elapsed(&frame_delta, &current_frame, &last_frame);
                if(time_lessthan(&frame_delta, &limit)) {
                    time_elapsed(&sleep_time, &limit, &frame_delta);
                    nanosleep(&sleep_time, NULL);
                }
            }

		break;
	}

	//sem_post(mutex);
	//sem_close(mutex);
	
	free_wayland();
	return exit_code;
}
