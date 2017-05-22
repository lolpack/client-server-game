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
#include <cstdlib>
#include <ctime>

// Port range 11,700 - 11,799

using namespace std;

sem_t maxConcurrent;
int MAX_CONCURRENT_USERS = 10;

void send(string msgStr, int sock, int size) {
  cout << "This code is hit!" << endl;
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

string read(int messageSizeBytes, int socket, sem_t &recSend) {
  int bytesLeft = messageSizeBytes; // bytes to read
  char buffer[messageSizeBytes]; // initially empty
  char *bp = buffer; //initially point at the first element
  while (bytesLeft > 0) {
    int bytesRecv = recv(socket, (void *)bp, bytesLeft, 0);
    if (bytesRecv <= 0) {
      cerr << "Error receiving message" << endl;
      exit(-1);
    }
    bytesLeft = bytesLeft - bytesRecv;
    bp = bp + bytesRecv;
  }
  cout << "MESSAGE RECEIVED" << endl;
  cout << buffer << endl;
  sem_post(&recSend);

  return string(buffer);
}

int calculateDifference(int guess, int randomNumber) {
  int diff = guess - randomNumber;
  cout << "THE DIFFERENCE " << diff << endl;
  cout << "THE ABS DIFFERENCE" << abs(diff) << endl;

  int absDiff = abs(diff);
  int sum = 0;
  // Pull off highest order number and add it to itself
  for (int i = to_string(absDiff).length(); i >= 0; i--) {
      sum += absDiff % 10;
      absDiff /= 10;
  }
  cout << "ABS DIFF" << sum << endl;
  return sum;
}

void* receiveRequest(void *arg) {
  int localSockNum = *(int*)arg; // Dereference pointer so local copy of sock num is held.
  delete (int*)arg;

  sem_t recSend;
  sem_init(&recSend, 0, 1); // Need mutex to wait for client and then respond
  string clientNameLength = read(5, localSockNum, recSend); // Initial request to know how big name is;

  send(string("AWK"), localSockNum, 3); // Awk request

  int randomNumber = rand() % 10000; // rand() return a number between ​0​ and 9999;

  cout << "RANDOM NUMBER " << randomNumber;;

  int nameLength = int(ntohs(stol(clientNameLength, NULL, 0)));

  cout << "length of name: " << nameLength << endl;

  string name = read(nameLength, localSockNum, recSend);

  cout << "NAME: " << name << endl;
  // unsigned short nameLength = htons(short(localSockNum));

  send(string("AWK"), localSockNum, 3);

  bool correct = false;

  while (!correct) {
    string guessString = read(6, localSockNum, recSend);

    int guess = short(ntohs(stol(guessString)));

    cout << "SHORT " << short(ntohs(stol(guessString))) << endl;
    cout << "NTOHS " << ntohs(stol(guessString)) << endl;
    cout << "STOL " << stol(guessString) << endl;

    cout << "GUESS " << guess << endl;

    int diff = calculateDifference(guess, randomNumber);

    unsigned short sendiff = htons(short(diff));
    cout << "Diff:  " << sendiff << "length" << to_string(sendiff).length() << endl;
    send(to_string(sendiff), localSockNum, to_string(sendiff).length());
    // sem_wait(&recSend);
  }
}

void processNewRequest(int clientSock) {
  pthread_t clientThread;

  pthread_create(&clientThread, NULL, &receiveRequest, (void*) new int(clientSock));
  pthread_join(clientThread, NULL);
}

int main (int argc, char** argv) {
  calculateDifference(1234, 5000);
  calculateDifference(1111, 4354);
  calculateDifference(12, 1234);
  if (argc < 2) {
    cerr << "Server MUST BE STARTED WITH PORT" << endl;
    cerr << "Example: ./server 11700" << endl;
    exit(-1);
  }
  unsigned short PORT = (unsigned short) strtoul(argv[1], NULL, 0);

  srand(time(NULL)); // Init random number generator

  struct sockaddr_in servAddr;
  const int MAXPENDING = 10;

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
  sem_init(&maxConcurrent, 0, MAX_CONCURRENT_USERS - 1); // Only allow 10 users at once.

  while (1) { // Continually run request acceptor.
    sem_wait(&maxConcurrent); // Wait for available processor before accepting request.

    int newSock = accept(sock,(struct sockaddr *) &clientAddr, &addrLen);
    if (newSock < 0) {
      cerr << "Error with incoming message, ignoring request" << endl;
    } else {
      processNewRequest(newSock);
    }
  }
}
