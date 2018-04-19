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

struct child {
	pid_t c_pid;
	int c_val;
	volatile int c_list[100];
	volatile int c_listsz;
}children[NCHILDREN];

void handler(int sig, siginfo_t *info, void *unused)
{
	struct child *p;

	/* search for the child */
	for (p = children; p < children + NCHILDREN; p++) {
		if (p->c_pid == info->si_pid)
			break; /* found */
	}

	/* if not found (this must not happen) */
	if (p >= children + NCHILDREN) {
		printf(
			F("ERROR: received signal %d but child not found (pid from signal = %d)\n"),
			sig, info->si_pid);
		return;
	}

	printf(F("received signal %s from <%d>\n"),
		sig == SIGUSR1
			? "SIGUSR1"
			: sig == SIGUSR2
				? "SIGUSR2"
				: "OTHER",
		p->c_pid);

	switch (sig) {
	case SIGUSR1:
		p->c_val++;
		break;

	case SIGUSR2: 
		printf(F("received number %d from child <%d>\n"),
			p->c_val, p->c_pid);
		p->c_list[p->c_listsz++] = p->c_val;
		p->c_val = 0;
		break;

	} /* switch */
} /* handler */
	

int main()
{
	my_pid = getpid();

	struct sigaction sa, oldsa; 

	sa.sa_sigaction = handler; /* set sigaction handler */
	sa.sa_flags =
		SA_RESTART |	/* restart the wait(2) syscall below */
		SA_SIGINFO;		/* use siginfo to access the pid of the signal sender */
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGUSR1);
	sigaddset(&sa.sa_mask, SIGUSR2);

	/* block signals we are going to deal with */
	sigprocmask(SIG_BLOCK, &sa.sa_mask, NULL);
	sigaction(SIGUSR1, &sa, &oldsa);
	sigaction(SIGUSR2, &sa, NULL);

	pid_t res;
	int i;

	for (i = 0; i < NCHILDREN; i++) {
		/* before forking, we init the child structure so we don't
		 * receive a signal before the counter is initialised to 0.
		 * There's a race condition in which the fork succeeds, the
		 * child makes the exec(2) syscall and sends the first signal
		 * before we have saved the pid of the child in the array,
		 * the handler doesn't find the slot and is unable to store
		 * the received signal properly.  So we block the signals
		 * to protect the handler to be executed inside the critical
		 * region.
		 */
		children[i].c_val = 0;
		children[i].c_listsz = 0;
		children[i].c_pid = res = fork();

		if (res == 0) { /* the child */
			/* unblock signals set in the parent */
			sigprocmask(SIG_UNBLOCK, &sa.sa_mask, NULL);

			/* and exec(2) */
			execlp(child, child, NULL);

			fprintf(stderr, /* error in exec(2) */
				F("ERROR: exec: %s: %s (errno = %d)\n"),
				child, strerror(errno), errno);
			exit(EXIT_FAILURE);

		} else if (res < 0) { /* error in fork(2) */
			fprintf(stderr,
				F("ERROR: fork: %s (errno = %d)\n"),
				strerror(errno), errno);
			exit(EXIT_FAILURE);
		}
		/* parent continues */
	}

	/* now we can unblock the signals as we have initialised the
	 * children array */
	sigprocmask(SIG_UNBLOCK, &sa.sa_mask, NULL);

	/* wait for the children to die */
	while ((res = wait(NULL)) >= 0) {
		struct child *p;
		for (p = children; p < children + NCHILDREN; p++)
			if (p->c_pid == res) break;
		if (p >= children + NCHILDREN) {
			fprintf(stderr,
				F("ERROR: wait: child not found (pid = %d)\n"),
				res);
			exit(EXIT_FAILURE);
		}
		/* p points to the child's data */
		printf(F("child <%d> numbers: "), p->c_pid);
		for(i = 0; i < p->c_listsz; i++)
			printf("%s%d",
				i ? ", " : "",
				p->c_list[i]);
		puts("");
	}

	sigaction(SIGUSR1, &oldsa, NULL);

	printf(F("END OF PROGRAM\n"));
} /* main */
