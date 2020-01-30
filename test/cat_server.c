/*
 * CS360:
 * Jim Plank
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "socketfun.h"

main(int argc, char **argv)
{
  int sock, fd;

  if (argc != 3) {
    fprintf(stderr, "usage: cat_server host port\n");
    exit(1);
  }

  sock = serve_socket(argv[1], atoi(argv[2]));
  fd = accept_connection(sock);
  printf("Got a connection\n");
  
  dup2(fd, 0);
  dup2(fd, 1);
  close(fd);
  close(sock);
  execl("simpcat", "simpcat", NULL);
  perror("execl");
  return 1;
}
