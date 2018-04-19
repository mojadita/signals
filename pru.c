#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define F(fmt) "[%d]:"__FILE__":%d: " fmt, getpid(), __LINE__

#define FL_KILL		1
#define FL_FORK		2

int flags = 0;

int main(int argc, char **argv)
{
	int opt;
	while ((opt = getopt(argc, argv, "kf")) != EOF) {
		switch (opt) {
		case 'k': flags |= FL_KILL; break;
		case 'f': flags |= FL_FORK; break;
		}
	}

	pid_t child;

	if (flags & FL_FORK) {
		printf(F("about to fork()\n"));
		if ((child = fork()) > 0) { /* parent */
			int status;		
			printf(F("about to wait()\n"));
			if ((child = wait(&status)) < 0) {
				fprintf(stderr, F("wait: %s (errno = %d)\n"),
					strerror(errno), errno);
				exit(EXIT_FAILURE);
			}
			if (WIFEXITED(status)) {
				printf(F("WAIT: child <%d> finished with status = %d\n"),
					child, WEXITSTATUS(status));
			} else if (WIFSIGNALED(status)) {
				printf(F("WAIT: child <%d> was signalled (signal %d)\n"),
					child, WTERMSIG(status));
			} else {
				printf(F("WAIT: child <%d> neither exit'ed, nor was"
					" signalled (shouldn't happen)\n"), child);
			}
		} else if (child == 0) { /* child */
			int secs = 10;
			if (flags & FL_KILL) {
				printf(F("setting an alarm of %d sec.\n"), secs);
				alarm(secs);
				printf(F("...and waiting for the signal to arrive\n"));
				pause();
			} else {
				printf(F("waiting for the delay to finish\n"));
				sleep(secs);
			}
		}
		/* parent & child */
		printf(F("ALL OK, ppid = %d\n"), getppid());
	} else {
		int status;		
		printf(F("about to wait() (no fork)\n"));
		if ((child = wait(&status)) < 0) {
			fprintf(stderr, F("wait: %s (errno = %d)\n"),
				strerror(errno), errno);
			exit(EXIT_FAILURE);
		}
	}
	exit(EXIT_SUCCESS);
		
}
