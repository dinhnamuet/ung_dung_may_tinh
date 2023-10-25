#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include "dongco.h"
void GUI()
{
	printf(" _______________________________________________\n");
	printf("|		TEST APP			|\n");
	printf("|1. FORWARD					|\n");
	printf("|2. REVERSE					|\n");
	printf("|3. STOP					|\n");
	printf("|4. EXIT					|\n");
	printf("|_______________________________________________|\n");
	printf(">>> ");
}
int main(int argc, char **argv[])
{
	uint32_t opt;
	uint32_t speed;
	int fd;
	fd = open("/dev/dongco", O_RDWR);
	if(-1 == fd)
	{
		printf("Cannot start this app\n");
		return -1;
	}
	while(1)
	{
		system("clear");
		GUI();
		scanf("%d", &opt);
		switch(opt)
		{
			case 1:
				printf("% pwm = ");
				scanf("%d", &speed);
				speed = (int)(speed*31875/100);
				ioctl(fd, FORWARD, &speed);
				break;
			case 2:
				printf("% pwm = ");
				scanf("%d", &speed);
				speed = (int)(speed*31875/100);
				ioctl(fd, REVERSE, &speed);
				break;
			case 3:
				ioctl(fd, STOP, NULL);
				break;
			case 4:
				close(fd);
				return 0;
			default:
				break;
		}
	}
	return 0;
}
