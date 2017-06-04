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
<<<<<<< HEAD
#include <vector>
=======
>>>>>>> server_work
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

<<<<<<< HEAD
  if (newString.length() > size) {
    cerr << "TOO LONG!" << endl;
    exit(-1); // too long
  }
=======
>>>>>>> server_work
  size++;
  char msg[size];
  strcpy(msg, newString.c_str());
  msg[size - 1] = '\n'; // Always end message with terminal char

<<<<<<< HEAD
  cout << "FINAL SIZE " << size << endl;
  cout << "MESSAGE " << msg << endl;
=======
>>>>>>> server_work
  int bytesSent = send(sock, (void *) msg, size, 0);
  if (bytesSent != size) {
    cerr << "TRANSMISSION ERROR" << endl;
    exit(-1);
  }
}

<<<<<<< HEAD
string read(int messageSizeBytes, int socket) {//, sem_t &recSend) {
  cout << "RECEIVING TRANSMISSION NOW" << endl;
=======
string read(int messageSizeBytes, int socket) {
>>>>>>> server_work
  int bytesLeft = messageSizeBytes; // bytes to read
  char buffer[messageSizeBytes]; // initially empty
  char *bp = buffer; //initially point at the first element
  while (bytesLeft > 0) {
    int bytesRecv = recv(socket, (void *)bp, bytesLeft, 0);
    if (bytesRecv <= 0) {
      cerr << "Error receiving message" << endl;
      exit(-1);
    }
<<<<<<< HEAD
    cout << bytesLeft << "BYSTES LEFT" << buffer << "BUFFER SO FAR" << endl;
    bytesLeft = bytesLeft - bytesRecv;
    bp = bp + bytesRecv;
  }
  cout << "MESSAGE RECEIVED" << endl;
  cout << buffer << endl;
  // sem_post(&recSend);
=======
    bytesLeft = bytesLeft - bytesRecv;
    bp = bp + bytesRecv;
  }
>>>>>>> server_work

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

<<<<<<< HEAD
=======
int getInput() {
  int i;
  if ( cin>>i )
    return i;
  else
    cin.clear();
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return 100000; // Something intentionally out of range.
}

>>>>>>> server_work
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

<<<<<<< HEAD
  int socket = getSocket(IPAddr, servPort);

  unsigned short nameLength = htons(short(playerName.length()));
  cout << to_string(nameLength).length();
  cout << "NAME LENGTH " << nameLength << endl;
  cout << "NAME LENGTH String " << to_string(nameLength) << endl;
=======
  playerName += string("  "); // Breathing room: makes printing prettier
  int socket = getSocket(IPAddr, servPort);

  unsigned short nameLength = htons(short(playerName.length()));
>>>>>>> server_work

  send(to_string(nameLength), socket, 5); // Send name length before name so server know how long it should be
  read(4, socket); // Wait for AWK

  send(playerName, socket, playerName.length());

  read(4, socket); // Wait for AWK

  int playerGuess;
  int turn = 1;
  bool correct = false;

<<<<<<< HEAD
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
      correct = true;
    } else {
      turn++;
=======
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
>>>>>>> server_work
    }
  }

  unsigned short turns = htons(short(turn));
<<<<<<< HEAD
  cout << "Turns " << turns << "length" << to_string(turns).length() <<endl;
  send(to_string(turns), socket, 5);

  // string leaderBoardLength = read(6, socket); // Initial request to know how big name is;
  // int boardLength = short(ntohs(stol(leaderBoardLength)));

  // send(string("AWK"), socket, 3); // Awk request

  string leaderBoard = read(501 , socket);
  replace( leaderBoard.begin(), leaderBoard.end(), '&', '\n');

  string leaderBoardSans0 = remove_leading(leaderBoard, '0');
  cout << "Leader board:\n";
  cout << leaderBoardSans0 << endl;
=======
  send(to_string(turns), socket, 100);

  // Logic to read the unformatted leader board from server

  string leaderBoard = read(501, socket);
  replace( leaderBoard.begin(), leaderBoard.end(), '&', '\n'); // Format for pretty printing.

  string leaderBoardSans0 = remove_leading(leaderBoard, '0');
  cout << "Leader board:\n";
  cout << leaderBoardSans0;
>>>>>>> server_work

  close(socket);
}