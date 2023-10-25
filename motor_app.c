#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <errno.h>
#include "dongco.h"
#define shared "encoder_read"
#define set_point "set_point"
#define named_semaphore "process_lock"
/* b1 viet 1 process co 2 thread gui & nhan socket */
pthread_t smg, rmg; /* threads of parent */
pthread_t r_e, wm; /* threads of child */
int chat_fd;
sem_t *lock;
pthread_mutex_t motor_lock	= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t read_done	= PTHREAD_COND_INITIALIZER;

struct data {
	char *setting;
	char *reading;
};

/* parent process */
/* thread to send data */
static void *send_msg(void *args)
{
	struct data *foo = (struct data *)args;
	char msg_sock[100];
	char last_msg[100];
	char direction[30];
	float speed;
	int time;
	memset(last_msg, '\0', sizeof(last_msg));
	while(1)
	{
		int current_sem = 0;
		sem_getvalue(lock, &current_sem);
		if(current_sem == 0)
		{
			sem_wait(lock);
			memset(msg_sock, '\0', sizeof(msg_sock));
			memset(direction, '\0', sizeof(direction));
			sscanf(foo->reading, "%s %f %d", direction, &speed, &time);
			speed	= ((speed*(1/0.02)*60)/374);
			time	= time/250;
			sprintf(msg_sock, "%s %d %d", direction, (int)speed, time);
			if(strncmp(msg_sock, last_msg, strlen(msg_sock)) != 0)
			{
				write(chat_fd, msg_sock, strlen(msg_sock));
			}
			strncpy(last_msg, msg_sock, strlen(msg_sock));
			sem_post(lock);
		}
	}
	close(chat_fd);
}

/* thread to receive data */
static void *recv_msg(void *args)
{
	struct data *foo_r = (struct data *)args;
	char rec_mess[100];
	int sem_cur;
	while(1)
	{
		memset(rec_mess, '\0', sizeof(rec_mess));
		read(chat_fd, rec_mess, sizeof(rec_mess));
		if(strncmp(rec_mess, "exit", 4) == 0)
		{
			sem_getvalue(lock, &sem_cur);
			pthread_cancel(smg);
			if(sem_cur == 0)
				sem_post(lock);
			break;
		}
		else
		{
			memset(foo_r->setting, '\0', sizeof(foo_r->setting));
			strncpy(foo_r->setting, rec_mess, strlen(rec_mess));
		}
	}
	close(chat_fd);
}

/* child process */
/* thread to read motor state */
static void *read_encoder(void *args)
{
	struct data *foo_r = (struct data *)args;
	int fd_en;
	fd_en = open("/dev/dongco", O_RDONLY);
	if(-1 == fd_en)
	{
		printf("read motor fault\n");
		exit(EXIT_FAILURE);
	}
	while(1)
	{
		sem_wait(lock);
		pthread_mutex_lock(&motor_lock);
		memset(foo_r->reading, '\0', sizeof(foo_r->reading));
		read(fd_en, foo_r->reading, sizeof(foo_r->reading));
		pthread_cond_signal(&read_done);
		pthread_mutex_unlock(&motor_lock);
		sem_post(lock);
	}
}
/* thread write motor file */
static void *write_motor(void *args)
{
	struct data *foo = (struct data *)args;
	char dir[100], set_dir[100];
	double Kp = 0.01;
	double Ki = 0.0002;
	double Kd = 0.04;
	double T = 0.02;
	double speed, setpoint, E, E1, E2, alpha, beta, gamma;
	uint32_t out, last_out;
	int fd_control;
	fd_control = open("/dev/dongco", O_RDWR);
	if(-1 == fd_control)
	{
		printf("cannot control motor\n");
		exit(EXIT_FAILURE);
	}
	while(1)
	{
		memset(set_dir, '\0', sizeof(set_dir));
		sscanf(foo->setting, "%s %f", set_dir, &setpoint);
		memset(dir, '\0', sizeof(dir));
		pthread_mutex_lock(&motor_lock);
		pthread_cond_wait(&read_done, &motor_lock);
		sscanf(foo->reading, "%s %f %s", dir, &speed, NULL);
		pthread_mutex_unlock(&motor_lock);
		speed = ((speed*(1/0.02)*60)/374);
		if(strncmp(set_dir, dir, strlen(set_dir)) != 0)
		{
			ioctl(fd_control, STOP, NULL);
			sleep(2);
		}
		E		= setpoint - speed;
		alpha		= 2*T*Kp + Ki*T*T + 2*Kd;
 		beta		= T*T*Ki - 4*Kd - 2*T*Kp;
 		gamma		= 2*Kd;
  		out		= (int)(alpha*E + beta*E1 + gamma*E2 + 2*T*last_out)/(2*T);
		if(out>255)
			out = 255;
		else if(out<0)
			out = 0;
		out *= 31875/255;
  		last_out	= out;
		E2		= E1;
		E1		= E;
		if(strncmp(set_dir, "Forward", strlen("forward")) == 0)
		{
			ioctl(fd_control, FORWARD, &out);
		}
		else if(strncmp(set_dir, "Reverse", strlen("reverse")) == 0)
		{
			ioctl(fd_control, REVERSE, &out);
		}
		else
		{
			ioctl(fd_control, STOP, NULL);
		}
	}
}
/* handler stop signal */
static void process_management(int num)
{
	wait(NULL);
	sem_close(lock);
	sem_unlink(named_semaphore);
	printf("Device terminated\n");
	exit(EXIT_FAILURE);
}

