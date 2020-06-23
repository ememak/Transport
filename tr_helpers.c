#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "tr_helpers.h"

int check_input(char *ip, char *port, char *bytes) {
  uint32_t tmp;
  int ok = inet_pton(AF_INET, ip, &tmp);
  if (ok != 1) {
    fprintf(stderr, "Invalid IP number: %s\n", ip);
    return -1;
  }
  int b = strtol(port, NULL, 10);
  if (b <= 0 || b >= 65536 || errno == ERANGE) {
    fprintf(stderr, "Invalid port number: %s\n", port);
    return -1;
  }
  b = strtol(bytes, NULL, 10);
  if (b <= 0 || errno == ERANGE) {
    fprintf(stderr, "Invalid bytes number: %s\n", bytes);
    return -1;
  }
  if (b > 10000000) {
    fprintf(stderr, "Too much data to download: %s\n", bytes);
    return -1;
  }
  return 0;
}

int send_get(int sockfd, struct sockaddr_in server, int start, int size) {
  char *buffer = malloc(50);
  memset(buffer, 0, 50);
  sprintf(buffer, "GET %d %d\n", start, size);
  ssize_t buff_len = strlen(buffer);
  if (sendto(sockfd, buffer, buff_len, 0, (struct sockaddr *)&server,
             sizeof(server)) != buff_len) {
    fprintf(stderr, "sendto error: %s\n", strerror(errno));
    return -1;
  }
  return 0;
}

int check_mess(char *sip, int sp, char *ip, int p) {
  if (p != sp) return 1;
  if (strcmp(sip, ip) != 0) return 1;
  return 0;
}
