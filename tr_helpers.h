#ifndef TR_HELP_H
#define TR_HELP_H
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdint.h>

int check_input(char *ip, char *port, char *bytes);
int send_get(int sockfd, struct sockaddr_in server, int start, int size);
int check_mess(char *sip, int sp, char *ip, int p);

#endif
