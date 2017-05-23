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
  string newString = string(size - msgStr.length(), '0') + msgStr;

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

string read(int messageSizeBytes, int socket) {//, sem_t &recSend) {
  cout << "RECEIVING TRANSMISSION NOW" << endl;
  int bytesLeft = messageSizeBytes; // bytes to read
  char buffer[messageSizeBytes]; // initially empty
  char *bp = buffer; //initially point at the first element
  while (bytesLeft > 0) {
    int bytesRecv = recv(socket, (void *)bp, bytesLeft, 0);
    if (bytesRecv <= 0) {
      cerr << "Error receiving message" << endl;
      exit(-1);
    }
    cout << bytesLeft << "BYSTES LEFT" << buffer << "BUFFER SO FAR" << endl;
    bytesLeft = bytesLeft - bytesRecv;
    bp = bp + bytesRecv;
  }
  cout << "MESSAGE RECEIVED" << endl;
  cout << buffer << endl;
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
    cerr << "Example: ./client 0.0.0.0 11700" << endl;
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
  cout << "NAME LENGTH " << nameLength << endl;
  cout << "NAME LENGTH String " << to_string(nameLength) << endl;

  send(to_string(nameLength), socket, 5); // Send name length before name so server know how long it should be
  read(4, socket); // Wait for AWK

  send(playerName, socket, playerName.length());

  read(4, socket); // Wait for AWK

  int playerGuess;
  int turn = 1;
  bool correct = false;

  while (!correct) {
    cout << "Turn: " << turn << endl;
    cout << "Enter a guess: ";
    cin >> playerGuess;

    unsigned short guess = htons(short(playerGuess));
    cout << "GUESS " << guess << "length" << to_string(guess).length() <<endl;
    send(to_string(guess), socket, 5);

    string resultOfGuess = read(5, socket); // Wait for AWK
    int result = short(ntohs(stol(resultOfGuess)));

    cout << "Result of guess: " << result << endl;

    if (result == 0) {
      cout << "Congratulations! It took " << turn << " turns to guess the number!"  << endl;
      correct = false;
    } else {
      turn++;
    }
  }

  unsigned short turns = htons(short(playerGuess));
  cout << "Turns " << turns << "length" << to_string(turns).length() <<endl;
  send(to_string(turns), socket, 5);

  close(socket);

}