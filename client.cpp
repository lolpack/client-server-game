#include <sys/types.h> // size_t, ssize_t
#include <sys/socket.h> // socket funcs
#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h> // htons, inet_pton
#include <unistd.h> // close
#include <iostream>
#include <fstream>
#include <string>
#include <pthread.h>
#include <semaphore.h>
#include <vector>
#include <cstring>

using namespace std;

void send(string msgStr, int sock) {
  char msg[50];
  if (msgStr.length() >= 50) {
    cerr << "TOO LONG!" << endl;
    exit(-1); // too long
  }
  strcpy(msg, msgStr.c_str());
  int bytesSent = send(sock, (void *) msg, 50, 0);
  if (bytesSent != 50) {
    cerr << "TRANSMISSION ERROR" << endl;
    exit(-1);
  }
}

int main() {
  int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock < 0) {
    cerr << "Error with socket" << endl; exit (-1);
  }

  const char *IPAddr = "10.124.72.20";
  unsigned short servPort = 11700;

  // Convert dotted decimal address to int
  unsigned long servIP;
  int status = inet_pton(AF_INET, IPAddr, (void*)&servIP);
  if (status <= 0) exit(-1);

  // Set the fields
  struct sockaddr_in servAddr;
  servAddr.sin_family = AF_INET; // always AF_INET
  servAddr.sin_addr.s_addr = servIP;
  servAddr.sin_port = htons(servPort);

  status = connect(sock, (struct sockaddr *) &servAddr, sizeof(servAddr));
  if (status < 0) {
    cerr << "Error with connect" << endl;
    exit (-1);
  }

  send("MY NAME IS AARON MY NAME IS AARON MY NAME IS AAR", sock);
}