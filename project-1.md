---
layout: page
title: "Project 1: \"Accio\" File using TCP"
group: "Project 1"
---

* toc
{:toc}

## Revisions

Not yet

## Overview

In this project, you will need to implement a simple client-server application that transfers a file over a TCP connection.
The project is split into 3 parts that need to be submitted independently before the corresponding deadlines.
All submissions should be done on Gradescope where your submission will be automatically checked.
You are allowed to resubmit any number of times.
For full credit, you must pass all automated tests.

All implementations should be written in Python [BSD sockets](http://en.wikipedia.org/wiki/Berkeley_sockets).
**No high-level network-layer abstractions (including `sendall` function) are allowed in this project.**
You are allowed to use some high-level abstractions for parts that are not directly related to networking, such as string parsing, multi-threading.

The objective of this project is to learn basic operations of BSD sockets, understand implications of using the API, as well as to discover common pitfalls when working with network operations.

You are required to use `git` to track the progress of your work. **The project can receive a full grade only if the submission includes git history no shorter than 3 commits.**
You should not commit any temporary files in your repository, otherwise points will be deducted.

Note: You are encouraged to host your code in private repositories on [GitHub](https://github.com/).  At the same time, you are PROHIBITED to make your code for the class project public during the class or any time after the class.  If you do so, you will be violating academic honestly policy that you have signed, as well as the student code of conduct and be subject to serious sanctions.
{: class="alert alert-warning"}

## Tasks Description

The project is split into three (non-equal) progressive parts with their individual deadlines:

- [Accio Client](project-1-accio-client.html)
- [Accio Server Simplified](project-1-accio-server-simplified.html)
- [Accio Server](project-1-accio-server.html)

## Environment Setup

The best way to guarantee full credit for the project is to do project development using a Ubuntu 16.04-based virtual machine.

You can easily create an image in your favourite virtualization engine (VirtualBox, VMware) using the Vagrant platform and steps outlined below.

### Set Up Vagrant and Create VM Instance

**Note that all example commands are executed on the host machine (your laptop), e.g., in `Terminal.app` (or `iTerm2.app`) on macOS, `cmd` in Windows, and `console` or `xterm` on Linux.  After the last step (`vagrant ssh`) you will get inside the virtual machine and can compile your code there.**

- Download and install your favourite virtualization engine, e.g., [VirtualBox](https://www.virtualbox.org/wiki/Downloads)

- Download and install [Vagrant tools](https://www.vagrantup.com/downloads.html) for your platform

- Set up project and VM instance

  * Clone project template

        git clone https://github.com/aa-fiu-classes/spring20-project1 ~/cnt4713-proj1
        cd ~/cnt4713-proj1

  * Initialize VM

        vagrant up

    Do not start VM instance manually from VirtualBox GUI, otherwise you may have various problems (connection error, connection timeout, missing packages, etc.)
    {: class="alert alert-warning"}

  * To establish an SSH session to the created VM, run

        vagrant ssh

  If you are using Putty on Windows platform, `vagrant ssh` will return information regarding the IP address and the port to connect to your virtual machine.

- Work on your project

  All files in `~/cnt4713-proj1` folder on the host machine will be automatically synchronized with `/vagrant` folder on the virtual machine.  For example, to compile your code, you can run the following commands:

        vagrant ssh
        cd /vagrant
        make

### Notes

* If you want to open another SSH session, just open another terminal and run `vagrant ssh` (or create a new Putty session).

* If you are using Windows, read [this article](http://www.sitepoint.com/getting-started-vagrant-windows/) to help yourself set up the environment.

* You are now free to add more files and modify the Makefile to make the `server.py` and `client.py` full-fledged implementation.

## Submission Requirements

To submit your project:

1. Create a `README.md` file placed in your code that includes:

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

Your code will then be automatically tested in some testing scenarios.

Note that your implementation will be tested against a reference implementation. Projects receive full credit if only all these checks are passed.

### Grading Criteria

1. (2.5 pts) At least 3 git commits
1. (2.5 pts) Client handles incorrect hostname/port
1. (2.5 pts) Server handles incorrect port
1. (2.5 pts) Server handles `SIGTERM` / `SIGQUIT` signals
1. (5 pts) Client connects and starts transmitting a file
1. (5 pts) Server accepts a connection and start saving a file
1. (5 pts) Client able to successfully transmit a small file (500 bytes)
1. (5 pts) Client able to successfully transmit a medium size file (1 MiB)
   * sending in large chunks without delays
1. (5 pts) Client able to successfully transmit a large size file (100 MiB)
   * sending in large chunks without delays
1. (5 pts) Server able to receive a small file (500 bytes) and save it in `1.file`
1. (15 pts) Server able to receive a medium file (1 MiB bytes) and save it in `1.file`
   * (5 pts) receiving file sent in large chunks without delays
   * (10 pts) receiving file sent in small chunks with delays
1. (15 pts) Server able to receive a large file (100 MiB bytes) and save it in `1.file`
   * (5 pts) receiving file sent in large chunks without delays
   * (10 pts) receiving file sent in small chunks with delays
1. (10 pts, 1 public and 1 private test) Server can properly receive 10 small files (sent without delays) in `1.file`, `2.file`, ... `10.file`
   * a single client connects sequentially
   * 10 clients connect simultaneously (our test will ensure proper ordering of connections)
1. (5 pts) Client aborts connection attempt after 10 seconds.
1. (5 pts) Client aborts connection when server gets disconnected (server app or network connection is down)
1. (5 pts) Server aborts connection (a file should be created, containing only `ERROR` string) when it does not receive data from client for more than 10 seconds.
1. (5 pts) Client is able to successfully send and server properly receive and save large file over lossy and large delay network (we will use `tc` based emulation).


### Grading Hint

- If you see 100% load on your CPU when you running your client/server, it is an indication that something is wrong and test script will legitimately (but randomly) fail.


### Deductions

1. (-5 pts) The submission archive contains temporary or other non-source code file, except `README.md`, `Vagrantfile`, files under `.git` subfolder.
