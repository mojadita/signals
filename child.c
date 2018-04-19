#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define F(fmt) "<%d>:"__FILE__":%d:%s: " fmt, my_pid, __LINE__, __func__
#define N	3

int numbers[N];

pid_t my_pid, parent_pid;

struct sigaction sa, oldsa; 
volatile int yet_unacknowledged;

void send(int n);

void handler()
{
	/* empty, this accepts the interrupt and interrupts the
	 * pause done below */
	printf(F("received signal SIGUSR1 from parent <%d>\n"),
		parent_pid);
	yet_unacknowledged = 0;
}

int main()
{
	parent_pid = getppid();
	my_pid = getpid();

	sa.sa_sigaction = handler; /* set sigaction handler */
	sa.sa_flags = 0;		/* use siginfo to access the pid of the signal sender */
	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGUSR1);

	sigaction(SIGUSR1, &sa, &oldsa);

	/* init random */
	{ /* block */
		int fd = open("/dev/urandom", O_RDONLY);
		int seed;
		read(fd, &seed, sizeof seed);
		close(fd);
		srand(seed);
	}

	int n = rand() % N + 1;
	int i;
	for (i = 0; i < n; i++) {
		int number = rand() % 3 + 1;
		send(number);
		numbers[i] = number;
	}
	printf(F("my numbers are: "));
	for (i = 0; i < n; i++)
		printf("%s%d", i ? ", " : "", numbers[i]);
	puts("");
}

void send(int n)
{
	int i;
	printf(F("Sending %d\n"), n);
	for (i = 0; i < n; i++) {
		printf(F("kill(%d, SIGUSR1);\n"), parent_pid);
		kill(parent_pid, SIGUSR1);
	}
	printf(F("kill(%d, SIGUSR2);\n"), parent_pid);
	sigprocmask(SIG_BLOCK, &sa.sa_mask, NULL);
	yet_unacknowledged = 1;
	kill(parent_pid, SIGUSR2);
	sigprocmask(SIG_UNBLOCK, &sa.sa_mask, NULL);
	while (yet_unacknowledged) sleep(1);
}
