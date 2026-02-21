#include <stdio.h>
#include <unistd.h>

#include "hello-world.h"

#define PRINT_STR "Hello World!\n"

int main(int argc, char *argv[])
{
	while (1) {
		printf(PRINT_STR);
		#ifdef SLEEP_TIME_US
			usleep(SLEEP_TIME_US);
		#else
			usleep(1000 * 1000);	/* Default sleep time of 1s */
		#endif
	}

	return 0;
}
