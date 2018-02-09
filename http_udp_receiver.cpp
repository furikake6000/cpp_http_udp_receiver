#include <iostream>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <algorithm>

#include "3rd_party/json.hpp"

using namespace std;
using json = nlohmann::json;

#define HTTP_PORT 80

// POSTメッセージからcontentの内容を取り出す
string get_http_content(string buf);

int main(int argc, char *argv[]){
  int s; //Socket
  struct sockaddr_in myskt; //My sockaddr

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
    cerr << "Note: You may have to give me permission, because this app uses PORT 80(http)." << endl;
    exit(1);
  }

  if (listen(s, 5) < 0) {
    perror("listen");
    exit(1);
  }

  //Ready to receiving
  cout << "HTTP->UDP receiver started..." << endl;

  // 応答用HTTPメッセージ作成
  string responsehtml = "<html><title>httpudp responder</title>Your signal accepted!</html>";
  string response =
    "HTTP/1.0 200 OK\r\nContent-Length: " +
    to_string(responsehtml.size()) +
    "\r\nContent-Type: text/html\r\n\r\n" +
    responsehtml + "\r\n";
  //Json読み取り不可メッセージ作成
  string errresponsehtml = "<html><title>httpudp responder</title>Your signal denied.<br/>Be sure your Json post is correct.</html>";
  string errresponse =
    "HTTP/1.0 400 Bad Request\r\nContent-Length: " +
    to_string(errresponsehtml.size()) +
    "\r\nContent-Type: text/html\r\n\r\n" +
    errresponsehtml + "\r\n";

  // 接続開始

  while(1){
    // 1セッション開始
    struct sockaddr_in client; //Client sockaddr
    unsigned int len = sizeof(client);
    char clbuf[1024];

    // 接続待機
    int s_client = accept(s, (struct sockaddr *)&client, &len);

    // データ受信
    memset(clbuf, 0, sizeof(clbuf));
    recv(s_client, clbuf, sizeof(clbuf), 0);
    try {
      // Content取り出しjson解析
      auto data = json::parse(get_http_content(clbuf).c_str());
      send(s_client, response.c_str(), (int)response.size(), 0);
    } catch(std::exception& e) {
      // json解析できなかったら 400 Bad Request を返す
      cerr << "Json parsing error." << std::endl;
      send(s_client, errresponse.c_str(), (int)errresponse.size(), 0);
    }

    // Write Json recognizer here!!!

    // セッション終了
    close(s_client);
  }

  close(s);
}

string get_http_content(string buf){
  // \R\N改行コード2回がcontent開始の合図
  string endsignal = "\r\n\r\n";
  auto r = std::search(buf.begin(), buf.end(), endsignal.begin(), endsignal.end());
  return string(r + endsignal.size(), buf.end());
}
