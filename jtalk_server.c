/*
 * Ben Siravantha
 * jtalk_server.c
 * turns jtalk into threaded server
 */

#include "fields.h"
#include "jrb.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <time.h>

pthread_mutex_t lock;
JRB talkers;
JRB past;

send_bytes(char *p, int len, int fd){
	char *ptr;
	int i;

	ptr = p;
	while(ptr < p+len){
		i = write(fd, ptr, p-ptr+len);
		if(i < 0){
			perror("write");
			exit(1);
		}
		ptr += i;
	}
}

send_string(char *s, int fd){
	int len;
	len = strlen(s);
	send_bytes((char *) &len, sizeof(int), fd);
	send_bytes(s, len, fd);
}

int receive_bytes(char *p, int len, int fd){
	char *ptr;
	int i;

	ptr = p;
	while(ptr < p+len){
		i = read(fd, ptr, p-ptr+len);
		if(i == 0){
			return -1;	
		};
		if(i < 0){
			return -1;
		}
		ptr += i;
	}
	return 1738;
}

int receive_string(char *s, int size, int fd){
	int len;

	int jgmoney;
	jgmoney = receive_bytes((char *) &len, 4, fd);
	if(jgmoney < 0) return -1;
	if(len > size-1){
		fprintf(stderr, "Receive string: string too small (%d vs %d)\n", len, size);
		exit(1);
	}
	receive_bytes(s, len, fd);
	s[len] = '\0';
	return 9000;
}

//user
struct connect_info{
	int connect_num;
	int fd;
	char *talker;
	//LIVE(1) or DEAD(0)
	int status;
	time_t join_time;
	time_t last_talked;
	time_t dietime;
};

//thread for each user
void *chat(void *fdescriptor){
	struct connect_info *serve_connect;
	serve_connect = (struct connect_info*) fdescriptor;
	int serve_fd = serve_connect->fd;
	int connect_num = serve_connect->connect_num;

	char join[1100];
	receive_string(join, 1100, serve_fd);
	send_string(join, serve_fd);
	pthread_mutex_lock(&lock);
	JRB i;
	jrb_traverse(i, talkers){
		send_string(join, ((struct connect_info*)i->val.v)->fd);
	}
	pthread_mutex_unlock(&lock);
	//set times
	time_t current_time;
	time_t mess_time;
	time(&current_time);
	time(&mess_time);
	serve_connect->join_time = current_time;
	serve_connect->last_talked = mess_time;

	//put name and connection into JRB
	char *name;
	name = strtok(strdup(join), ":");
	serve_connect->talker = name;
	serve_connect->status = 1;
	
	jrb_insert_int(past, serve_connect->connect_num, new_jval_v(serve_connect));
	jrb_insert_int(talkers, serve_connect->connect_num, new_jval_v(serve_connect));
	char message[1100];
	int check;
	while(1){
		check = receive_string(message, 1100, serve_fd);
		if(check < 0) break;
		pthread_mutex_lock(&lock);
		time(&mess_time);
		serve_connect->last_talked = mess_time;
		JRB temp;
		jrb_traverse(temp, talkers){
			send_string(message, ((struct connect_info*)temp->val.v)->fd);
		}
		pthread_mutex_unlock(&lock);
	}
	//cleanup of client
	time_t death;
	time(&death);
	serve_connect->dietime = death;
	serve_connect->status = 0;	
	pthread_mutex_lock(&lock);
	char *quit = malloc(1100);
	sprintf(quit, "%s has quit\n", name);
	jrb_delete_node(jrb_find_int(talkers, serve_connect->connect_num));
	JRB j;
	jrb_traverse(j, talkers){
		send_string(quit, ((struct connect_info*)j->val.v)->fd);
	}

	pthread_mutex_unlock(&lock);
	pthread_exit(NULL);

}

//thread for console commands
void *console(void *info){
	IS is;
	is = new_inputstruct(NULL);
	char **args;
	args = (char **) info;
	while(!feof(stdin)){
		printf("Jtalk_server_console: ");
		get_line(is);
		fflush(stdout);
		if(strcmp(is->text1, "ALL\n") == 0){
			printf("Jtalk server on %s port %d\n", args[1], atoi(args[2]));
			JRB temp;
			jrb_traverse(temp, past){
				printf("%-6d%-9s", temp->key.i, ((struct connect_info*)temp->val.v)->talker);
				if(((struct connect_info*)temp->val.v)->status == 1){
					printf("LIVE\n");
				}
				else{
					printf("DEAD\n");
				}
				printf("   Joined      at %s", ctime(&((struct connect_info*)temp->val.v)->join_time));
				printf("   Last talked at %s", ctime(&((struct connect_info*)temp->val.v)->last_talked));
				if(((struct connect_info*)temp->val.v)->status == 0){
					printf("   Quit        at %s", ctime(&((struct connect_info*)temp->val.v)->dietime));
				}
			}


		}
		else if(strcmp(is->text1, "TALKERS\n") == 0){
			printf("Jtalk server on %s port %d\n", args[1], atoi(args[2]));
			JRB i;
			jrb_traverse(i, talkers){
				printf("%-6d%-10s last talked at %s", i->key.i, ((struct connect_info*)i->val.v)->talker, ctime(&((struct connect_info*)i->val.v)->last_talked));				
			}
		}
		else{
			printf("Unknown console command: %s", is->text1);
		}
	}
	exit(0);
}


int main(int argc, char **argv){
	if(argc != 3){
		fprintf(stderr, "usage: ./jtalk_server host port\n");
		exit(1);
	}

	int sock, sfd;
	talkers = make_jrb();
	past = make_jrb();
	pthread_mutex_init(&lock, NULL);
	//open hostname and port for conncections
	sock = serve_socket(argv[1], atoi(argv[2]));
	
	//thread for console
	pthread_t console_thread;
	if(pthread_create(&console_thread, NULL, console, argv) != 0){
		perror("pthread_create");
		exit(1);
	}

	//keep track of num connections
	int connection_num = 1;
	struct connect_info *info;

	//continue to accept connections
	while(1){
		sfd = accept_connection(sock);
		
		pthread_t new_connect;
		info = (struct connect_info*)malloc(sizeof(struct connect_info));
		info->connect_num = connection_num;
		info->fd = sfd;
		if(pthread_create(&new_connect, NULL, chat, (void *)info) != 0){
			perror("pthread_create");
			exit(1);
		}

		connection_num++;
	}

	return 0;
}
