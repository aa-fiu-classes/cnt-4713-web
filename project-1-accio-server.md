---
layout: page
title: "Project 1: \"Accio\" File using TCP (Server)"
group: "Project 1"
---

# Accio Server

## Revisions

Not yet

## Server Application Specification

The server application MUST be implemented in `server.py` Python file, accepting two command-line arguments:

    $ ./server.py <PORT> <FILE-DIR>

- `<PORT>`: port number on which server will listen on connections.  The server must accept connections coming from any interface.
- `<FILE-DIR>`: directory name where to save the received files.

For example, the command below should start the server listening on port `5000` and saving received files in the directory `/save`.

    $ ./server.py 5000 /save

**Requirements**:

- The server must open a listening socket on the specified in the command line port number

- The server should gracefully process an incorrect port number and exit with a non-zero error code (you can assume that the folder is always correct).  In addition to exit, the server must print out on standard error (using `sys.stderr.write()`) an error message that starts with `ERROR:` string.

- The server should exit with code zero when receiving `SIGQUIT`/`SIGTERM` signal

- The server should be able to accept and process multiple connection from clients at the same time

- The server must count all established connections (1 for the first connect, 2 for the second, etc.).  The received file over the connection must be saved to `<FILE-DIR>/<CONNECTION-ID>.file` file  (e.g., `/save/1.file`, `/save/2.file`, etc.).  If the client doesn't send any data during gracefully terminated TCP connection, the server should create an empty file with the name that corresponds to the connection number.

- The server must assume an error if no data received from the client for over `10 seconds`.  It should abort the connection and write a single `ERROR` string (without end-of-line/carret-return symbol) into the corresponding file.  Note that any partial input must be discarded.

- The server should be able to accept and save files up to `100 MiB`


## Hints

Here are some hints of using multi-thread techniques to implement the server.

*  For the server, you may have the main thread listening (and accepting) incoming **connection requests**.
   * Any special socket API you need here?
   * How to keep the listening socket receiving new requests?

*  Once you accept a new connection, create a child thread for the new connection.
   * Is the new connection using the same socket as the one used by the main thread?
