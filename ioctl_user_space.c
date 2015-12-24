#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define SCULL_IOC_MAGIC		'k'
#define SCULL_IOC_MAXNR		6
#define SCULL_ICO_RESET		_IO(SCULL_IOC_MAGIC, 0)
#define SCULL_IOC_RNUMP		_IOR(SCULL_IOC_MAGIC, 1, int)
#define SCULL_IOC_WNUMP		_IOW(SCULL_IOC_MAGIC, 2, int)
#define SCULL_IOC_WRNUMP	_IOWR(SCULL_IOC_MAGIC, 3, int)
#define SCULL_IOC_RNUMV		_IO(SCULL_IOC_MAGIC, 4)
#define SCULL_IOC_WNUMV		_IO(SCULL_IOC_MAGIC, 5)
#define SCULL_IOC_WRNUMV	_IO(SCULL_IOC_MAGIC, 6)

int main(void)
{
	int fd;
	fd = open("/dev/scullm0", O_RDWR);
	if (0 > fd) {
		fprintf(stderr, "open error :%s\n",
				strerror(errno));
		exit(EXIT_FAILURE);
	}
	int ret;

	ret = ioctl(fd, SCULL_IOC_RNUMV);
	fprintf(stdout, "via value return initing num :%d\n", ret);
	if (0 > (ret = ioctl(fd, SCULL_IOC_WNUMV, 1))) {
		fprintf(stderr, "ioctl error :%s\n",
				strerror(errno));
		exit(EXIT_FAILURE);
	}
	fprintf(stdout, "via value set 1 to kernel\n");
	ret = ioctl(fd, SCULL_IOC_RNUMV);
	fprintf(stdout, "via value return 1 to kernel, ret :%d\n",
			ret);
	int tmp;
	if (0 > ioctl(fd, SCULL_IOC_RNUMP, &tmp)) {
		fprintf(stderr, "ioctl error :%s\n",
				strerror(errno));
		exit(EXIT_FAILURE);
	}
	fprintf(stdout, "via tmp pointer read num, tmp :%d\n",
			tmp);
	tmp = 3;
	if (0 > ioctl(fd, SCULL_IOC_WRNUMP, &tmp)) {
		fprintf(stderr, "ioctl error :%s\n",
				strerror(errno));
		exit(EXIT_FAILURE);
	}
	fprintf(stdout, "via tmp pointer set num, tmp :%d\n",
			tmp);
	fprintf(stdout, "via retrun ioctl :%d\n",
			ioctl(fd, SCULL_IOC_RNUMV));
	tmp = 3;
	if (0 < fd) {
		close(fd);
		fd = -1;
	}

	return 0;
}
