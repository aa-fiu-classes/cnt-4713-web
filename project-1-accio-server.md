---
layout: page
title: "Project 1: \"Accio\" File using TCP (Server)"
group: "Project 1"
---

# Accio Server

The Accio server is an extension of the simplified server that:
- processes multiple simultaneous connections in parallel
- saves the received data into the specified folder.

## Revisions

Not yet

## Server Application Specification

The server application MUST be implemented in `server.py` Python file, accepting two command-line arguments:

    $ python3 server.py <PORT> <FILE-DIR>

- `<PORT>`: port number on which server will listen on connections.  The server must accept connections coming from any interface.
- `<FILE-DIR>`: directory name where to save the received files.

For example, the command below should start the server listening on port `5000` and saving received files in the directory `/save`.

    $ python3 server.py 5000 /save

**Requirements**:

- The server must open a listening socket on the specified in the command line port number on all interfaces. In order to do that, you should hard-code `0.0.0.0` as a host/IP in `socket.bind` method.

- The server should gracefully process an incorrect port number and exit with a non-zero error code.  In addition to exit, the server must print out on standard error (using `sys.stderr.write()`) an error message that starts with `ERROR:` string.

- The server should exit with code zero when receiving `SIGQUIT`, `SIGTERM`, `SIGINT` signal

- The server should be able to accept and process multiple connections (at least 10) from clients at the same time

   * To test, you can open multiple `telnet` sessions to your server.  All of them should show `accio` command.

- The server must count all established connections (1 for the first connect, 2 for the second, etc.).  The received file over the connection must be saved to `<FILE-DIR>/<CONNECTION-ID>.file` file  (e.g., `/save/1.file`, `/save/2.file`, etc.).  If the client does not send any data during gracefully terminated TCP connection, the server should create an empty file with the name that corresponds to the connection number.

   **NOTE** DO NOT invent anything else with the file name. The server should name files as `1.file`, `2.file`, etc. in the specified folder REGARDLESS how they were named originally.  Your client MUST NOT transmit any additional information, besides what is defined in the specification.  The size of the file is equal to all of the received data of the connection!

- The server must assume an error if no data received from the client for over `10 seconds`.  It should abort the connection and write a single `ERROR` string (without end-of-line/carret-return symbol) into the corresponding file.  Note that any partial input must be discarded.

- The server should be able to accept large enough files (`100 MiB` or more)

## Hints

The suggested way to support simultaneous connections is to introduce threading. Overall, the structure should be following:

- Main thread (main program)

   * processes command-line params
   * creates socket, listen, and has a while loop to accept connections
   * accept connection
       - increase global connection counter
       - create a new thread and pass the accepted connection socket and the value of the global connection counter
   * return to waiting for a new connection
   
- "Child" thread

   * do all the work processing a single connection

## Submission Requirements

Refer to [Project 1 submission requirements](project-1.html#Genreral-Submission-Requirements)

## Grading

Your code will then be automatically tested in some testing scenarios.

Note that your implementation will be tested against a reference implementation. Projects receive full credit if only all these checks are passed.

### Grading Criteria

Maximum points: 100

1. (5 pts) At least 3 new git commits
1. (5 pts) Server handles incorrect port
1. (5 pts) Server **gracefully** handles `SIGINT` signal
1. (5 pts) Server accepts a connection
1. (5 pts) Server accepts a connection and sends `accio\r\n` command
1. (5 pts) Server starts receiving data
1. (5 pts) Server accepts another connection after the first connection finished
1. (5 pts) When server receives and process 10 connections simultaneously
1. (5 pts) Server aborts connection and write ERROR into corresponding file (resetting any received content) when it does not receive new data from client for more than 10 seconds.
1. (5 pts) Server accepts another connection after the first connection timed out
1. (5 pts) Server successfully receives a small file (~500 bytes) using the submitted version of the client (from part 1)
1. (5 pts) Server correctly saves the file from the previous test
1. (5 pts) Server successfully receives a small file (~500 bytes) using the instructor's version of the client
1. (5 pts) Server correctly saves the file from the previous test
1. (5 pts) Server successfully receives sequentially 10 large files (~10 MiBytes) using the submitted version of the client (from part 1) and saves all the files from the previous test in `1.file`, `2.file`, ... `10.file`
1. (5 pts) Same as previous but in parallel
1. (5 pts) Server successfully receives sequentially 10 large files (~10 MiBytes) using the instructor's version of the client (from part 1) and saves all the files from the previous test in `1.file`, `2.file`, ... `10.file`
1. (5 pts) Same as previous but in parallel
1. (5 pts) Server successfully receives sequentially 10 large files (~10 MiBytes) using the instructor's version of the client (from part 1) and saves all the files from the previous test in `1.file`, `2.file`, ... `10.file` (**with** emulated delays and/or transmission errors)
1. (5 pts) Same as previous but in parallel

Note that you may receive deductions if your repository contains temporary files or your submission is lacking the required README file.
