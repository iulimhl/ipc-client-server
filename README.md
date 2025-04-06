# Client-Server Command System (C, IPC)

A C-based client-server application for authenticated command execution using inter-process communication mechanisms.

## Features
- Command-based protocol using newline-delimited input and byte-prefixed responses
- Authentication with `login : username`, where username must exist in the users.txt file
- Commands include:
  - `get-logged-users` – lists currently logged-in users
  - `get-proc-info : pid` – shows info from `/proc/<pid>/status`
  - `logout`, `quit`
- Communication handled via pipes, FIFOs, and socketpairs
- Commands executed in child processes (no exec/system used)

## Demo
![Video](demo2.mp4) 

## Technologies
- C
- Linux
- Pipes, FIFO, Socketpair
- File parsing

## How to Run
1. Open two terminals.
2. Compile the server:
   g++ server.c -o server
   ./server
3. Compile the client:
   gcc slienti.c -o client
   ./client

