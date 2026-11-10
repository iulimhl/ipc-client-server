# Client-Server Commands with Unix IPC

A C++ exercise in communication between processes on Linux. The client sends text commands through named pipes, and the server uses child processes, pipes, and Unix socket pairs to handle them.

## Run on Linux

From the repository root, compile both programs with `g++`. The source files have `.c` extensions but use C++.

```sh
g++ server.c -o server
g++ slienti.c -o client
./server
```

In a second terminal in the same folder, run `./client`. Start with `login : <username>` using a name from `users.txt`. Try `get-logged-users` or `get-proc-info : 1`, then `logout` or `quit`. Process details come from Linux's `/proc` filesystem.

[Watch the demo](demo2.mp4)
