#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>

int main(int argc, char *argv[])
{
	int ret = 0;
	void *hpg_addr = NULL;
	size_t hpg_len = 0;
	char *cptr = NULL;
	char letter = 'A';
	unsigned int i = 0;

	if (argc != 2) {
		printf("Invalid input arguments provided\n");
		printf("Usage: %s <total-huge-page-size-in-bytes>\n", argv[0]);
		return 0;
	}

	hpg_len = atoi(argv[1]);
	if (!hpg_len) {
		printf("Failed to read total huge page size from %s\n", argv[1]);
		return -1;
	}

	printf("Using huge-page mapping of size %d\n", hpg_len);
	hpg_addr = mmap(NULL, hpg_len, PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_HUGE_2MB | MAP_HUGETLB | MAP_ANONYMOUS, -1, 0);
	if ((hpg_addr == ((void*) -1)) || !hpg_addr) {
		printf("Failed to get huge-page mapping: %d\n",
			(hpg_addr == ((void*) -1)) ? -errno : -EFAULT);
		return -1;
	}

	printf("Writing stuff to huge pages\n");
	cptr = (char*) hpg_addr;
	for (i = 0; i < 26; i++) {
		*cptr = letter + i;
		cptr++;
	}

	while(1);	/* to inspect the process */
	(void) munmap(hpg_addr, hpg_len);
	return 0;
}
