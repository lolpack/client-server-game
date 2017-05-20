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

// Port range 11,700 - 11,799

using namespace std;

void send(string msgStr, int sock) {
  char msg[50];
  if (msgStr.length() >= 50) exit(-1); // too long
  strcpy(msg, msgStr.c_str());
  int bytesSent = send(sock, (void *) msg, 50, 0);
  if (bytesSent != 50) exit(-1);
}

int main () {
  unsigned short PORT;

  cout << "Enter a port to bind to" << endl;
  cin >> PORT;

  struct sockaddr_in servAddr;
  const int MAXPENDING = 5;

  servAddr.sin_family = AF_INET;  // always AF_INET
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(PORT);

  int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); if (sock < 0) {
    cerr << "Error with socket" << endl; exit (-1);
  }

  if (bind(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0) {
    cerr << "Error with bind" << endl; exit (-1);
  }

  if (listen(sock, MAXPENDING) < 0 ) { //Listen is non‐blocking: returns immediately.
    cerr << "Error with listen" << endl; exit (-1);
  }

  struct sockaddr_in clientAddr;
  socklen_t addrLen = sizeof(clientAddr);

  int clientSock = accept(sock,(struct sockaddr *) &clientAddr, &addrLen);
  if (clientSock < 0) {
    cerr << "Error with incoming message" << endl;
    exit(-1);
  }

  cout << clientSock;
  int bytesLeft = 50; // bytes to read
  char buffer[50]; // initially empty
  char *bp = buffer; //initially point at the first element
  while (bytesLeft > 0) {
    int bytesRecv = recv(clientSock, (void *)bp, bytesLeft, 0);
    cout << buffer << endl;
    cout << "Bytes recev " << bytesRecv  << endl;
    if (bytesRecv <= 0) {
      cout << "Too few";
      exit(-1);
    }
    bytesLeft = bytesLeft - bytesRecv;
    bp = bp + bytesRecv;
  }


  // send("MY NAME IS AARON MY NAME IS AARON MY NAME IS AARON", sock);
}
