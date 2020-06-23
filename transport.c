// Maciej Korpalski
// 299513
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "tr_helpers.h"
#define ERROR(str)                                     \
  {                                                    \
    fprintf(stderr, "%s: %s\n", str, strerror(errno)); \
    exit(EXIT_FAILURE);                                \
  }
#define BS 1000    // Data in one standard packet
#define RWSM 1000  // Max window size

int main(int argc, char *argv[]) {
  if (argc != 5) {
    fprintf(stderr, "Usage: %s IP_addr port file_name bytes_count\n", argv[0]);
    return EXIT_FAILURE;
  } else {
    int ok = check_input(argv[1], argv[2], argv[4]);
    if (ok != 0) return EXIT_FAILURE;
  }
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) ERROR("socket error");

  struct sockaddr_in server;
  int port = atoi(argv[2]);
  char *ip = argv[1];
  memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(port);
  inet_pton(AF_INET, ip, &server.sin_addr);

  FILE *f = fopen(argv[3], "wb");
  int N = atoi(argv[4]), NSegm = N / BS, NRecv = 0, TotRecv = 0, LastShown = 0;
  if (NSegm * BS < N) NSegm++;
  int RWS = RWSM > NSegm ? NSegm : RWSM;  // Window Size
  char *buffer = malloc(RWS * BS + 10);
  int ack[RWS], Last = N % BS == 0 ? BS : N % BS;
  struct timeval rtv;  // round time period

  for (int i = 0; i < RWS; i++) {
    if (i != NSegm - 1)
      send_get(sockfd, server, BS * i, BS);
    else
      send_get(sockfd, server, BS * i, Last);
    ack[i] = -1;
    if (i == NSegm - 1) break;
  }
  gettimeofday(&rtv, NULL);
  rtv.tv_usec += 100000;

  while (NRecv < NSegm) {
    while (1) {
      struct sockaddr_in sender;
      socklen_t sender_len = sizeof(sender);
      char resp[BS + 100];
      struct timeval tv, now;
      gettimeofday(&now, NULL);
      timersub(&rtv, &now, &tv);

      fd_set descriptors;
      FD_ZERO(&descriptors);
      FD_SET(sockfd, &descriptors);
      int ready = select(sockfd + 1, &descriptors, NULL, NULL, &tv);
      if (ready <= 0) {
        break;
      }
      ssize_t datagram_len = recvfrom(sockfd, resp, BS + 100, 0,
                                      (struct sockaddr *)&sender, &sender_len);
      if (datagram_len < 0) ERROR("recvfrom error");

      char sender_ip_str[20];
      inet_ntop(AF_INET, &(sender.sin_addr), sender_ip_str,
                sizeof(sender_ip_str));
      if (check_mess(sender_ip_str, ntohs(sender.sin_port), ip, port) == 1) {
        continue;
      }

      int st, len, read;
      int ok = sscanf(resp, "DATA %u %u\n%n", &st, &len, &read);
      if (ok == EOF) ERROR("Wrong input format!\n");
      while (isspace(resp[read - 2]) > 0) read--;
      ok = 1;
      if (st / BS >= NRecv && st / BS < NRecv + RWS && ack[(st / BS) % RWS] < 0)
        ack[(st / BS) % RWS] = 1 + (st / BS) / RWS;
      else
        ok = 0;
      if (ok == 0) continue;
      TotRecv++;
      memcpy(buffer + (BS * ((st / BS) % RWS)), resp + read, len);

      while (ack[NRecv % RWS] == 1 + NRecv / RWS) {
        if (NRecv != NSegm - 1)
          fwrite(buffer + BS * (NRecv % RWS), sizeof(uint8_t), BS, f);
        else
          fwrite(buffer + BS * (NRecv % RWS), sizeof(uint8_t), Last, f);
        NRecv++;
      }
    }
    struct timeval now;
    gettimeofday(&now, NULL);
    for (int i = 0; i < RWS; i++) {
      int idx = NRecv + i;
      if (idx >= NSegm) break;
      if (timercmp(&rtv, &now, <) && ack[idx % RWS] < 1 + idx / RWS) {
        if (idx != NSegm - 1)
          send_get(sockfd, server, BS * idx, BS);
        else
          send_get(sockfd, server, BS * idx, Last);
        ack[idx % RWS] = -1;
        if (idx == NSegm - 1) break;
      }
    }
    gettimeofday(&rtv, NULL);
    rtv.tv_usec += 100000;
    if (TotRecv >= LastShown || TotRecv == NSegm) {
      printf("Progress: %.4lf%%\n", 100.0 * (double)TotRecv / (double)NSegm);
      LastShown += ((RWS / 5) > 0 ? RWS / 5 : 1);
    }
  }

  if (close(sockfd) < 0) ERROR("close error");

  free(buffer);
  fclose(f);
  return 0;
}
