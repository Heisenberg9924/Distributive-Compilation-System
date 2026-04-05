# Distributed Remote Code Execution System

## Project Overview

This mini project implements a simple Distributed Remote Code Execution System in C using TCP sockets on Linux.

There are two roles in the system:

- Machine A: Client
- Machine B: Server

The client takes a `.c` or `.cpp` source file, compiles it locally, sends the compiled executable to a remote server, and receives the execution output back from the server. The server receives the executable, stores it, gives execute permission, runs it, captures the output, and sends the output file contents back to the client.

This project is designed to be simple, readable, and suitable for a college mini project demonstration.

## Objectives Completed

The implementation currently includes all of the following:

- Local compilation of `.c` files using `gcc`
- Local compilation of `.cpp` files using `g++`
- TCP client-server communication on Linux
- File transfer using a proper size-first protocol
- Transfer buffer size fixed to `1024` bytes
- Reliable send and receive loops to avoid partial-transfer corruption
- Remote saving of the executable on the server
- Execute permission assignment with `chmod()`
- Remote program execution using `fork()` + `exec()`
- Output capture into a file on the server
- Sending execution output back to the client
- Printing remote output on the client terminal
- Round-robin load balancing across 2 or more server IP addresses
- Basic support for multiple clients using `fork()` on the server
- Basic error handling for file and socket failures
- Basic execution timeout on the server

## Files Included

- `client.c`
  Client-side program that compiles source code locally, selects the next server using round robin, sends the executable, receives the output, and prints it.

- `server.c`
  Server-side program that listens on port `8080`, accepts client connections, receives the executable, runs it, captures output, and sends the output back.

- `Makefile`
  Build file with `all` and `clean` targets to compile the client and server programs.

- `README.md`
  Documentation for project description, implementation details, compilation, execution steps, and examples.

## Technologies and System Calls Used

The project uses only standard Linux libraries and system calls as required.

### Socket-related calls

- `socket()`
- `bind()`
- `listen()`
- `accept()`
- `connect()`
- `send()`
- `recv()`

### Process and execution calls

- `fork()`
- `exec()`
- `waitpid()`

### File-related calls

- `open()`
- `read()`
- `write()`
- `close()`
- `stat()`
- `fstat()`
- `chmod()`
- `unlink()`

### Other Linux/POSIX calls

- `setsockopt()`
- `dup2()`
- `kill()`
- `sigaction()`
- `nanosleep()`

## System Design

## Client Responsibilities

The client performs these steps:

1. Accepts a source file path and a list of 2 or more server IP addresses.
2. Checks whether the source file exists.
3. Detects whether the file is `.c` or `.cpp`.
4. Compiles the source file locally into a temporary executable in `/tmp`.
5. Selects the next server IP using round robin.
6. Connects to the selected server on port `8080`.
7. Sends the executable size first.
8. Sends the executable file data in chunks of `1024` bytes.
9. Waits for the server response.
10. Receives the output size first.
11. Receives the output data in chunks of `1024` bytes.
12. Prints the received output to the terminal.
13. Deletes the temporary local executable.

## Server Responsibilities

The server performs these steps:

1. Creates a TCP socket.
2. Binds to port `8080`.
3. Listens for incoming client connections.
4. Accepts a new client connection.
5. Forks a child process to handle that client.
6. Receives the executable size first.
7. Receives the executable file data and saves it to `/tmp`.
8. Gives execute permission to the received file.
9. Executes the file using `fork()` + `exec()`.
10. Redirects the program output to a temporary output file.
11. Applies a timeout of `5` seconds to stop long-running programs.
12. Reads the output file.
13. Sends the output file size first.
14. Sends the output file data to the client.
15. Removes temporary files after execution.

## File Transfer Protocol

To avoid data corruption and incomplete file transfer, a simple protocol is used on both client and server:

1. Send file size as a 64-bit unsigned integer in network byte order.
2. Send the file contents in chunks of `1024` bytes.
3. The receiver first reads the exact size value.
4. The receiver then continues reading until the entire file has been received.

The same protocol is used for:

- Sending the executable from client to server
- Sending the output file from server to client

This ensures that the receiver knows exactly how many bytes to expect.

## Round Robin Load Balancing

Basic load balancing is implemented in the client using round robin.

### How it works

- The client is run with 2 or more server IP addresses.
- The client stores the last used server index in a local file named `.client_rr_state`.
- On each new execution request, the client chooses the next server in the list.
- After the last server, selection returns to the first server.

### Example

If the client is run as:

