---
layout: page
title: "Project 1: \"Accio\" File using TCP"
group: "Project 1"
---

{% asset accio.png class="col-md-6 pull-right" style="max-width:100%" %}

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

The project is split into three progressive parts with their individual deadlines:

- [Part 1: Accio Client](project-1-accio-client.html)
- [Part 2: Accio Server Simplified](project-1-accio-server-simplified.html)
- [Part 3: Accio Server](project-1-accio-server.html)

## Environment Setup

To successfully finish the project you can use any platform that can run Python3.  However, as a general practice and ability to test your implementation under emulated network conditions, it is highly recommended that you use the provided Ubuntu 18.04 based environment.

You can easily create an image in your favourite virtualization engine (e.g., VirtualBox) using the Vagrant platform and steps outlined below.

### Set Up Vagrant and Create VM Instance

**Note that all example commands are executed on the host machine (your laptop), e.g., in `Terminal.app` (or `iTerm2.app`) on macOS, `cmd` in Windows, and `console` or `xterm` on Linux.  After the last step (`vagrant ssh`) you will get inside the virtual machine and can compile your code there.**

- Download and install your favourite virtualization engine, e.g., [VirtualBox](https://www.virtualbox.org/wiki/Downloads)

- Download and install [Vagrant tools](https://www.vagrantup.com/downloads.html) for your platform

- Set up project and VM instance

  * Clone project template

        git clone https://github.com/aa-fiu-classes/fall20-project1 ~/cnt4713-proj1
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

* You are now free to add more files to make the `server.py` and `client.py` full-fledged implementation.

## General Submission Requirements

Submissions for three parts of the project must be cumulative, e.g., part 2 should include everything from part 1 and part 3 should include everything from parts 1 and 2.
{: class="bs-callout bs-callout-warning" }

To submit your project:

1. Create a `README.md` file placed in your code that includes:

    * The problems you ran into and how you solved the problems (this can be cumulative across the submission, i.e., you add more as you proceed with parts)
    * Acknowledgement of any online tutorials or code example (except class website) you have been using.

2. Submit to Gradescope via connection to Github. Your repository should include all your source code (`client.py`, `server-s.py`, `server.py`, and any other Python files you have created) and `README.md`. Make sure you do NOT include temporary files (deductions will be made if so).

Before submission, please make sure the client and server conforms to the specification and you did not implement what implementation does not require.

Submissions that do not follow these requirements will not get any credit.
{: class="bs-callout bs-callout-warning" }

## General Grading Policy

Your code will be first checked by a software plagiarism detecting tool.
If we find any plagiarism, you will not get any credit.

Your code will then be automatically tested in some testing scenarios.

Note that your implementation will be tested against a reference implementation. Projects receive full credit if only all these checks are passed.

## Late Submission Policy

Part 1 and Part 2 can be submitted up until deadline for the Part 3.
However, if Part 1 and/or Part 2 are submitted after Part 1 and Part 2 deadlines, the corresponding grade is subject to **20% penalty**.

## Overall Grading Hint

- If you see 100% load on your CPU when you running your client/server, it is an indication that something is wrong and test script will legitimately (but randomly) fail.

## Deductions

- (-5 pts) The submission contains temporary or other non-source code file, except `README.md`, `Vagrantfile`, `.gitignore` and other necessary files.

- (-5 pts) The submission does not contain README.md with the required content.