static void free_sem(int num)
{
	sem_close(lock);
	sem_unlink(named_semaphore);
	printf("Semaphore free success\n");
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
	pid_t read_dev;
	/* create shared memory  */
	int fd, set;
	fd = shm_open(shared, O_CREAT| O_RDWR, 666);
	set = shm_open(set_point, O_CREAT| O_RDWR, 666);
	if(-1 == fd || -1 == set)
	{
		printf("Cannot create fd of shared memory\n");
		return -1;
	}
	ftruncate(fd, 100);
	ftruncate(set, 100);
	/* init semaphore */
	lock = sem_open(named_semaphore, O_CREAT|O_EXCL, 666, 1);
	if(lock == SEM_FAILED)
	{
		if(errno != EEXIST)
		{
			printf("cannot create semaphore\n");
			return -1;
		}
		lock = sem_open(named_semaphore, 0);
		if(lock == SEM_FAILED)
		{
			printf("Het cuu semaphore\n");
			return -1;
		}
	}
	signal(SIGINT, free_sem);
	/* start operating */
	read_dev = fork();
	if(-1 == read_dev)
	{
		printf("Cannot fork child process\n");
		return -1;
	}
	else
	{
		if(read_dev == 0)
		{
			struct data sh;
			/* process con */
			/* child process create 2 threads, 1 for read encoder and 1 for write control data to motor */
			sh.reading	= (char *)mmap(NULL, 100, PROT_READ| PROT_WRITE, MAP_SHARED, fd, 0);
			sh.setting	= (char *)mmap(NULL, 100, PROT_READ, MAP_SHARED, set, 0);
			//sh->setting		= (char *)spoint;
			//sh->reading		= (char *)shared_mem;
			if(sh.reading == NULL|| sh.setting == NULL)
			{
				printf("Cannot mapping to physical address!\n");
				return -1;
			}
			pthread_create(&r_e, NULL, &read_encoder, &sh);
			pthread_create(&wm, NULL, &write_motor, &sh);
			pthread_join(r_e, NULL);
			pthread_join(wm, NULL);
			munmap(sh.reading, 100);
			munmap(sh.setting, 100);
		}
		else
		{
			/* process cha */
			/* process cha create 2 threads, 1 for send data to GUI and 1 for received */
			int server_socket, port_no, len;
			struct sockaddr_in server_addr, client_addr;
			struct data sh;
			/* parent config for processing */
			if(argc < 2)
			{
				printf("Command: ./app + <port number>\n");
				return -1;
			}
			port_no = atoi(argv[1]);
			/* create a socket */
			server_socket = socket(AF_INET, SOCK_STREAM, 0);
			if(-1 == server_socket)
			{
				printf("Cannot create endpoint\n");
				return -1;
			}
			server_addr.sin_family		= AF_INET;
			server_addr.sin_port		= htons(port_no);
			server_addr.sin_addr.s_addr 	= INADDR_ANY;
			if(bind(server_socket, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
			{
				printf("Cannot bind socket with address and port\n");
				return -1;
			}
			listen(server_socket, 5);
			len = sizeof(client_addr);
			while(1)
			{
				printf("Server is listening at port %d ...!\n", port_no);
				chat_fd = accept(server_socket, (struct sockaddr *)&client_addr, (socklen_t *)&len);
				if(chat_fd < 0)
				{
					printf("Khong the ket noi\n");
					return -1;
				}
				system("clear");
				printf("Server got connection\n");	
				/* mapping shared memory to virtual address */
				sh.reading	= (char *)mmap(NULL, 100, PROT_READ| PROT_WRITE, MAP_SHARED, fd, 0);
				sh.setting	= (char *)mmap(NULL, 100, PROT_WRITE, MAP_SHARED, set, 0);
				//sh->setting 		= (char *)spoint;
				//sh->reading 		= (char *)shared_mem;
				if(sh.setting == NULL|| sh.reading == NULL)
				{
					printf("Cannot mapping to physical address!\n");
					return -1;
				}
				/* neu ket noi socket thanh cong thi tao ra 2 threads de gui nhan data */
				pthread_create(&smg, NULL, &send_msg, &sh);
				pthread_create(&rmg, NULL, &recv_msg, &sh);
				signal(SIGCHLD, process_management);
				pthread_join(smg, NULL);
				pthread_join(rmg, NULL);
				munmap(sh.setting, 100);
				munmap(sh.reading, 100);
				shm_unlink(shared);
				shm_unlink(set_point);
			}
		}
	}
	return 0;
}


