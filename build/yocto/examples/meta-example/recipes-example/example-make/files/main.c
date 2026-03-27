#include <stdio.h>
#include <unistd.h>

void example_print(void);

int main(int argc, char *argv[])
{
	while (1) {
		printf("Hello from Makefile program in main()\n");
		example_print();
		usleep(1000 * 1000);	/* Default sleep time of 1s */
	}

	return 0;
}

