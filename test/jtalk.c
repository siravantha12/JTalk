/*
 * CS360:
 * Jim Plank
 */

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "socketfun.h"
#include "fields.h"

send_bytes(char *p, int len, int fd)
{
  char *ptr;
  int i;

  ptr = p;
  while(ptr < p+len) {
    i = write(fd, ptr, p-ptr+len);
    if (i < 0) {
      perror("write");
      exit(1);
    }
    ptr += i;
  }
}

send_string(char *s, int fd)
{
  int len;

  len = strlen(s);
  send_bytes((char *) &len, sizeof(int), fd);
  send_bytes(s, len, fd);
}

receive_bytes(char *p, int len, int fd)
{
  char *ptr;
  int i;

  ptr = p;
  while(ptr < p+len) {
    i = read(fd, ptr, p-ptr+len);
    if (i == 0) exit(0);      /* If the socket closes, exit */
    if (i < 0) {
      perror("read");
      exit(1);
    }
    ptr += i;
  }
}

receive_string(char *s, int size, int fd)
{
  int len;

  receive_bytes((char *) &len, 4, fd);
  if (len > size-1) {
    fprintf(stderr, "Receive string: string too small (%d vs %d)\n", len, size);
    exit(1);
  }
  receive_bytes(s, len, fd);
  s[len] = '\0';
}

void *from_socket(void *v)
{
  int fd, *fdp;
  char s[1100];

  fdp = (int *) v;
  fd = *fdp;

  while(1) { 
    receive_string(s, 1100, fd); 
    printf("%s", s);
  }
  return NULL;
}

main(int argc, char **argv)
{
  pthread_t tid;
  void *retval;
  char *name, *s;
  int fd;
  IS is;
  

  if (argc != 3) {
    fprintf(stderr, "usage: tjtalk host port\n");
    exit(1);
  }

  if (atoi(argv[2]) < 5000) {
    fprintf(stderr, "Must use a port >= 5000");
    exit(1);
  }

  /* Get the user's name */

  is = new_inputstruct(NULL);
  do {
    printf("Enter your name: ");
    fflush(stdout);
  } while (get_line(is) == 0);

  if (is->NF == -1) exit(0);
  name = (char *) malloc(sizeof(char) * (strlen(is->fields[0])+3));
  strcpy(name, is->fields[0]);
  strcat(name, ": ");

  /* Send the fact that the user has just joined to the server */

  fd = request_connection(argv[1], atoi(argv[2]));
  s = (char *) malloc(sizeof(char)*(strlen(name)+1000));
  strcpy(s, name);
  strcat(s, "has just joined\n");
  send_string(s, fd);

  /* Fork off a thread that reads from the socket and prints to the screen */

  if (pthread_create(&tid, NULL, from_socket, &fd) < 0) {
    perror("pthread_create");
    exit(1);
  }

  /* Now, you read from the terminal and send each line to the server */

  while (get_line(is) >= 0) {
    if (is->NF > 0) {    /* Ignore blank lines */
      strcpy(s, name);
      strcat(s, is->text1);
      send_string(s, fd);
    }
  }
}
