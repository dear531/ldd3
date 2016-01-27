/*
 * for scull_block module, block and nonblock
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define SCULL_FILE	"/dev/scullb0"

int main(void)
{
	int fd = -1;
	fd = open(SCULL_FILE, O_RDWR);
	int opt = 0;
	if (0 > fd) {
		fprintf(stderr, "open error :%s\n",
				strerror(errno));
		exit(EXIT_FAILURE);
	}
#if 0
	opt = fcntl(fd, F_GETFL);
	if (0 > opt) {
		fprintf(stderr, "fcntl get fd opt error :%s\n",
				strerror(errno));
		exit(EXIT_FAILURE);
	}
	if (0 > fcntl(fd, F_SETFL, opt | O_NONBLOCK)) {
		fprintf(stderr, "fcntl set fd nonblock error :%s\n",
				strerror(errno));
		exit(EXIT_FAILURE);
	}
#endif
	char buff[1024] = {0};
	int n = 0;
again:
	n = read(fd, buff, sizeof(buff) / sizeof(*buff));
	if (0 > n && errno == EAGAIN) {
		fprintf(stdout, "in one minit agin\n");
		sleep(1);
		goto again;
	} else if (0 > n) {
		fprintf(stderr, "read error :%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	} else {
		fprintf(stdout, "buff :%s\n", buff);
		fprintf(stdout, "n :%d\n", n);
	}
	if (0 > fd) {
		close(fd);
		fd = -1;
	}
	return 0;
}
