#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define F(fmt) "[%d]:"__FILE__":%d:%s: " fmt, my_pid, __LINE__, __func__

#define NCHILDREN 8

char child[] = "child";

pid_t my_pid;

int sum = 0; /* here's where the number of signals is accumulated */

void handler(int sig, siginfo_t *info, void *unused)
{
	switch (sig) {
	case SIGUSR1: sum++; break;
	}
}
	
int main()
{
	int i;

	my_pid = getpid();

	struct sigaction sa, oldsa; 
	sa.sa_sigaction = handler;
	sa.sa_flags = SA_RESTART; /* restart the wait(2) syscall below */
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGUSR1);
	sigaction(SIGUSR1, &sa, &oldsa);

	pid_t res;

	for (i = 0; i < NCHILDREN; i++) {
		res = fork();
		if (res > 0) /* parent */
			continue;
		else if (res < 0) {
			fprintf(stderr,
				F("ERROR: fork: %s (errno = %d)\n"),
				strerror(errno), errno);
			exit(EXIT_FAILURE);
		} else { /* child */
			execlp(child, child, NULL);
			fprintf(stderr,
				F("ERROR: exec: %s: %s (errno = %d)\n"),
				child, strerror(errno), errno);
			exit(EXIT_FAILURE);
		}
	}

	while (wait(NULL) >= 0); /* wait for children until no more */

	sigaction(SIGUSR1, &oldsa, NULL);

	printf(F("sum = %d\n"), sum);
}
