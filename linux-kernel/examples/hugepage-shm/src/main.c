#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/shm.h>

int main(int argc, char *argv[])
{
	int ret = 0;
	int shm_id = -1;
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

	printf("Using huge-page shm of size %d\n", hpg_len);
	shm_id = shmget(IPC_PRIVATE, hpg_len,
			0644 | IPC_CREAT | SHM_HUGETLB | SHM_HUGE_2MB);
	if (shm_id < 0) {
		printf("Failed to get huge-page shm: %d\n", -errno);
		return -1;
	}

	hpg_addr = shmat(shm_id, NULL, 0);
	if ((hpg_addr == ((void*) -1)) || !hpg_addr) {
		printf("Failed to attach to huge-page shm: %d\n",
			(hpg_addr == ((void*) -1)) ? -errno : -EFAULT);
		(void) shmctl(shm_id, IPC_RMID, NULL);
		return -1;
	}

	printf("Writing stuff to huge pages\n");
	cptr = (char*) hpg_addr;
	for (i = 0; i < 26; i++) {
		*cptr = letter + i;
		cptr++;
	}

	while(1);	/* to inspect the process */
	(void) shmdt(hpg_addr);
	(void) shmctl(shm_id, IPC_RMID, NULL);
	return 0;
}
