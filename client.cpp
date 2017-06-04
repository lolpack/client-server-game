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
#include <cstring>
#include <algorithm>

using namespace std;

const int UNISIGNED_SHORT_LENGTH = 5;

template <typename T_STR, typename T_CHAR>
T_STR remove_leading(T_STR const & str, T_CHAR c)
{
  auto end = str.end();

  for (auto i = str.begin(); i != end; ++i) {
      if (*i != c) {
          return T_STR(i, end);
      }
  }

  // All characters were leading or the string is empty.
  return T_STR();
}

void send(string msgStr, int sock, int size) {
  string newString = string(size - msgStr.length(), '0') + msgStr;

  size++;
  char msg[size];
  strcpy(msg, newString.c_str());
  msg[size - 1] = '\n'; // Always end message with terminal char

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

int getInput() {
  int i;
  if ( cin>>i )
    return i;
  else
    cin.clear();
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return 100000; // Something intentionally out of range.
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

  playerName += string("  "); // Breathing room: makes printing prettier
  int socket = getSocket(IPAddr, servPort);

  unsigned short nameLength = htons(short(playerName.length()));

  send(to_string(nameLength), socket, 5); // Send name length before name so server know how long it should be
  read(4, socket); // Wait for AWK

  send(playerName, socket, playerName.length());

  read(4, socket); // Wait for AWK

  int playerGuess;
  int turn = 1;
  bool correct = false;

  cout << endl;

  while (!correct) {
    cout << "Turn: " << turn << endl;
    cout << "Enter a guess: ";

    playerGuess = getInput();
    if (playerGuess > 9999 ||  playerGuess < 0) {
      cout << "Guesses must be a number between 0000 and 9999!" << endl;
    } else {
      unsigned short guess = htons(short(playerGuess));
      send(to_string(guess), socket, 100);

      string resultOfGuess = read(101, socket); // Wait for AWK
      int result = short(ntohs(stol(resultOfGuess)));

      cout << "Result of guess: " << result << endl << endl;

      if (result == 0) {
        cout << "Congratulations! It took " << turn << " turns to guess the number!"  << endl << endl;
        correct = true;
      } else {
        turn++;
      }
    }
  }

  unsigned short turns = htons(short(turn));
  send(to_string(turns), socket, 100);

  // Logic to read the unformatted leader board from server

  string leaderBoard = read(501, socket);
  replace( leaderBoard.begin(), leaderBoard.end(), '&', '\n'); // Format for pretty printing.

  string leaderBoardSans0 = remove_leading(leaderBoard, '0');
  cout << "Leader board:\n";
  cout << leaderBoardSans0;

  close(socket);
}