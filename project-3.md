---
layout: page
title: "Project 3: Build Your Own Router"
group: "Project 3"
---

**Project specification will be published shortly**

{% asset riddikulus.jpg class="col-md-6 pull-right" style="max-width:100%" %}

* toc
{:toc}

**Revisions**

Not yet

## Overview

In this project, you will be writing a simple router with a static routing table. Your router will receive raw Ethernet frames and process them just like a real router: forward them to the correct outgoing interface, create new frames, etc.  The starter code will provide the framework to receive Ethernet frames; your job is to create the forwarding logic.

All implementations should be written in Python [BSD sockets](http://en.wikipedia.org/wiki/Berkeley_sockets).
**No high-level network-layer abstractions are allowed in this project.**
You are allowed to use some high-level abstractions for parts that are not directly related to networking, such as string parsing, multi-threading.

You are required to use `git` to track the progress of your work. **The project can receive a full grade only if the submission includes git history no shorter than 3 commits FROM ALL PARTICIPANTS OF YOUR GROUP.**  If commit history includes commits made only by one group member, other group members will receive **no credit**.

You are encouraged to host your code in private repositories on [GitHub](https://github.com/), [GitLab](https://gitlab.com), or other places.  At the same time, you are PROHIBITED to make your code for the class project public during the class or any time after the class.  If you do so, you will be violating academic honestly policy that you have signed, as well as the student code of conduct and be subject to serious sanctions.
{: class="alert alert-warning"}

## Task Description

There are four main parts in this assignment:

- Handle Ethernet frames
- Handle ARP packets
- Handle IPv4 packets
- Handle select ICMP packets

These handlings needs to be implemented in the [skeleton code](https://github.com/aa-fiu-classes/fall21-project3) (see environment setup below for further details) and largely follow the logic in the following diagram:

{% asset p3-flowchart.svg width="100%" %}


This assignment runs on top of [Mininet](http://mininet.org/) which was built at Stanford.  Mininet allows you to emulate a network topology on a single machine.  It provides the needed isolation between the emulated nodes so that your router node can process and forward real Ethernet frames between the hosts like a real router.  You don't have to know how Mininet works to complete this assignment, but if you're curious, you can learn more information about Mininet on [its official website](http://mininet.org/).

Your router will route real packets between emulated hosts in a single-router topology.  The project environment and the starter code has the following default topology:

                                                  +----------------+ server1-eth0
                                                  |                  192.168.2.2/24
                                                  +                   +-----------+
                                              192.168.2.1/24          |           |
                                              sw0-eth1                |  server1  |
    +----------+                +------------------+                  |           |
    |          |                |                  |                  +-----------+
    |  client  |                |   SimpleRouter   |
    |          |                |      (sw0)       |
    +----------+                |                  |
       client-eth0              +------------------+                  +-----------+
       10.0.1.100/8           sw0-eth3        sw0-eth2                |           |
          +                 10.0.1.1/8        172.64.3.1/16           |  server2  |
          |                      +                +                   |           |
          |                      |                |                   +-----------+
          |                      |                |                  server2-eth0
          +----------------------+                +----------------+ 172.64.3.10/16

The corresponding routing table for the SimpleRouter `sw0` in this default topology:

    Destination  Gateway      Mask             Iface
    ------------ ------------ ---------------- --------
    10.0.1.0     0.0.0.0      255.255.255.0    sw0-eth3
    192.168.2.0  0.0.0.0      255.255.255.0    sw0-eth1
    172.64.3.0   0.0.0.0      255.255.0.0      sw0-eth2

Do not hardcode any IP addresses, network, or interface information.  We will be testing your code on other single-router topologies with different number of servers and clients, and different IP and network addresses.
{: class="alert alert-info"}

If your router is functioning correctly, all of the following operations should work:

- `ping` from the client to any of the router's interfaces:

    ```bash
    mininet> client ping 192.168.2.1
    ...
    mininet> client ping 172.64.3.1
    ...
    mininet> client ping 10.0.1.1
    ...
    ```

- `ping` from the client to any of the app servers:

    ```bash
    mininet> client ping server1  # or client ping 192.168.2.2
    ...
    mininet> client ping server2  # or client ping 172.64.3.10
    ...
    ```

- `traceroute` from the client to any of the router's interfaces:

    ```bash
    mininet> client traceroute 192.168.2.1
    ...
    mininet> client traceroute 172.64.3.1
    ...
    mininet> client traceroute 10.0.1.1
    ...
    ```

- Tracerouting from the client to any of the app servers:

    ```bash
    mininet> client traceroute server1
    ...
    mininet> client traceroute server2
    ...
    ```

- Transferring file from client to server(s) using the code from your project 1 or 2. Notice that outputs need to be redirected into files; otherwise, the client or server(s) may stop responding.

    ```bash
    mininet> server1 /path/to/your/server.py 5000 /folder/to/save > server1Output &
    ...
    mininet> client /path/to/your/client.py 192.168.2.2 5000 /file/to/transfer > clientOutput &
    ...
    ```

### Ethernet Frames

A data packet on a physical Ethernet link is called an Ethernet packet, which transports an Ethernet frame as its payload.

The starter code will provide you with a  [raw Ethernet frame](https://en.wikipedia.org/wiki/Ethernet_frame).  Your implementation should understand source and destination MAC addresses and properly dispatch the frame based on the protocol.

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |     Destination Address                                       |
    +         (48 bits)             +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                               |          Source Address       |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+             (48 bits)         +
    |                                                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |        Type (16 bits)         |                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+                               +
    |                                                               |
    |                                                               |
    ~                          Payload                              ~
    |                                                               |
    |                                                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

Note that the actual Ethernet frame also includes a 32-bit Cyclical Redundancy Check (CRC). In this project, you will not need it, as it will be added automatically.

- `Type`: Payload type

    * `0x0806` (ARP)
    * `0x0800` (IPv4)

For your convenience, the starter code defines Ethernet header as `EtherHeader` class in `router_base/headers.py`.
The class gives direct access to header fields:

- `dhost` (6 bytes): destination ethernet address
- `shost` (6 bytes): source ethernet address
- `type`  (2 bytes): packet type ID

Given a buffer `buf` that contains a packet with Ethernet header, you can decode it using the following code:

```python
from router_base import headers
pkt = headers.EtherHeader(buf)
```

To create a new Ethernet header:

```python
from router_base import headers
pkt = headers.EtherHeader(shost="f0:18:98:96:E3:18", dhost="f8:e9:4e:74:de:3a", type=2054)
buf = pkt.encode()
```

If you have a buffer `buf` that contains a packet with Ethernet header that you want to replace (or change a field), you can use the following code:

```python
from router_base import headers
pkt = headers.EtherHeader()
offset = pkt.decode(buf)

pkt.dhost = "ff:ff:ff:ff:ff:ff"

new_packet = pkt.encode() + buf[offset:]
```

**Requirements**

- Your router should ignore Ethernet frames other than ARP and IPv4.

- Your router must ignore Ethernet frames not destined to the router, i.e., when destination hardware address is neither the corresponding MAC address of the interface nor a broadcast address (`FF:FF:FF:FF:FF:FF`).

- Your router must appropriately dispatch Ethernet frames (their payload) carrying ARP and IPv4 packets.

### ARP Packets

[The Address Resolution Protocol (ARP)](https://en.wikipedia.org/wiki/Address_Resolution_Protocol) ([RFC826](https://tools.ietf.org/html/rfc826)) is a telecommunication protocol used for resolution of Internet layer addresses (e.g., IPv4) into link layer addresses (e.g., Ethernet).  In particular, before your router can forward an IP packet to the next-hop specified in the routing table, it needs to use ARP request/reply to discover a MAC address of the next-hop.   Similarly, other hosts in the network need to use ARP request/replies in order to send IP packets to your router.

Note that ARP requests are sent to the broadcast MAC address (`FF:FF:FF:FF:FF:FF`). ARP replies are sent directly to the requester's MAC address.

        0                   1                   2                   3
        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |         Hardware Type         |        Protocol Type          |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |  HW addr len  | Prot addr len |           Opcode              |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |                                                               |
        ~                    Source hardware address                    ~
        |                                                               |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |                                                               |
        ~                    Source protocol address                    ~
        |                                                               |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |                                                               |
        ~                  Destination hardware address                 ~
        |                                                               |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |                                                               |
        ~                  Destination protocol address                 ~
        |                                                               |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

* `Hardware Type`: `0x0001` (Ethernet)

* `Protocol Type`: `0x0800` (IPv4)

* `Opcode`:

  * `1` (ARP request)
  * `2` (ARP reply)

* `HW addr len`: number of octets in the specified hardware address. Ethernet has 6-octet addresses, so 0x06.

* `Prot addr len`: number of octets in the requested network address. IPv4 has 4-octet addresses, so 0x04.


For your convenience, the starter code defines ARP header as `ArpHeader` class in `router_base/headers.py`. The class gives direct access to header fields:

- `hrd` (2 bytes): format of hardware address
- `pro` (2 bytes): format of protocol address
- `hln` (1 byte):  length of hardware address
- `pln` (1 byte):  length of protocol address
- `op`  (2 bytes): ARP opcode (command)
- `sha` (6 bytes): sender hardware address
- `sip` (4 bytes): sender IPv4 address
- `tha` (6 bytes): target hardware address
- `tip` (4 bytes): target IP address

Given a buffer `buf` that contains a packet with ARP header, you can decode it using the following code:

```python
from router_base import headers
pkt = headers.ArpHeader(buf)

# if buf contains Ethernet header first, use the proper offset slicing, for example
# pkt = headers.ArpHeader(buf[14:])
```

To create a new ARP header:

```python
from router_base import headers
pkt = headers.ArpHeader(hln=6, pln=4, op=1, sha='f0:18:98:96:e3:18', sip='192.168.100.156', tha='f8:e9:4e:74:de:3a', tip='192.168.100.151')
buf = pkt.encode()
```

If you have a buffer `buf` that contains a packet with ARP header that you want to replace (or change a field), you can use the following code:

```python
from router_base import headers
pkt = headers.ArpHeader()
offset = pkt.decode(buf)

pkt.sip = "1.1.1.1"
pkt.sha = "ff:ff:ff:ff:ff:ff"

new_packet = pkt.encode() + buf[offset:]
```

**Requirements**

- Your router must properly process incoming ARP requests and replies:

    * Must properly respond to ARP requests for MAC address for the IP address of the corresponding network interface
    * Must ignore other ARP requests

- When your router receives an IP packet to be forwarded to a next-hop IP address, it should check ARP cache if it contains the corresponding MAC address:

    * If a valid entry found, the router should proceed with handling the IP packet
    * Otherwise, the router should queue the received packet and start sending ARP request to discover the IP-MAC mapping.

- When router receives an ARP reply, it should record IP-MAC mapping information in ARP cache (Source IP/Source hardware address in the ARP reply).  Afterwards, the router should send out all corresponding enqueued packets.

  <span class="label label-warning">Note</span> Your implementation should not save IP-MAC mapping based on any other messages, only from ARP replies!

- To reduce staleness of the ARP information, entries in ARP cache should time out after `30 seconds`.  The starter code (`ArpCache` class) already includes the facility to mark ARP entries "invalid".  Your task is to remove such entries.  If there is an ongoing traffic (e.g., client still pinging the server), then the router should go through the standard mechanism to send ARP request and then cache the response.  If there is no ongoing traffic, then ARP cache should eventually become empty.

- The router should send an ARP request about once a second until an ARP reply comes back or the request has been sent out at least `5 times`.

  If your router didn't receive ARP reply after re-transmitting an ARP request 5 times, it should stop re-transmitting, remove the pending request, and any packets that are queued for the transmission that are associated with the request.

  Your router should also send an ICMP Destination Unreachable message to the source IP.

### IPv4 Packets

[Internet Protocol version 4 (IPv4)](https://en.wikipedia.org/wiki/IPv4) ([RFC 791](https://tools.ietf.org/html/rfc791)) is the dominant communication protocol for relaying datagrams across network boundaries.  Its routing function enables internetworking, and essentially establishes the Internet.  IP has the task of delivering packets from the source host to the destination host solely based on the IP addresses in the packet headers.  For this purpose, IP defines packet structures that encapsulate the data to be delivered.  It also defines addressing methods that are used to label the datagram with source and destination information.

        0                   1                   2                   3
        0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |Version|  IHL  |Type of Service|          Total Length         |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |         Identification        |Flags|      Fragment Offset    |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |  Time to Live |    Protocol   |         Header Checksum       |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |                       Source Address                          |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |                    Destination Address                        |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |                    Options                    |    Padding    |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |                                                               |
        |                                                               |
        ~                          Payload                              ~
        |                                                               |
        |                                                               |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


For your convenience, the starter code defines ARP header as `IpHeader` class in `router_base/headers.py`. The class gives direct access to header fields:

- `v`  (4 bits):   IPv4 version (4 bits)
- `hl` (4 bits):   Header length (4 bits)
- `tos` (1 byte):  type of service
- `len` (2 bytes): total length
- `id`  (2 bytes): identification
- `off` (2 bytes): fragment offset field and flags
- `ttl` (1 byte):  time to live
- `p`   (1 byte):  protocol
- `sum` (2 bytes): checksum
- `src` (4 bytes): source address
- `dst` (4 bytes): dest address

Given a buffer `buf` that contains a packet with IP header, you can decode it using the following code:

```python
from router_base import headers
pkt = headers.IpHeader(buf)

# if buf contains Ethernet header first, use the proper offset slicing, for example
# pkt = headers.IpHeader(buf[14:])
```

To create a new IP header:

```python
from router_base import headers
pkt = headers.IpHeader(hl=5, tos=0, len=84, id=42095, off=0, ttl=53, p=1, sum=47603, src='1.1.1.1', dst='192.168.100.156')
buf = pkt.encode()
```

If you have a buffer `buf` that contains a packet with IP header that you want to replace (or change a field), you can use the following code:

```python
from router_base import headers
pkt = headers.IpHeader()
offset = pkt.decode(buf)

pkt.src = "1.1.1.1"
pkt.dst = "2.2.2.2"

new_packet = pkt.encode() + buf[offset:]
```

**Requirements**

- For each incoming IPv4 packet, your router should verify its checksum and the minimum length of an IP packet

    * Invalid packets must be discarded (a proper ICMP error response is NOT required for this project).

- Your router should classify datagrams into (1) destined to the router (to one of the IP addresses of the router), and (2) datagrams to be forwarded:

    * For (1), if packet carries ICMP payload, it should be properly dispatched.  Otherwise, discarded (a proper ICMP error response is NOT required for this project).

    * For (2), your router should use the longest prefix match algorithm to find a next-hop IP address in the routing table and attempt to forward it there

- For each forwarded IPv4 packet, your router should correctly decrement TTL and recompute the checksum.

### ICMP Packets

[Internet Control Message Protocol (ICMP)](https://en.wikipedia.org/wiki/Internet_Control_Message_Protocol) ([RFC 792](https://tools.ietf.org/html/rfc792)) is a simple protocol that can send control information to a host.

In this assignment, your router will use ICMP to send messages back to a sending host.

- `Echo` or `Echo Reply` message

         0                   1                   2                   3
         0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |     Type      |    Code = 0   |          Checksum             |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |           Identifier          |        Sequence Number        |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |     Data ...
        +-+-+-+-+-

    * `Type`

      - `8`: echo message
      - `0`: echo reply message

- `Time Exceeded` message

         0                   1                   2                   3
         0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |  Type = 11    |     Code      |          Checksum             |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |                             unused                            |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |      Internet Header + 64 bits of Original Data Datagram      |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    * `Code`

      - `0`: time to live exceeded in transit
      - `1`: fragment reassembly time exceeded (NOT required to implement)

- `Destination Unreachable` message

         0                   1                   2                   3
         0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |  Type = 3     |     Code      |          Checksum             |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |                             unused                            |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        |      Internet Header + 64 bits of Original Data Datagram      |
        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    * `Code`

      - `0`: Destination net Unreachable
      - `1`: Destination host Unreachable
      - `3`: Destination port unreachable


When an ICMP message is composed by a router, the source address field of the internet header can be the IP address of any of the router's interfaces, as specified in [RFC 792](https://tools.ietf.org/html/rfc792).

Note that `Time Exceeded` message is needed for `traceroute` to work properly.

For your convenience, the starter code defines ICMP header for types 0, 3, 4, 8, and 11 and as `IcmpHeader` class in `router_base/headers.py`. The class gives direct access to header fields:

- `type` (1 byte):  ICMP Type
- `code` (2 bytes): ICMP Code
- `sum`  (2 bytes): ICMP Checksum

For types 3, 11, and 4:
- `data` (variable length): original IP header + 8 bytes of Original Data Datagram

For types 0 and 8
- `id` (2 bytes):           If code = 0, an identifier to aid in matching echos and replies, may be zero.
- `seqNum` (2 bytes):       If code = 0, a sequence number to aid in matching echos and replies, may be zero.
- `data` (variable length): Opaque data

Given a buffer `buf` that contains a packet with ICMPs header, you can decode it using the following code:

```python
from router_base import headers
pkt = headers.IcmpHeader(buf)

# if buf contains Ethernet and IP header first, use the proper offset slicing, for example
# pkt = headers.IcmpHeader(buf[14+20:])
#
# However, note that IP header can contain options and it's size not fixed to 20 bytes.
# Therefore, to properly handle offset, you will need to decode IP header first, then obtain header length,
# and calculate the offset.
```

To create a new ICMP header:

```python
from router_base import headers
pkt = headers.IcmpHeader(type=0, code=0, sum=26887, id=50522, seqNum=2, data=b'random-data')
buf = pkt.encode()
```

The starter code has limited support for changing fields in ICMP header.
If you have a buffer `buf` that contains a packet with ICMP header that you want to replace (or change a field), you can use the following code:

```python
from router_base import headers
pkt = headers.IcmpHeader()
offset = pkt.decode(buf)

pkt.type = 0

new_packet = pkt.encode() + buf[offset:]
```

**Requirements**

Your router should properly generate the following ICMP messages, including proper ICMP header checksums:

- `Echo Reply` message (`type 0`):

    Sent in response to an incoming `Echo Request` message (ping) to one of the router's interfaces.
    Echo requests sent to other IP addresses should be forwarded to the next hop address as usual.

- `Time Exceeded` message (`type 11`, `code 0`):

    Sent if an IP packet is discarded during processing because the TTL field is 0. This is needed for traceroute to work.

- `Port Unreachable` message (`type 3`, `code 3`):

    Sent if an IP packet containing a UDP or TCP payload is sent to one of the router's interfaces. This is needed for traceroute to work.

<!-- ///////////// -->
<!-- ///////////// -->
<!-- ///////////// -->

## Environment Setup

### Initial Setup

For this project you should use the provided Vagrant environment, as it installs several critical dependencies and starts a daemon.  You can follow instructions in `Vagrantfile` to recreate environment natively, but do it on your own risk.


1. Clone project template

    ```bash
    git clone https://github.com/aa-fiu-classes/fall21-project3 ~/cnt4713-proj3
    cd ~/cnt4713-proj3
    ```

2. Initialize VM

    ```bash
    vagrant up
    ```

3. To establish an SSH session to the created VM, run

    ```bash
    vagrant ssh
    ```

    In this project you will need to open at least two SSH sessions to the VM: to run Mininet to emulate topology and to run commands on emulated nodes, and to run your router implementation.

    If you would like to span per-emulated node `xterm` terminals from Mininet, you can establish SSH sessions in a slightly different way (probably will not work on Windows):

    ```bash
    vagrant ssh -- -Y
    ```

### Running Your Router

To run your router, you will need to run in parallel two commands: Mininet process that emulates network topology and your router app.  For ease of debugging, can run them in `screen` (or `tmux`) environments or simply in separate SSH sessions:

- To run Mininet network emulation process

    ```bash
    vagrant ssh #  or   vagrant ssh -- -Y

    cd /vagrant
    sudo python ./run.py # must be run as superuser
    ...
    mininet>
    ```

- To run your router

    ```bash
    vagrant ssh

    cd /vagrant
    python3 ridikkulus_router.py
    ```

    <span class="label label-info">Note</span>
    If after start of the router, you see the following message

    ```
    Resetting SimpleRouter with 0 ports
    Interface list empty
    ```

    You should start or restart Mininet process.  The expected initial output should be:

    ```
    Resetting SimpleRouter with 3 ports
    sw0-eth1 (192.168.2.1, f6:fc:48:40:43:af)
    sw0-eth2 (172.64.3.1, 56:be:8e:bd:91:bf)
    sw0-eth3 (10.0.1.1, 22:69:6c:08:25:e9)
    ...
    ```

The VM environment you're using in this project runs a process that redirects packets from the Mininet-emulated switch to your router.  If you want to see debug output of that redirector, you can use `journalctl` command:

```
root@vagrant:/vagrant/# journalctl -f _SYSTEMD_UNIT=pox.service
-- Logs begin at Sat 2017-04-08 20:51:21 UTC. --
Apr 09 19:57:48 vagrant pox.py[8879]: POX 0.5.0 (eel) / Copyright 2011-2014 James McCauley, et al.
Apr 09 19:57:48 vagrant pox.py[8879]: INFO:.usr.local.lib.python2.7.dist-packages.ucla_cs118.pox_rpc_server:Starting packet redirector...
Apr 09 19:57:48 vagrant pox.py[8879]: -- 04/09/17 19:57:48.798 Network: listening for tcp connections at 127.0.0.1:8888
Apr 09 19:57:48 vagrant pox.py[8879]: -- 04/09/17 19:57:48.798 Network: published endpoints for object adapter `SimpleRouter':
Apr 09 19:57:48 vagrant pox.py[8879]:    tcp -h 127.0.0.1 -p 8888
Apr 09 19:57:48 vagrant pox.py[8879]: -- 04/09/17 19:57:48.799 Network: accepting tcp connections at 127.0.0.1:8888
Apr 09 19:57:48 vagrant pox.py[8879]: INFO:core:POX 0.5.0 (eel) is up.
...
```

{% comment %}

## Starter Code Overview

Here is the overal structure of the starter code:

                        simple_router.py
                        +--------------+            core/protocol.py
                        |              |
                        | SimpleRouter |            core/utils.py
                        |              |
                        +---+-----+----+
             m_arp        1 | 1|  | 1       m_ifaces
           +----------------+  |  +------------------+
           |                   |                     |
           |                   |m_routingTable       |
           |                   |                     |
           |1                  |1                    | N (dictionary)
           v                   v                     v
    +------+-----+      +------+-------+       +-----+-----+
    |            |      |              |       |           |
    |  ArpCache  |      | RoutingTable |       | Interface |
    |            |      |              |       |           |
    +------------+      +--------------+       +-----------+
     arp_cache.py      routing_table.py      core/interface.py

- `SimpleRouter`

    Main class for your simple router, encapsulating `ArpCache`, `RoutingTable`, and as set of `Interface` objects.

- `Interface`

    Class containing information about router's interface, including router interface name (`name`), hardware address (`addr`), and IPv4 address (`ip`).

- `RoutingTable` (`routing_table.py`)

    Class implementing a simple routing table for your router.  The content is automatically loaded from a text file with default filename is `RTABLE` (name can be changed using `RoutingTable` option in `router.config` config file)

- `ArpCache` (`arp_cache.py`)

    Class for handling ARP entries and pending ARP requests.

### Key Methods

Your router receives a raw Ethernet frame and sends raw Ethernet frames when sending a reply to the sending host or forwarding the frame to the next hop. The basic functions to handle these functions are:

- <span class="label label-danger">Need to implement</span>
  Method that receives a raw Ethernet frame (`simple-router.hpp|cpp`):

    ```c++
    /**
     * This method is called each time the router receives a packet on
     * the interface.  The packet buffer \p packet and the receiving
     * interface \p inIface are passed in as parameters.
     */
    void
    SimpleRouter::handlePacket(const Buffer& packet, const std::string& inIface);
    ```

- <span class="label label-primary">Implemented</span>
  Method to send raw Ethernet frames (`simple-router.hpp|cpp`):

    ```c++
    /**
     * Call this method to send packet \p packt from the router on interface \p outIface
     */
    void
    SimpleRouter::sendPacket(const Buffer& packet, const std::string& outIface);
    ```

- <span class="label label-danger">Need to implement</span>
  Method to handle ARP cache events (`arp-cache.hpp|cpp`):

    ```c++
    /**
     * This method gets called every second. For each request sent out,
     * you should keep checking whether to resend a request or remove it.
     */
    void
    ArpCache::periodicCheckArpRequestsAndCacheEntries();
    ```

- <span class="label label-danger">Need to implement</span>  Method to lookup entry in the routing table (`routing-table.hpp|cpp`):

    ```c++
    /**
     * This method should lookup a proper entry in the routing table
     * using "longest-prefix match" algorithm
     */
     RoutingTableEntry
     RoutingTable::lookup(uint32_t ip) const;
    ```

### Debugging Functions

We have provided you with some basic debugging functions in `core/utils.hpp` (`core/utils.cpp`). Feel free to use them to print out network header information from your packets. Below are some functions you may find useful:

- `print_hdrs(const uint8_t *buf, uint32_t length)`, `print_hdrs(const Buffer& packet)`

     Print out all possible headers starting from the Ethernet header in the packet

{% endcomment %}

### Logging Packets

You can use Mininet to monitor traffic that goes in
and out of the emulated nodes, i.e., router, server1 and
server2. For example, to see the packets in and out of `server1` node, use the following command in Mininet command-line interface (CLI):

```bash
mininet> server1 sudo tcpdump -n -i server1-eth0
```

Alternatively, you can bring up a terminal inside `server1` using the following command

```bash
mininet> xterm server1
```

then inside the newly opened `xterm`:

```bash
$ sudo tcpdump -n -i server1-eth0
```

### Length of Assignment

**This assignment is considerably hard, so get started early, not as many of you did for project 2.**

In our reference solution, we added less than 520 lines of Python code, including whitespace and comments.
Of course, your solution may need fewer or more lines of code, but this gives you a rough idea of the size of the assignment to a first approximation.

## A Few Hints

Given a raw Ethernet frame, if the frame contains an IP packet that is not destined towards one of our interfaces:

- Sanity-check the packet (meets minimum length and has correct checksum).

- Decrement the TTL by 1, and recompute the packet checksum over the modified header.

- Find out which entry in the routing table has the longest prefix match with the destination IP address.

- Check the ARP cache for the next-hop MAC address corresponding to the next-hop IP. If it's there, send it. Otherwise, send an ARP request for the next-hop IP (if one hasn't been sent within the last second), and add the packet to the queue of packets waiting on this ARP request.

If an incoming IP packet is destined towards one of your router's IP addresses, you should take the following actions, consistent with the section on protocols above:

- If the packet is an ICMP echo request and its checksum is valid, send an ICMP echo reply to the sending host.

- If the packet contains a TCP or UDP payload, send an ICMP port unreachable to the sending host. Otherwise, ignore the packet. Packets destined elsewhere should be forwarded using your normal forwarding logic.

Obviously, this is a very simplified version of the forwarding process, and the low-level details follow. For example, if an error occurs in any of the above steps, you will have to send an ICMP message back to the sender notifying them of an error. You may also get an ARP request or reply, which has to interact with the ARP cache correctly.


## Submission Requirements

To submit your project:

1. Create a `README.md` file placed in your code that includes:

    * Name and UID of each team member (up to 3 members in one team) and the contribution of each member
    * The high level design of your implementation
    * The problems you ran into and how you solved the problems
    * List of any additional libraries used
    * Acknowledgement of any online tutorials or code example (except class website) you have been using.

2. Submit to Gradescope via connection to Github. Do not submit individual files or .zip file, as it will most likely result in grading failures.

Before submission, please make sure the client and server conforms to the specification and you did not implement what implementation does not require.

Submissions that do not follow these requirements will not get any credit.
{: class="bs-callout bs-callout-warning" }

## Grading

### Grading Criteria

Note that test cases and points are subject adjustments.

1. Ping tests (50 pts)

    * 1.1. (5 pts, public) Pings from client to all other hosts (all pings expected to succeed), including non-existing host (error expected)

    * 1.2. (5 pts, public) Pings from server1 to all other hosts (all pings expected to succeed), including non-existing host (error expected)

    * 1.3. (5 pts, public) Pings from server2 to all other hosts (all pings expected to succeed), including non-existing host (error expected)

    * 1.4. (5 pts, public) Ping responses (from client) have proper TTLs

    * 1.5. (5 pts, public) Ping from client to server1, check ARP cache, there should be two entries
 
    * 1.6. (10 pts, private)  Ping from client to server1, after 40 seconds, the ARP cache should be empty (+ no segfaults)

    * 1.7. (10 pts, private)  Ping from client a non-existing IP, router sends proper ARP requests (+ no segfaults)

    * 1.8. (5 pts, private)  Ping from client, receive host unreachable message

2. Traceroute tests (20 pts)

    * 2.1. (5 pts, public) Traceroute from client to all other hosts, including a non-existing host

    * 2.2. (5 pts, private) Traceroute from server1 to all other hosts, including a non-existing host

    * 2.3. (5 pts, private) Traceroute from server2 to all other hosts, including a non-existing host

    * 2.4. (5 pts, private) Traceroute from client to router's interfaces (get 1 line)

3. File transfer tests (20 pts)

    * 3.1. (10 pts, public) Transfer a small file (50k) from server1 to client

    * 3.2. (10 pts, private) transfer a medium file (1M) from server1 to client

    * 3.3. (10 pts, private) transfer a large file (10M) from server1 to client

4. Additional ICMP tests (10 pts)

    * 4.1. (5 pt) ICMP destination unreachable: when no ARP reply after `5 requests` and enqueued packets are dropped.

    * 4.2. (5 pts) ICMP port unreachable when router receives IP/UDP packets sent to one of its interface.

Note that the router should work in other single-router network topologies / with different routing tables

## Acknowledgement

This project is based on the [CS144 class project](http://web.stanford.edu/class/cs144/) by Professor Philip Levis and Professor Nick McKeown, Stanford University.
