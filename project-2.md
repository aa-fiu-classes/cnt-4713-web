---
layout: page
title: "Project 2: \"Confundo\" Transport Protocol over UDP "
group: "Project 2"
---

* toc
{:toc}

**Revisions**

Not yet

## Overview

In this project you will need to implement `Confundo`, a basic version of reliable data transfer protocol, including basics of connection establishment and congestion control.
You will implement Confundo protocol in context of server and client applications, where client transmits a file as soon as the connection is established (same as in project 1).

All implementations should be written in Python [BSD sockets](http://en.wikipedia.org/wiki/Berkeley_sockets).
**No high-level network-layer abstractions are allowed in this project.**
You are allowed to use some high-level abstractions for parts that are not directly related to networking, such as string parsing.

For simplicity, we will **NOT** use any multi threading or parallelism in project 2.  Just a single connection at a time.

The objective of this project is to deepen your understanding on how TCP protocol works and specifically how sequence number operate, how retransmissions happen, and specifics of congestion control advancement.

You are required to use `git` to track the progress of your work. **The project can receive a full grade only if the submission includes git history no shorter than 3 commits FROM ALL PARTICIPANTS OF YOUR GROUP.**  If commit history includes commits made only by one group member, other group members will receive **no credit**.

You are encouraged to host your code in private repositories on [GitHub](https://github.com/), [GitLab](https://gitlab.com), or other places.  At the same time, you are PROHIBITED to make your code for the class project public during the class or any time after the class.  If you do so, you will be violating academic honestly policy that you have signed, as well as the student code of conduct and be subject to serious sanctions.
{: class="alert alert-warning"}

## Task Description

The project includes implementation of a new Confundo Socket, which is API compatible with regular TCP socket, but implements everything on top of UDP protocol, according to the defined specification.
There is a lot of implementation in the skeleton code, and the minimal base implementation can be done by just meaningfully updating the marked pieces in `confundo/socket.py` code.  You do not have to follow the exact logic there and are free to completely rewrite the code.

You should be able to use the new Confundo.Socket in your project 1 implementation, just make sure you don't use multi threading as things will potentially "explode".

The skeleton does not implement anything regarding congestion control, which is your responsibility to implement.

Also, the skeleton has a very simplistic implementation of the sending logic (stop and go, sending a single segment and waiting for the ACK).
To get max credit for the project, you would need to improve this logic.

Note that it is your responsibility to update/set any constants (e.g., in `confundo/common.py`) according to the written specification.

The project includes an extended skeleton code that you can find here: [https://github.com/aa-fiu-classes/fall21-project2](https://github.com/aa-fiu-classes/fall21-project2).  Additional instructions are below.

### Confundo Protocol Specification

**Header Format**

- The payload of each UDP packet sent by server and client MUST start with the following `12-byte` header.  All fields are in network order (most significant bit first):

         0                   1                   2                   3
         0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |                        Sequence Number                        |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |                     Acknowledgment Number                     |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |         Connection ID         |         Not Used        |A|S|F|
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

  Where

    - `Sequence Number` (32 bits): The sequence number of the first data octet in this packet (except when SYN is present). If SYN is present the sequence number is the initial sequence number (ISN) and the first data octet is ISN+1.

        The sequence number is given in **the unit of bytes**.

    - `Acknowledgement Number` (32 bits): If the ACK control bit is set this field contains the value of the next sequence number the sender of the segment is expecting to receive.  Once a connection is established this is always sent.

        The acknowledgement number is given in **the unit of bytes**.

    - `Connection ID` (16 bits): A number representing connection identifier.

    - `Not Used` (13 bits): Must be zero.

    - `A` (ACK, 1 bit): Indicates that there the value of `Acknowledgment Number` field is valid

    - `S` (SYN, 1 bit): Synchronize sequence numbers (Confundo connection establishment)

    - `F` (FIN, 1 bit): No more data from sender (Confundo connection termination)

**Requirements**

- The maximum UDP packet size is `424 bytes` including a header (`412 bytes` for the payload)

- MTU size for congestion control operations is 412 (payload size)

- The maximum sequence and acknowledgment number should be `50000` and be reset to zero whenever it reaches the maximum value.

- Packet retransmission (and appropriate congestion control actions) should be triggered when no data was acknowledged for more than `0.5 seconds` (fixed retransmission timeout).

- Initial and minimum congestion window size (`CWND`) should be `412`

- Initial slow-start threshold (`SS-THRESH`) should be `12000`

- Initial sequence number should be `50000`

- If `ACK` field is not set, `Acknowledgment Number` field should be set to 0

- `FIN` should take logically one byte of the data stream (same as in TCP, see examples)

- `FIN` and `FIN | ACK` packets must not carry any payload

### Client Application Specification

The client application MUST be written in `client.py` file (you are allowed to create other `.py` files and modules), accepting three command-line arguments:

    $ python3 ./client.py <HOSTNAME-OR-IP> <PORT> <FILENAME>

- `<HOSTNAME-OR-IP>`: hostname or IP address of the server to connect (send UDP datagrams)
- `<PORT>`: port number of the server to connect (send UDP datagrams)
- `<FILENAME>`: name of the file to transfer to the server after the connection is established.

For example, the command below should result in connection to a server on the same machine listening on port 5000 and transfer content of `file.txt`:

    $ python3 ./client.py localhost 5000 file.txt

**Requirements**:

- The client must open a UDP socket and initiate 3-way handshake to the specified hostname/ip and port

  * Send UDP packet `src-ip=DEFAULT, src-port=DEFAULT, dst-ip=HOSTNAME-OR-IP, dst-port=PORT` with `SYN` flag set, `Connection ID` initialized to `0`, `Sequence Number` set to `77`, and `Acknowledgement Number` set to `0`

  * Expect response from server with `SYN | ACK` flags.  The client must record the returned `Connection ID` and use it in all subsequent packets.

  * Send UDP packet with `ACK` flag including the first part of the specified file.

- The client should gracefully process incorrect hostname and port number and exist with a non-zero exit code (you can assume that the specified file is always correct).  In addition to exit, the client must print out on standard error (using `sys.stderr.write()`) an error message that starts with `ERROR:` string.

- After file is successfully transferred file (all bytes acknowledged), the client should gracefully terminate the connection

  * Send UDP packet with `FIN` flag set

  * Expect packet with `ACK` flag

  * Wait for `2 seconds` for incoming packet(s) with `FIN` flag (`FIN-WAIT`)

  * During the wait, respond to each incoming `FIN` with an `ACK` packet; drop any other non-`FIN` packet.

  * Close connection and terminate with code zero.

                 Client3                                   Server
                    |                                        |
                   ...                                      ...
                    |       (10000 bytes transferred)        |
                    |                                        |
                    |      seq=22346, ack=0, id=3, FIN       |
                    | -------------------------------------> |
                    |  (Confudo prohibits payload in FIN)    |
                    |                                        |
                    |     seq=4322, ack=22347, id=3, ACK     |
              +-->  | <------------------------------------- |
              |     |                                        |
              |    ...                                      ...
              |     |      seq=4322, ack=0, id=3, FIN        |
           2  |     | <------------------------------------- |
              |     |    seq=22347, ack=4323, id=3, ACK      |
           s  |     | -------------------------------------> |
           e  |     |                                        |
           c  |    ...          (if ACK is lost)            ...
           o  |     |                                        |
           n ...    |      seq=4322, ack=0, id=3, FIN        |
           d  |     | <------------------------------------- |
              |     |    seq=22347, ack=4323, id=3, ACK      |
           w  |     | -------------------------------------> |
           a  |     |                                        |
           i  |    ...                                     close
           t  |     |                                    connection
              |     |
              |     |
              +-->  |
                  close
                connection

- Client should support transfer of large files that do not fit into RAM, i.e., you cannot read file into memory and send it or receive all in memory and then write.

- Whenever client receives no packets from server for more than `10 seconds`, it should abort the connection (close socket and exit with non-zero code)

### Congestion Control Requirements

Client and server (mostly client) is required to implement TCP Tahoe congestion window maintenance logic (without 3-dup ACK loss detection):

- After connection is established, the client should send up to `CWND` bytes of data without starting the wait for acknowledgements

- After each `ACK` is received

  * (*Slow start*) If `CWND < SS-THRESH`: `CWND` += 412

  * (*Congestion Avoidance*) If `CWND >= SS-THRESH`: `CWND` += (412 * 412) / CWND

- After timeout, set `SS-THRESH -> CWND / 2`, `CWND -> 412`, and retransmit data after the last acknowledged byte.

- For each valid packet of the connection (except packets with only `ACK` flag and empty payload), the server responds with an `ACK` packet, which includes the next expected in-sequence byte to receive (cumulative acknowledgement).

### Misc Requirements

- The following output MUST be written to the standard output (using `print()`) in the **exact format defined**.  You will get no credit if the format is not followed exactly and our test script cannot automatically parse it.  If any other information needs to be shown, it MUST be written to the standard error (using `sys.stderr.write()`)

  * Packet received:

        "RECV" <Sequence Number> <Acknowledgement Number> <Connection ID> <CWND> <SS-THRESH> ["ACK"] ["SYN"] ["FIN"]

    - `[xx]` means that `xx` value is optional
    - `"xx"` means `xx` string should appear on the output
    - `<yy>` means that value of `yy` variable should appear on the output

    For example:

            RECV 4322 22347 3 1024 512 ACK
            RECV 4322 0 3 1024 512 FIN

  * If received packet is dropped (e.g., unknown `connection ID`):

        "DROP" <Sequence Number> <Acknowledgement Number> <Connection ID> ["ACK"] ["SYN"] ["FIN"]

  * Packet sent:

        "SEND" <Sequence Number> <Acknowledgement Number> <Connection ID> <CWND> <SS-THRESH> ["ACK"] ["SYN"] ["FIN"] ["DUP"]


## A Few Hints

The best way to approach this project is in incremental steps.  Do not try to implement all of the functionality at once.

- Read and understand the skeleton code, especially the marked out areas.  The skeleton code will NOT work (has syntax errors) until those areas are corrected.

- Do initial implementation regarding the marked out areas and test it with the provided remote server.

- Adopt a version of your project 1 server to work with Confundo Socket and do local testing.

- Add necessary "DROP" and "DUP" statements, as these are not in the skeleton.

After these are done, move to more complex steps:

- Add congestion control logic

- Extend stop-an-go to a more efficient sending/ack/retx logic.

### Submission Hints

Checklist:

- Are your cout statements correct? You should not have any other `print()` or `sys.stdout.write()` statements besides the one in the spec, they should be in the right format, printed at the right time with the right optional outputs, and with the correct `cwnd` and `ssthresh` values.

    - No other cout statements, others should be usig `sys.stderr.write()`
    - `print()` statements are in the exact correct format
    - You are adding in the `[ACK]` `[SYN]` `[FIN]` `[DUP]` optional output correctly. DUP in particular is easy to miss.
    - Are you printing a statement every time you should be? Is there a case where you receive something but don't print it, etc.

- Are your timers correct?

    - Is your 0.5 second retransmission working properly?
    - Are you waiting the 2 seconds before closing the client?
    - Do you have the 10 second timer working properly?
    - Is there a place where you should be updating your timer threshold (e.g., restarting when it begins to countdown) but you aren't?

- Is there a memory error? We had a couple and are not sure if this error is directly related but fixing these will help you chase down this error more easily. Most of these should be caught

    -  Are you ever dereferencing a pointer that isn't pointing at anything?
    -  Do you properly free your memory for all of your allocated objects?
    -  Do you ever invalidate an iterator by mistake?
    -  Do you somehow allocate so much memory that the kernel has to send a SIGKILL to terminate your program? This really shouldn't happen realistically but you never know...

- Do you ever somehow go into an infinite loop by accident?

- Do you ever unintentionally set your error code to something other than 0 where it should be? If so then your error might look similar but with something besides -9...

This error is especially baffling because it can sometimes occur for some test cases, sometimes for others.  For us, it was periodically failing 2.3, 2.4, and 2.9.  If it is periodically failing then it is likely something to do with a process that doesn't occur every time, such as a timer error, a drop packet `std::cout`, a retransmission error, or bug in loss control.

### Emulating packet loss

If are using the [Vagrantfile provided in project-2 skeleton](https://github.com/aa-fiu-classes/fall21-project2), you can automatically instantiate two virtual machines that are connected to each other using a private network (`eth1` interface on each).  You can also run preinstalled `/set-loss.sh` script to enable emulation of 10% loss and delay of 20ms in each direction (need to run on each VM separately).

You can use the following commands to adjust parameters of the emulation:

- To check the current parameters for the private network (`eth1`)

        sudo tc qdisc show dev eth1

- To change current parameters to loss rate of 20% and delay 100ms:

        sudo tc qdisc change dev eth1 root netem loss 20% delay 100ms

- To delete the network emulation:

        sudo tc qdisc del dev eth1 root

- If network emulation hasn't yet setup or you have deleted it, you can add it to, e.g., 10% loss without delay emulation:

        sudo tc qdisc add dev eth1 root netem loss 10%

- You can also change network emulation to re-order packets.  The command below makes 4 out of every 5 packets (1-4, 6-9, ...) to be delayed by 100ms, while every 5th packet (, 10, 15, ...) will be sent immediately:

        sudo tc qdisc change dev eth1 root netem reorder 100% gap 5 delay 100ms

        # or if you're just adding the rule
        # sudo tc qdisc add dev eth1 root netem reorder 100% gap 5 delay 100ms

- More examples can be found in [Network Emulation tutorial](http://www.linuxfoundation.org/collaborate/workgroups/networking/netem)

If you are not using the provided Vagrant, you should adjust network interface to one that matches your experiments.

## Environment Setup

The best way to guarantee full credit for the project is to do project development using a Ubuntu 18.04-based virtual machine.

You can easily create an image in your favourite virtualization engine (VirtualBox, VMware) using the Vagrant platform and steps outlined below.

### Set up Vagrant and create VM instance

**Note that all example commands are executed on the host machine (your laptop), e.g., in Terminal.app (or iTerm2.app) on macOS, cmd in Windows, and console or xterm on Linux.  After the last step (`vagrant ssh`) you will get inside the virtual machine and can compile your code there.**

- Download and install your favourite virtualization engine, e.g., [VirtualBox](https://www.virtualbox.org/wiki/Downloads)

- Download and install [Vagrant tools](https://www.vagrantup.com/downloads.html) for your platform

- Set up project and VM instances

  * Clone project template

        git clone https://github.com/aa-fiu-classes/fall21-project2 ~/cnt4713-proj2
        cd ~/cnt4713-proj2

  * Initialize VMs, one to run the client app and the other to run the server app

        vagrant up

        # Or you can set up them individually
        # vagrant up client
        # vagrant up server

    Do not start VM instance manually from VirtualBox GUI, otherwise you may have various problems (connection error, connection timeout, missing packages, etc.)
    {: class="alert alert-warning"}

  * To establish an SSH session to the created VM, run

    To ssh to the client VM

        vagrant ssh
        # or vagrant ssh client

    To ssh to the server VM

        vagrant ssh server

  If you are using PuTTY on Windows platform, `vagrant ssh` (`vagrant ssh server`) will return information regarding the IP address and the port to connect to your virtual machine.

- Work on your project

  All files in `~/cnt4713-proj2` folder on the host machine will be automatically synchronized with `/vagrant` folder on both virtual machines.  For example, to compile your code, you can run the following commands:

        vagrant ssh
        cd /vagrant
        make

### Notes

* If you want to open another SSH session, just open another terminal and run `vagrant ssh` (or create a new Putty session).

* The client and server VMs are connected using the private network `10.0.0.0/24` (`eth1`)

  - client's IP address: `10.0.0.2`
  - server's IP address: `10.0.0.1`

  Note that these addresses do not mean that your server and client must support only this environment.  The server and client should be able to work a linux host with any other IP addresses as well.

* If you are using Windows, read [this article](http://www.sitepoint.com/getting-started-vagrant-windows/) to help yourself set up the environment.

* The code base contains the basic `Makefile` and two empty files `server.py` and `client.py`.

        $ vagrant ssh
        vagrant@client:~$ cd /vagrant
        vagrant@client:/vagrant$ ls -a
        .  ..  client.py  confundo.lua  .gitignore  README.md  server.py  .vagrant  Vagrantfile

* You are now free to add more files and modify the Makefile to make the `server.py` and `client.py` full-fledged implementation.

## Submission Requirements

To submit your project:

1. Create a `README.md` file placed in your code that includes:

    * Name and UID of each team member (up to 3 members in one team) and the contribution of each member
    * The high level design of your server and client
    * The problems you ran into and how you solved the problems
    * List of any additional libraries used
    * Acknowledgement of any online tutorials or code example (except class website) you have been using.

    **If you need additional dependencies for your project, you must update Vagrant file.**

2. Submit to Gradescope via connection to Github. Your repository should include all your source code (`client.py`, `server.py`, and any other Python files you have created) and `README.md`. Make sure you do NOT include temporary files (deductions will be made if so).

Before submission, please make sure the client and server conforms to the specification and you did not implement what implementation does not require.

Submissions that do not follow these requirements will not get any credit.
{: class="bs-callout bs-callout-warning" }

## Grading

Your code will be first checked by a software plagiarism detecting tool. If we find any plagiarism, you will not get any credit.

Your code will then be automatically tested in some testing scenarios. If your code can pass all our automated test cases, you will get the full credit.

We may test your server against a "standard" implementation of the client, your client against a "standard" server, as well as your client against your server.  Projects receive full credit only if all these checks are passed.

### Grading Criteria

Tests are subject to changes

1. Miscellaneous tests

    1.1  (2.5 pts, public) At least 3 git commits (at least one from each group member): MUST include copy of `git log --pretty=format:"%h%x09%an%x09%ad%x09%s"`

2. Client tests

    2.1. (5 pts, public) Client sends SYN packet with correct values in its header

    2.2. (5 pts, public) Client has correct initial values Sequence Number

    2.3. (5 pts, public) Data segments that client sends are not exceeding 412 bytes and on average larger than 370 bytes (for 1~MByte file)

    2.4. (5 pts, public) Client should reset its sequence number to zero when the sequence number reaches the maximum value (2.5pts if wraps, 2.5pts if wraps correctly)

    2.5. (5 pts, public) Client sends a FIN packet after transmitting a file

    2.6. (5 pts, public) After finishing connection, client responds with ACK for incoming FINs for 2 seconds, dropping packets for this connection afterwards

    2.7. (10 pts, public) Client successfully transmits a small file

    2.8. (5 pts, public) Client aborts the connection if no incoming packets for more than 10 seconds

    2.9. (5 pts, public) Client has correct initial values for CWND, SS-THRESH

    2.10. (5 pts, public) Client properly increases congestion window size in slow start phase

    2.11. (5 pts, public) Client properly increases congestion window size in congestion avoidance phase

    2.12. (10 pts, public) Client detects and retransmits lost data segments

    2.13. (5 pts, public) Client sets SS-THRESH and CWND values properly after timeout

    2.14. (10 pts, public) Client successfully transmits a small file over a lossy link with delay

    2.15. (10 pts, public) Client successfully transmits a large file over a lossy link with delay

