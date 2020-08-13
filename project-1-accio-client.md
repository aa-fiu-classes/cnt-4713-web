---
layout: page
title: "Project 1: \"Accio\" File using TCP (Client)"
group: "Project 1"
---

# Accio Client

Accio client is a relatively simple application that connects to a server, expects to receive `accio` command from the server, and sends the content of a file to the server after the command is received.

## Revisions

Not yet

## Client Application Specification

The client application MUST be implemented in `client.py` file, accepting three command-line arguments:

    $ ./client.py <HOSTNAME-OR-IP> <PORT> <FILENAME>

- `<HOSTNAME-OR-IP>`: hostname or IP address of the server to connect
- `<PORT>`: port number of the server to connect
- `<FILENAME>`: name of the file to transfer to the server after the connection is established.

For example, the command below should result in connection to a server on the same machine listening on port 5000 and transfer content of `file.txt`:

    $ ./client.py localhost 5000 file.txt

**Requirements**:

- The client must be able to connect to the specified server and port, receive `accio\r\n` command from the server, and  transfer the specified file, and gracefully terminate the connection.

- The client should not start sending anything until a full `accio\r\n` command has been received!

- The client should gracefully process incorrect hostname and port number and exit with a non-zero exit code (you can assume that the specified file is always correct).  In addition to exit, the client must print out on standard error (using `sys.stderr.write()`) an error message that starts with `ERROR:` string.

- Client application should exit with code zero after successful transfer of the file to server.  It should support transfer of reasonably large files (`100 MiB` or more).

- Client should handle connection and transmission errors.  The reaction to network or server errors should be **no longer that 10 seconds**:

    * Timeout to connect to server should be no longer than `10 seconds`
    * Timeout for not being able to send more data to server (not being able to write to send buffer) should be no longer than `10 seconds`.

    Whenever timeout occurs, the client should abort the connection, print an error string starting with `ERROR:` to standard error (using `sys.stderr.write()`), and exit with non-zero code.

## Hints

*  An incorrect port for the requirement above is one that is either outside the valid range for TCP port numbers (1-65535) or that is not a valid common protocol string (like `ssh`, `telnet`, `smtp`, and others; on Unix/Linux/Mac you can check the complete list in `/etc/services`).  In your implementation, you just need to properly handle exception for `socket.connect` call, as it will do all the checking for you.

## Approach to implement

- Create an empty private git repository on Github (must be private!).

- Write a basic skeleton for `client.py`:

  * Add command-line parameter processing either by simply looking at `sys.argv` (recommended) or using a more advanced parsing using `argparse` module.

- Using a task from the assignment 2, extend `client.py` to have routines to connect to the server/port specified as the command line parameter.

    * Test it with, for example, `python3 client.py www.google.com 80 file.txt` 
    
- Add routines to receive the command from the server.  Note that the server does not to have to send this command in a single TCP data segment.  In other words, `.recv` call may return a single byte.  Your routine must be prepared to receive the command in small parts (e.g., character by character, including `\r` and `\n`.

    * Test it with a demo server (IP address/port posted on class Piazza):  `python3 client.py demo-server demo-port file.txt`.

- Add routines to read file in chunks (e.g., 1000 bytes long) and send it over the socket.

    * Test again with the demo server.  You can check if the file was successfully received using instruction in Piazza.

- Add exception handling to gracefully handle disconnections

    * Test using the second "broken" demo server

- Add socket option and extend exception handling for 10 second timeout.

    * To test connection timeout, try to connect to a wrong port on a remote server, e.g., `python3 client.py www.google.com 8080 file.txt`.  Do not try localhost, as you will immediately get an ICMP error and will not be able to test the timeout function.

    * To test 10 second disconnection handling, use the "broken" demo server

