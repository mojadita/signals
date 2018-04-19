#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define F(fmt) "<%d>:"__FILE__":%d:%s: " fmt, my_pid, __LINE__, __func__

pid_t my_pid;

int main()
{
	pid_t parent_pid = getppid();
	struct timeval now;
	gettimeofday(&now, NULL);
	srand((int)now.tv_sec ^ (int)now.tv_usec);
	my_pid = getpid();
	int sum = 0;

	int n = rand() % 3 + 1;
	int i;
	for (i = 0; i < n; i++) {
		int number = rand() % 10 + 10;
		printf(F("Sending %d\n"), number);
		int j;
		for (j = 0; j < number; j++) {
			printf(F("kill(%d, SIGUSR1);\n"), parent_pid);
			kill(parent_pid, SIGUSR1);
		}
		sum += number;
	}
	printf(F("my sum = %d\n"), sum);
}
