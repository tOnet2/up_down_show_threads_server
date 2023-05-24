#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct sockaddr sa;
typedef struct sockaddr_in sain;
typedef struct for_thread {
	int *var;
	int *cs;
	char *uc;
	size_t uc_length;
	sem_t *sem;
	pthread_mutex_t *mutex;
} ft;

void* thread_main(void *argv);

int main()
{
	const char unkown_cmd[] = "Unknown command :(\n";
	int ls = socket(AF_INET, SOCK_STREAM, 0);
	int optavl = 1;
	setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, &optavl, sizeof(optavl));
	int var = 0;
	sem_t sem;
	sem_init(&sem, 0, 0);
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	ft data;
	data.var = &var;
	data.sem = &sem;
	data.mutex = &mutex;
	data.uc = (char*)unkown_cmd;
	data.uc_length = sizeof(unkown_cmd) - 1;
	sain lsaddr;
	lsaddr.sin_family = AF_INET;
	lsaddr.sin_port = htons(1234);
	lsaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	socklen_t lsaddr_len = sizeof(lsaddr);
	bind(ls, (const sa*)&lsaddr, lsaddr_len);
	listen(ls, 5);
	for (;;) {
		int cs = accept(ls, NULL, NULL);
		pthread_t thread;
		data.cs = &cs;
		pthread_create(&thread, NULL, thread_main, &data);
		sem_wait(&sem);
	}
	return 0;
}

void* thread_main(void *argv)
{
	pthread_detach(pthread_self());
	ft *data = argv;
	int cs = *data->cs;
	sem_t *sem = data->sem;
	sem_post(sem);
	int *var = data->var;
	char *uc = data->uc;
	size_t uc_len = data->uc_length;
	pthread_mutex_t *mutex = data->mutex;
	char buf[100];
	char var_string[13];
	int snprintf_return;
	ssize_t recv_return;
	while (cs && (recv_return = recv(cs, buf, 100, 0))) {
		if (recv_return == 100) {
			close(cs);
			cs = 0;
		}
		if (strstr(buf, "UP")) {
			pthread_mutex_lock(mutex);
			++*var;
			pthread_mutex_unlock(mutex);
		} else if (strstr(buf, "DOWN")) {
			pthread_mutex_lock(mutex);
			--*var;
			pthread_mutex_unlock(mutex);
		} else if (strstr(buf, "SHOW")) {
			pthread_mutex_lock(mutex);
			snprintf_return = snprintf(var_string, 13, "%d\n", *var);
			pthread_mutex_unlock(mutex);
			send(cs, (const char*)var_string, snprintf_return, 0);
			bzero(var_string, snprintf_return);
		} else if (strstr(buf, "EXIT")) {
			close(cs);
			cs = 0;
		} else {
			send(cs, (const char*)uc, uc_len, 0);
		}
	}
	return NULL;
}
