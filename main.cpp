#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define HTTP_PORT 80

int main(int argc, char *argv[]){
  int s; //Socket
  struct sockaddr_in myskt; //My sockaddr

  char buf[1024];

  if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0){
    perror("socket");
    exit(1);
  }
  memset(&myskt, 0, sizeof myskt);
  myskt.sin_family = AF_INET;
  myskt.sin_port = htons(HTTP_PORT);
  myskt.sin_addr.s_addr = htonl(INADDR_ANY);

  //??
  int yes=1;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));

  if (bind(s, (struct sockaddr *)&myskt, sizeof(myskt)) < 0) {
    perror("bind");
    exit(1);
  }

  if (listen(s, 5) < 0) {
    perror("listen");
    exit(1);
  }

  //Ready to receiving
  printf("HTTP->UDP receiver started...\n");

  // 応答用HTTPメッセージ作成
  memset(buf, 0, sizeof(buf));
  sprintf(buf,
   "HTTP/1.0 200 OK\r\n"
   "Content-Length: 200\r\n"
   "Content-Type: text/html\r\n"
   "\r\n"
   "<html><title>httpudp responder</title>Your signal accepted!</html>\r\n");

  while(1){
    struct sockaddr_in client; //Client sockaddr
    unsigned int len = sizeof(client);
    char clbuf[1024];

    int s_client = accept(s, (struct sockaddr *)&client, &len);

    memset(clbuf, 0, sizeof(clbuf));
    recv(s_client, clbuf, sizeof(clbuf), 0);
    printf("%s", clbuf);

    send(s_client, buf, (int)strlen(buf), 0);

    close(s_client);
  }

  close(s);
}