```bash
./client sample.c 192.168.1.10 192.168.1.11 192.168.1.12
```

The selected server order across consecutive runs will be:

1. `192.168.1.10`
2. `192.168.1.11`
3. `192.168.1.12`
4. `192.168.1.10`
5. `192.168.1.11`

## Execution Model

The project uses `fork()` + `exec()` instead of `system()`.

### Why this approach was used

- It is closer to standard systems programming practice
- It gives better control over child process execution
- It avoids shell-based execution
- It makes redirection of output easier and cleaner

### On the client side

- A child process is created to run `gcc` or `g++`
- The parent waits for compilation to complete

### On the server side

- A child process handles each client connection
- Another child process is created to execute the received program
- Standard output and standard error are redirected to an output file using `dup2()`

## Multiple Client Support

The server supports multiple clients in a basic way.

- The main server process continues accepting new client connections
- For each accepted client, the server creates a child process using `fork()`
- Each child process handles one client independently

This is enough for a simple mini project demonstration of concurrent client handling.

## Timeout Support

Bonus timeout support has been added.

- The server waits for the uploaded executable to finish
- If it runs longer than `5` seconds, the server kills the program using `SIGKILL`
- In that case, the output file contains:

```text
Execution timed out on the server.
```

The timeout value is defined in `server.c` as:

```c
#define EXEC_TIMEOUT_SECONDS 5
```

## Error Handling Implemented

The project includes basic error handling for the required cases.

### Client-side errors

- Source file not found
- Unsupported source file type
- Compilation failure
- Invalid server IP address
- Socket creation failure
- Connection failure
- Send failure
- Receive failure

### Server-side errors

- Socket creation failure
- Bind failure
- Listen failure
- Accept failure
- File creation failure
- File read/write failure
- Permission change failure
- Execution failure
- Send failure

Errors are printed using `perror()` or clear user-facing messages where appropriate.

## Build Instructions

Compile the project using:

```bash
make
```

This creates:

- `client`
- `server`

To remove generated binaries:

```bash
make clean
```

## How To Run

## Step 1: Build the project

```bash
make
```

## Step 2: Start the server on each server machine

Run the following on each server machine:

```bash
./server
```

The server will listen on port `8080`.

Expected server message:

```text
Server listening on port 8080...
```

## Step 3: Create a sample source file on the client machine

Example `sample.c`:

```c
#include <stdio.h>

int main(void) {
    printf("Hello from remote machine!\n");
    return 0;
}
```

## Step 4: Run the client

Run the client with the source file and server IP addresses:

```bash
./client sample.c 192.168.1.10 192.168.1.11
```

You can also provide more servers:

```bash
./client sample.cpp 192.168.1.10 192.168.1.11 192.168.1.12
```

## Example Client Output

```text
Selected server: 192.168.1.10
Executable sent successfully. Waiting for remote output...
Hello from remote machine!
```

On the next run, round robin selects the next server:

```text
Selected server: 192.168.1.11
Executable sent successfully. Waiting for remote output...
Hello from remote machine!
```

## Example Server Output

```text
Server listening on port 8080...
Accepted connection from 192.168.1.20
```

## Verified During Implementation

The following was completed and checked during implementation:

- `client.c` compiled successfully
- `server.c` compiled successfully
- `make` completed successfully
- End-to-end localhost testing was performed
- Verified flow:
  client compiled a test source file
  executable was sent to the server
  server received and ran the executable
  server captured output
  output was sent back to the client
  client printed the returned output

## Makefile Targets

The `Makefile` contains:

### `all`

Builds both:

- `client`
- `server`

### `clean`

Removes:

- `client`
- `server`

## Important Notes

- This project is intended for LAN-based demonstration and academic use
- The server executes the uploaded binary directly, so only trusted code should be used
- Temporary executable and output files are created in `/tmp`
- The round-robin state file is stored as `.client_rr_state` in the client working directory
- Port number is fixed as `8080`
- Buffer size is fixed as `1024`

## Future Improvements

Possible future extensions for this project:

- Add authentication between client and server
- Add encrypted communication
- Add support for command-line arguments to the remote executable
- Add logging for all requests
- Add a configuration file for server IPs instead of command-line arguments
- Add stronger isolation for remote execution

## Conclusion

This project successfully demonstrates a distributed remote execution workflow in C using Linux sockets, file transfer, process creation, remote execution, output capture, and simple round-robin load balancing. It satisfies the requested project requirements and includes the bonus features of timeout handling, `fork()` + `exec()` based execution, and basic multiple-client support.
