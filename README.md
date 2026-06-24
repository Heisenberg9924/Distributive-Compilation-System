# Distributed Compilation System

A distributed client-server system that enables remote compilation and execution of C programs across multiple worker nodes. The system dynamically selects the least loaded worker server, transfers executables, executes them remotely, and returns execution results to the client.

## Features

- Distributed compilation and execution
- Dynamic worker selection based on CPU load
- TCP socket-based communication
- Multithreaded server architecture using POSIX threads
- Remote execution and output retrieval
- Linux-based system monitoring
- Concurrent client handling

## Architecture

Client
|
|-- Queries worker load
|
|-- Selects least loaded worker
|
|-- Sends executable
|
V
Worker Server
|
|-- Receives executable
|-- Executes program
|-- Captures output
|-- Sends results back
|
V
Client Displays Output

## Tech Stack

- C Programming
- POSIX Threads (pthreads)
- TCP/IP Socket Programming
- Linux System Calls
- GCC Compiler


