#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>


int main(void)
{
	int fd;
	int n;
	char bufw[10] = "012345";
	if (0 > (fd = open("/dev/scull0", O_RDWR))) {
		fprintf(stderr, "open error :%s\n", strerror(errno));
		return -1;
	}
	fprintf(stdout, "open fd :%d for write\n", fd);
	if (0 > (n = write(fd, bufw, strlen(bufw)))) {
		fprintf(stderr, "write error :%s\n",
				strerror(errno));
		exit(EXIT_FAILURE);
	} else if (0 < n) {
		fprintf(stdout, "write n :%d, strlen(bufw):%ld\n",
				n, strlen(bufw));
	} else {
		fprintf(stdout, "is file tail\n");
	}
	if (0 < fd) {
		close(fd);
		fd = -1;
	}
	char buf[10] = {0};
	if (0 > (fd = open("/dev/scull0", O_RDWR))) {
		fprintf(stderr, "open error :%s\n", strerror(errno));
		return -1;
	}
	lseek(fd, 0, SEEK_END);
	fprintf(stdout, "second open fd :%d for write\n", fd);
	if (0 > (n = read(fd, buf, sizeof(buf) / sizeof(*buf)))) {
		fprintf(stderr, "read error :%s\n",
				strerror(errno));
		exit(EXIT_FAILURE);
	} else if (0 < n) {
		fprintf(stdout, "buf:%s, n:%d\n", buf, n);
	} else {
		fprintf(stdout, "is file tail\n");
	}
	if (0 < fd) {
		close(fd);
		fd = -1;
	}

	return 0;
}
