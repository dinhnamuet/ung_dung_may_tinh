#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
int main()
{
	int fd;
	char buff[20];
	char direction[10];
	float speed;
	float time;
	char *start = "Forward 200";
	fd = open("/dev/dongco", O_WRONLY);
	if(-1 == fd)
	{
		return -1;
	}
	write(fd, start, strlen(start));
	close(fd);
	while(1)
	{
		memset(buff, '\0', sizeof(buff));
		fd = open("/dev/dongco", O_RDONLY);
		if(-1 == fd)
		{
			printf("Loi\n");
			return -1;
		}
		read(fd, buff, sizeof(buff));
		sscanf(buff, "%s %f %f", direction, &speed, &time);
		speed = (speed*60)/374;
		time /= 250;
		sprintf(buff, "%s %d %f", direction, (int)speed, time);
		printf("motor state: %s\n", buff);
		close(fd);
		sleep(1);
	}
	return 0;
}
