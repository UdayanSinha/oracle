/*
 * main.c: User-space test program
	   For misc-char-drv kernel module
 * Author: Udayan Prabir Sinha
 * Date: 2023-08-26
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#define PROG_NAME	"user-test"

#define IOC_CMD_PANIC		0x5800

static void print_usage(void)
{
	printf("Usage:\n\t");
	printf("%s read node-path num-bytes\n\t", PROG_NAME);
	printf("%s ioctl node-path cmd\n\n", PROG_NAME);
}

static int op_read(char *node_path, char *num_bytes_str)
{
	int fd = -1;
	int ret = 0;
	size_t read_num = strtoul(num_bytes_str, NULL, 0);
	ssize_t bytes_rcvd = 0;
	uint8_t *read_data = NULL;
	uint32_t i;

	printf("Number of bytes to read from %s: %lu\n", node_path, read_num);

	printf("Attempting to open %s\n", node_path);
	fd = open(node_path, O_RDONLY, S_IRUSR | S_IRGRP | S_IROTH);
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

	printf("Successful malloc(), attempting read() on %s\n", node_path);
	bytes_rcvd = read(fd, (void *)read_data, read_num);
	if (bytes_rcvd < 0) {
		perror("read() failed");
		ret = -1;
		goto out;
	} else {
		for (i = 0; i < bytes_rcvd; i++)
			printf("Read Data[%u]: 0x%x\n", i, read_data[i]);
	}

out:
	printf("Closing %s\n", node_path);
	if (close(fd) < 0)
		perror("close() failed");

	if (read_data)
		free(read_data);

	return ret;
}

static int op_ioctl(char *node_path, char *cmd_str)
{
	int fd = -1;
	uint32_t cmd = ~0;
	int ret = 0;

	if (!strcmp(cmd_str, "panic")) {
		cmd = IOC_CMD_PANIC;
	} else {
		printf("Invalid ioctl() command: %s\n", cmd_str);
		return -1;
	}

	printf("Attempting to open %s\n", node_path);
	fd = open(node_path, O_RDONLY, S_IRUSR | S_IRGRP | S_IROTH);
	if (fd < 0) {
		perror("open() failed");
		return -1;
	}

	printf("Successful open(), attempting ioctl() on %s\n", node_path);
	ret = ioctl(fd, IOC_CMD_PANIC, 0);
	if (ret) {
		perror("ioctl() failed");
		ret = -1;
		goto out;
	}

out:
	printf("Closing %s\n", node_path);
	if (close(fd) < 0)
		perror("close() failed");

	return ret;
}

int main (int argc, char *argv[])
{
	int ret = 0;

	if (argc != 4) {
		print_usage();
		ret = -1;
	} else {
		if (!strcmp(argv[1], "read")) {
			ret = op_read(argv[2], argv[3]);
		} else if (!strcmp(argv[1], "ioctl")) {
			ret = op_ioctl(argv[2], argv[3]);
		} else {
			printf("Invalid operation type: %s\n", argv[1]);
			print_usage();
			ret = -1;
		}
	}

	return ret;
}
