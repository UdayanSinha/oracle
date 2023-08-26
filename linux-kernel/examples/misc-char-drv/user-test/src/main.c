/*
 * main.c: User-space test program
	   For misc-char-drv kernel module
 * Author: Udayan Prabir Sinha
 * Date: 2023-08-26
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>

#define DEV_PATH "/dev/misc-char-drv"

int main (int argc, char *argv[])
{
	int fd = -1;
	int ret = 0;
	ssize_t num_byte = 0;
	uint8_t *read_data = NULL;
	size_t read_num = 4;
	uint32_t i;

	if (argc < 2) {
		printf("Using default number of bytes to read: %lu\n",
				read_num);
	} else if (argc == 2) {
		read_num = strtoul(argv[1], NULL, 0);
		printf("Number of bytes to read: %lu\n", read_num);
	} else {
		printf("Invalid number of input arguments: %d\n", argc - 1);
		printf("Usage: /path/to/user-test <n>\n");
		printf("\t n: Number of bytes to read\n");
		return -1;
	}

	printf("Attempting to open %s\n", DEV_PATH);

	fd = open(DEV_PATH, O_RDONLY, S_IRUSR | S_IRGRP | S_IROTH);
	if (fd < 0) {
		perror("open() failed");
		return -1;
	}

	printf("Successful open(), attempting to malloc() read buffer\n");
	read_data = (uint8_t *)malloc(read_num);
	if (!read_data) {
		perror("malloc() failed");
		ret = -1;
		goto out;
	}

	printf("Successful malloc(), attempting to read from %s\n", DEV_PATH);
	num_byte = read(fd, (void *)read_data, read_num);
	if (num_byte < 0) {
		perror("read() failed");
		ret = -1;
		goto out;
	} else {
		for (i = 0; i < num_byte; i++)
			printf("Read Data[%u]: 0x%x\n", i, read_data[i]);
	}

out:
	printf("Closing %s\n", DEV_PATH);
	if (close(fd) < 0)
		perror("close() failed");

	if (read_data)
		free(read_data);

	return ret;
}
