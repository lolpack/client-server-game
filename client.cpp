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

const int UNISIGNED_SHORT_LENGTH = 5;

void send(string msgStr, int sock, int size) {
  if (msgStr.length() > size) {
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

string read(int messageSizeBytes, int socket) {//, sem_t &recSend) {
  cout << "RECEIVING TRANSMISSION NOW" << endl;
  int bytesLeft = messageSizeBytes; // bytes to read
  char buffer[messageSizeBytes]; // initially empty
  char *bp = buffer; //initially point at the first element
  while (bytesLeft > 0) {
    int bytesRecv = recv(socket, (void *)bp, bytesLeft, 0);
    cout << buffer << endl;
    if (bytesRecv <= 0) {
      cerr << "Error receiving message" << endl;
      exit(-1);
    }
    bytesLeft = bytesLeft - bytesRecv;
    bp = bp + bytesRecv;
  }
  cout << "MESSAGE RECEIVED" << endl;
  // sem_post(&recSend);

  return string(buffer);
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
    cerr << "Example: ./client 0.0.0.0 117000" << endl;
    exit(-1);
  }
  char *IPAddr = argv[1]; // IP Address
  const unsigned short servPort = (unsigned short) strtoul(argv[2], NULL, 0);

  string playerName;

  cout << "Welcome to Number Guessing Game! Enter your name:  ";
  cin >> playerName;

  int socket = getSocket(IPAddr, servPort);

  unsigned short nameLength = htons(short(playerName.length()));
  cout << to_string(nameLength).length();

  send(to_string(nameLength), socket, to_string(nameLength).length()); // Send name length before name so server know how long it should be
  read(3, socket); // Wait for AWK

  send(playerName, socket, playerName.length());

  read(3, socket); // Wait for AWK

  int playerGuess;
  int turn = 1;
  bool correct = false;

  while (!correct) {
    cout << "Turn: " << turn << endl;
    cout << "Enter a guess: ";
    cin >> playerGuess;

    unsigned long guess = htonl(long(playerGuess));
    send(to_string(guess), socket, to_string(guess).length());

    turn++;
  }


}