#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define F(fmt) "<%d>:"__FILE__":%d:%s: " fmt, my_pid, __LINE__, __func__

pid_t my_pid, parent_pid;

void send(int n);

int main()
{
	parent_pid = getppid();
	my_pid = getpid();

	/* init random */
	struct timeval now;
	gettimeofday(&now, NULL);
	srand((int)now.tv_sec ^ (int)now.tv_usec);

	int sum = 0;

	int n = rand() % 3 + 1;
	int i;
	for (i = 0; i < n; i++) {
		int number = rand() % 3 + 1;
		send(number);
		sum += number;
	}
	printf(F("my sum is %d\n"), sum);
}

void send(int n)
{
	int i;
	printf(F("Sending %d\n"), n);
	for (i = 0; i < n; i++) {
		printf(F("kill(%d, SIGUSR1);\n"), parent_pid);
		kill(parent_pid, SIGUSR1);
		sleep(1);
	}
	printf(F("kill(%d, SIGUSR2);\n"), parent_pid);
	kill(parent_pid, SIGUSR2);
	sleep(1);
}
