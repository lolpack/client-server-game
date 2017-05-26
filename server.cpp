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
#include <queue>
#include <vector>

// Port range 11,700 - 11,799

using namespace std;

struct Winner {
  string name;
  int turns;
};

struct compareWinners {
  bool operator()(const Winner& l, const Winner& r) const
  {
    return l.turns >= r.turns; // Compare two winners turns. Least number of turns wins.
  }
};

priority_queue<Winner, vector<Winner>, compareWinners> *leaderBoard = new priority_queue<Winner, vector<Winner>, compareWinners>;
vector<Winner> tempLeaderBoard;

sem_t maxConcurrent;
sem_t leaderBoardLock;
int MAX_CONCURRENT_USERS = 10;

void send(string msgStr, int sock, int size) {
  string newString = string(size - msgStr.length(), '0') + msgStr;

  size++;
  char msg[size];
  strcpy(msg, newString.c_str());
  msg[size - 1] = '\n'; // Always end message with terminal char

  int bytesSent = send(sock, (void *) msg, size, 0);
  if (bytesSent != size) {
    cerr << "TRANSMISSION ERROR" << endl;
    return;
  }
}

string read(int messageSizeBytes, int socket) {
  int bytesLeft = messageSizeBytes; // bytes to read
  char buffer[messageSizeBytes]; // initially empty
  char *bp = buffer; //initially point at the first element
  while (bytesLeft > 0) {
    int bytesRecv = recv(socket, (void *)bp, bytesLeft, 0);
    if (bytesRecv <= 0) {
      cerr << "Error receiving message from client" << endl;
      return string("BAD MESSAGE");
    }
    bytesLeft = bytesLeft - bytesRecv;
    bp = bp + bytesRecv;
  }

  return string(buffer);
}

int calculateDifference(int guess, int randomNumber) {
  int sum = 0;
  // Pull off highest order number and add it to itself
  for (int i = 4; i >= 0; i--) {
    int guessMod = guess % 10;
    int randoMod = randomNumber % 10;

    sum += abs(randoMod - guessMod);
    guess /= 10;
    randomNumber /= 10;
  }

  return sum;
}

void* receiveRequest(void *arg) {
  int localSockNum = *(int*)arg; // Dereference pointer so local copy of sock num is held.
  delete (int*)arg;

  int *z; // Return some value to supress compiler warning;

  sem_t recSend;
  sem_init(&recSend, 0, 1); // Need mutex to wait for client and then respond
  string clientNameLength = read(6, localSockNum); // Initial request to know how big name is;

  if (clientNameLength == string("BAD MESSAGE")) { // Safely return thread if response is unreadable.
    sem_post(&leaderBoardLock);
    close(localSockNum);
    return (void*) z;
  }

  send(string("AWK"), localSockNum, 3); // Awk that length is received.

  int randomNumber = rand() % 10000; // rand() return a number between ​0​ and 9999;

  int nameLength = short(ntohs(stol(clientNameLength)));
  string name = read(nameLength, localSockNum);

  if (name == string("BAD MESSAGE")) {
    sem_post(&leaderBoardLock);
    close(localSockNum);
    return (void*) z;
  }

  cout << "Random number generated for " << name << ": " << randomNumber << endl;
  cout.flush(); // Force cout before loop

  send(string("AWK"), localSockNum, 3);

  bool correct = false;

  while (!correct) {
    string guessString = read(101, localSockNum);

    if (guessString == string("BAD MESSAGE")) {
      sem_post(&leaderBoardLock);
      close(localSockNum);
      return (void*) z;
    }

    int guess = short(ntohs(stol(guessString)));

    int diff = calculateDifference(guess, randomNumber);

    unsigned short sendiff = htons(short(diff));
    send(to_string(sendiff), localSockNum, 100);

    if (sendiff == 0) {
      correct = true;
    }
  }

  string turnsResponse = read(101, localSockNum);

  if (turnsResponse == string("BAD MESSAGE")) {
    sem_post(&leaderBoardLock);
    close(localSockNum);
    return (void*) z;
  }

  int turns = short(ntohs(stol(turnsResponse)));

  Winner winner;
  winner.name = name;
  winner.turns = turns;

  sem_wait(&leaderBoardLock);

  leaderBoard->push(winner);

  int topThree = 3; // Iterate through the first 3 in Pqueue or the number of values in Pqueue
  if (leaderBoard->size() < 3) {
    topThree = leaderBoard->size();
  }

  string leaderBoardText;

  for (int j = 0; j < topThree; j++) {
    Winner *tempwin = new Winner;
    tempwin->name = leaderBoard->top().name;
    tempwin->turns = leaderBoard->top().turns;
    leaderBoard->pop();
    string eachRow = string(to_string(j + 1)) + string(". ") + string(tempwin->name) + string(" ") + string(to_string(tempwin->turns)) + string("&&"); // Use delimiter so to replace carriage return

    leaderBoardText = leaderBoardText + eachRow;

    tempLeaderBoard.push_back((*tempwin));
    delete tempwin;
  }

  for (int k = 0; k < topThree; k++) {
    Winner winl = tempLeaderBoard.back();
    leaderBoard->push(winl); // Take items temporarily in pQueue and put it back in vector;
    tempLeaderBoard.pop_back();
  }

  sem_post(&leaderBoardLock);

  send(leaderBoardText, localSockNum, 1000);
  close(localSockNum);
  sem_post(&maxConcurrent);
}

int main (int argc, char** argv) {
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
  sem_init(&maxConcurrent, 0, MAX_CONCURRENT_USERS); // Only allow 10 users at once.
  sem_init(&leaderBoardLock, 0, 1);

  while (true) { // Continually run request acceptor.
    sem_wait(&maxConcurrent); // Wait for available processor before accepting request.

    int newSock = accept(sock,(struct sockaddr *) &clientAddr, &addrLen);
    if (newSock < 0) {
      cerr << "Error with incoming message, ignoring request" << endl;
    } else {
      pthread_t clientThread;

      pthread_create(&clientThread, NULL, &receiveRequest, (void*) new int(newSock));
      pthread_detach(clientThread);
    }
  }
}
