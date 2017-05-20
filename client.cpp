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

void send(string msgStr, int sock, int size) {
  if (msgStr.length() >= size) {
    cerr << "TOO LONG!" << endl;
    exit(-1); // too long
  }
  size++;
  char msg[size];
  strcpy(msg, msgStr.c_str());
  msg[size - 1] = '\n'; // Always end message with terminal char

  int bytesSent = send(sock, (void *) msg, size, 0);
  if (bytesSent != size) {
    cerr << "TRANSMISSION ERROR" << endl;
    exit(-1);
  }
}

int getSocket(char *IPAddr, unsigned short servPort) {
  int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock < 0) {
    cerr << "Error with socket" << endl; exit (-1);
  }


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

  return sock;
}

int main(int argc, char** argv) {
  if (argc < 3) {
    cerr << "CLIENT MUST BE STARTED WITH IP and PORT" << endl;
    exit(-1);
  }
  char *IPAddr = argv[1]; // IP Address
  const unsigned short servPort = (unsigned short) strtoul(argv[2], NULL, 0);

  string playerName;

  cout << "Enter Player Name" << endl;
  cin >> playerName;

  int socket = getSocket(IPAddr, servPort);
  cout << short(playerName.length()) << endl;
  cout << to_string(htons(short(playerName.length()))) << endl;

  unsigned short nameLength = htons(short(playerName.length()));
  char data_to_send[5];
  memcpy(&data_to_send, &nameLength, sizeof(nameLength));
  cout << "DATA" << data_to_send;
  send(to_string(nameLength), socket, 5);
}