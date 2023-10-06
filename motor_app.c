#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
#include <arpa/inet.h>
#include <errno.h>
#define shared "encoder_read"
#define named_semaphore "process_lock"
/* b1 viet 1 process co 2 thread gui & nhan socket */
pthread_t smg, rmg; /* threads of parent */
pthread_t r_e, wm; /* threads of child */
int chat_fd;
sem_t *lock;
/* parent process */
/* thread to send data */
static void *send_msg(void *args)
{
	char *message = (char *)args;
	printf("%s\n", message);
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
		//printf("Semaphore of send socket process == %d\n", current_sem);
		if(current_sem == 0)
		{
			sem_wait(lock);
			memset(msg_sock, '\0', sizeof(msg_sock));
			memset(direction, '\0', sizeof(direction));
			sscanf(message, "%s %f %d", direction, &speed, &time);
			speed = ((speed*(1/0.02)*60)/374);
			time = time/250;
			sprintf(msg_sock, "%s %d %d", direction, (int)speed, time);
			if(strncmp(msg_sock, last_msg, strlen(msg_sock)) != 0)
			{
				write(chat_fd, msg_sock, strlen(msg_sock));
			}
			strncpy(last_msg, msg_sock, strlen(msg_sock));
			printf("send: %s\n", msg_sock);
			sem_post(lock);
		}
	}
	close(chat_fd);
}

/* thread to receive data */
static void *recv_msg(void *args)
{
	char rec_mess[20];
	while(1)
	{
		memset(rec_mess, '\0', sizeof(rec_mess));
		read(chat_fd, rec_mess, sizeof(rec_mess));
		if(strncmp(rec_mess, "exit", 4) == 0)
		{
			pthread_cancel(smg);
			break;
		}
	}
	close(chat_fd);
}

/* child process */
/* thread to read motor state */
static void *read_encoder(void *args)
{
	char *encoder = (char *)args;
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
		memset(args, '\0', sizeof(args));
		read(fd_en, encoder, sizeof(encoder));
		sem_post(lock);
		usleep(20000);
	}
}
/* thread write motor file */
static void *write_motor(void *args)
{
	/* do something */
	while(1);
}

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
	int fd;
	fd = shm_open(shared, O_CREAT| O_RDWR, 666);
	if(-1 == fd)
	{
		printf("Cannot create fd of shared memory\n");
		return -1;
	}
	ftruncate(fd, 100);
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
			/* process con */
			/* child process create 2 threads, 1 for read encoder and 1 for write control data to motor */
			char *shared_mem = (char *)mmap(NULL, 100, PROT_READ| PROT_WRITE, MAP_SHARED, fd, 0);
			if(shared_mem == NULL)
			{
				printf("Cannot mapping to physical address!\n");
				return -1;
			}
			pthread_create(&r_e, NULL, &read_encoder, shared_mem);
			pthread_create(&wm, NULL, &write_motor, NULL);
			pthread_join(r_e, NULL);
			pthread_join(wm, NULL);
			munmap(shared_mem, 100);
		}
		else
		{
			/* process cha */
			/* process cha create 2 threads, 1 for send data to GUI and 1 for received */
			int server_socket, port_no, len;
			struct sockaddr_in server_addr, client_addr;
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
			server_addr.sin_family = AF_INET;
			server_addr.sin_port = htons(port_no);
			server_addr.sin_addr.s_addr = INADDR_ANY;
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
				char *shared_mem = (char *)mmap(NULL, 100, PROT_READ| PROT_WRITE, MAP_SHARED, fd, 0);
				if(shared_mem == NULL)
				{
					printf("Cannot mapping to physical address!\n");
					return -1;
				}
				/* neu ket noi socket thanh cong thi tao ra 2 threads de gui nhan data */
				pthread_create(&smg, NULL, &send_msg, shared_mem);
				pthread_create(&rmg, NULL, &recv_msg, NULL);
				signal(SIGCHLD, process_management);
				pthread_join(smg, NULL);
				pthread_join(rmg, NULL);
				munmap(shared_mem, 100);
				shm_unlink(shared);
			}
		}
	}
	return 0;
}


