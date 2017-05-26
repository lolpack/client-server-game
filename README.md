# client-server-game
Multi Threaded Client Server Game in C++

Usage

`$ make`

Run the server with a port

`./server 11700`

Run the client with an IP and port

`./client 0.0.0.0 11700`

You will be prompted to enter your name. This will generate a random number (0-9999) on the server. Now you will enter guesses (0-9999) into the client that the server will check. The server returns a "closeness" score. This is calculated by taking sum of differences of the four digits between the guess and the random number. Once you win, you will see the leader board.

*Only 10 clients allowed at once*