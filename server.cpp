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
    return l.turns > r.turns; // Compare two winners turns. Least number of turns wins.
  }
};

priority_queue<Winner, vector<Winner>, compareWinners> *leaderBoard = new priority_queue<Winner, vector<Winner>, compareWinners>;
vector<Winner> tempLeaderBoard;

sem_t maxConcurrent;
sem_t leaderBoardLock;
int MAX_CONCURRENT_USERS = 10;

void send(string msgStr, int sock, int size) {
  string newString = string(size - msgStr.length(), '0') + msgStr;
  cout << "This code is hit!" << endl;
  if (newString.length() > size) {
    cerr << "TOO LONG!" << endl;
    exit(-1); // too long
  }
  size++;
  char msg[size];
  strcpy(msg, newString.c_str());
  msg[size - 1] = '\n'; // Always end message with terminal char

  cout << "FINAL SIZE " << size << endl;
  cout << "MESSAGE " << msg << endl;
  int bytesSent = send(sock, (void *) msg, size, 0);
  if (bytesSent != size) {
    cerr << "TRANSMISSION ERROR" << endl;
    exit(-1);
  }
}

string read(int messageSizeBytes, int socket) {
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
  string clientNameLength = read(6, localSockNum); // Initial request to know how big name is;

  send(string("AWK"), localSockNum, 3); // Awk request

  int randomNumber = rand() % 10000; // rand() return a number between ​0​ and 9999;

  cout << "RANDOM NUMBER " << randomNumber;;

  int nameLength = short(ntohs(stol(clientNameLength)));

  cout << "length of name: " << nameLength << endl;

  string name = read(nameLength, localSockNum);

  cout << "NAME: " << name << endl;
  // unsigned short nameLength = htons(short(localSockNum));

  send(string("AWK"), localSockNum, 3);

  bool correct = false;

  while (!correct) {
    string guessString = read(6, localSockNum);

    cout << "GUESS STRING " << guessString << endl;
    int guess = short(ntohs(stol(guessString)));

    cout << "SHORT " << short(ntohs(stol(guessString))) << endl;
    cout << "NTOHS " << ntohs(stol(guessString)) << endl;
    cout << "STOL " << stol(guessString) << endl;

    cout << "GUESS " << guess << endl;

    int diff = calculateDifference(guess, randomNumber);

    unsigned short sendiff = htons(short(diff));
    cout << "Diff:  " << sendiff << "length" << to_string(sendiff).length() << endl;
    send(to_string(sendiff), localSockNum, 5);

    if (sendiff == 0) {
      correct = true;
    }
    // sem_wait(&recSend);
  }

  string turnsResponse = read(6, localSockNum);

  int turns = short(ntohs(stol(turnsResponse)));
  cout << "TURNS " << turns << endl;

  Winner winner;

  winner.name = name;
  winner.turns = turns;

  sem_wait(&leaderBoardLock);

  leaderBoard->push(winner);

  string leaderBoardText;

  int topThree = 3; // Iterate through the first 3 in Pqueue or the number of values in Pqueue
  cout << "LEADER BOARD SIZE" << leaderBoard->size() << endl;
  if (leaderBoard->size() < 3) {
    topThree = leaderBoard->size();
  }

  for (int j = 0; j < topThree; j++) {
    Winner tempwin = leaderBoard->top();
    leaderBoard->pop();
    string eachRow = string(to_string(j + 1)) + string(". ") + string(tempwin.name) + string(" ") + string(to_string(tempwin.turns)) + string("&&");

    leaderBoardText = leaderBoardText + eachRow;

    tempLeaderBoard.push_back(tempwin);
  }

  cout << leaderBoardText << endl;

  for (int k = 1; k < topThree; k++) {
    leaderBoard->push(tempLeaderBoard.back()); // Take items temporarily in pQueue and put it back in vector;
    tempLeaderBoard.pop_back();
  }

  cout << "LEADER BOARD SIZE" << leaderBoard->size() << endl;

  sem_post(&leaderBoardLock);

  // unsigned short leaderBoardLength = htons(short(leaderBoardText.length()));
  // cout << to_string(leaderBoardLength).length();
  // cout << "LeaderBoard LENGTH " << leaderBoardLength << endl;
  // cout << "LeaderBoard LENGTH String " << to_string(leaderBoardLength) << endl;

  // send(to_string(leaderBoardLength), localSockNum, 5); // Send name length before name so server know how long it should be

  // read(4, localSockNum); // Wait for AWK

  send(leaderBoardText, localSockNum, 500);
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
