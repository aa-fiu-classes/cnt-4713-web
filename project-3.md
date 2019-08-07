---
layout: page
title: "Project 3: Build Your Own Router"
group: "Project 3"
---

<div class="row" markdown="1">
<div class="col-md-12" markdown="1">
<img src="{{ site.baseurl }}/images/covfefe.jpg" class="col-md-6 pull-right" style="max-width: 286px" />

* toc
{:toc}

## Overview

In this project, you will be writing a simple router with a static routing table. Your router will receive raw Ethernet frames and process them just like a real router: forward them to the correct outgoing interface, create new frames, etc.  The starter code will provide the framework to receive Ethernet frames; your job is to create the forwarding logic.

You are allowed to use some high-level abstractions, including C++11 extensions, for parts that are not directly related to networking, such as string parsing, multi-threading, etc.

You are required to use `git` to track the progress of your work. **The project can receive a full grade only if the submission includes git history no shorter than 3 commits FROM ALL PARTICIPANTS OF YOUR GROUP.**  If commit history includes commits made only by one group member, other group members will receive **no credit**.

You are encouraged to host your code in private repositories on [GitHub](https://github.com/), [GitLab](https://gitlab.com), or other places.  At the same time, you are PROHIBITED to make your code for the class project public during the class or any time after the class.  If you do so, you will be violating academic honestly policy that you have signed, as well as the student code of conduct and be subject to serious sanctions.
{: class="alert alert-danger"}

</div>
</div>

## Task Description

There are four main parts in this assignment:

- Handle Ethernet frames
- Handle ARP packets
- Handle IPv4 packets
- Handle ICMP packets

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
    0.0.0.0      10.0.1.100   0.0.0.0          sw0-eth3
    192.168.2.2  192.168.2.2  255.255.255.0    sw0-eth1
    172.64.3.10  172.64.3.10  255.255.0.0      sw0-eth2

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
    mininet> server1 /path/to/your/server 5000 /folder/to/save > server1Output &
    ...
    mininet> client /path/to/your/client 192.168.2.2 5000 /file/to/transfer > clientOutput &
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

Note that actual Ethernet frame also includes a 32-bit Cyclical Redundancy Check (CRC). In this project, you will not need it, as it will be added automatically.

- `Type`: Payload type

    * `0x0806` (ARP)
    * `0x0800` (IPv4)

For your convenience, the starter code defines Ethernet header as an `ethernet_hdr` structure in `core/protocol.hpp`:

```c++
struct ethernet_hdr
{
  uint8_t  ether_dhost[ETHER_ADDR_LEN]; /* destination ethernet address */
  uint8_t  ether_shost[ETHER_ADDR_LEN]; /* source ethernet address */
  uint16_t ether_type;                  /* packet type ID */
} __attribute__ ((packed)) ;
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

For your convenience, the starter code defines the ARP header as an `arp_hdr` structure in `core/protocol.hpp`:

```c++
struct arp_hdr
{
  unsigned short  arp_hrd;                 /* format of hardware address   */
  unsigned short  arp_pro;                 /* format of protocol address   */
  unsigned char   arp_hln;                 /* length of hardware address   */
  unsigned char   arp_pln;                 /* length of protocol address   */
  unsigned short  arp_op;                  /* ARP opcode (command)         */
  unsigned char   arp_sha[ETHER_ADDR_LEN]; /* sender hardware address      */
  uint32_t        arp_sip;                 /* sender IP address            */
  unsigned char   arp_tha[ETHER_ADDR_LEN]; /* target hardware address      */
  uint32_t        arp_tip;                 /* target IP address            */
} __attribute__ ((packed)) ;
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

  <del>Your implementation can also record mapping from ARP requests using source IP and hardware address, but it is not required in this project.</del>

- To reduce staleness of the ARP information, entries in ARP cache should time out after `30 seconds`.  The starter code (`ArpCache` class) already includes the facility to mark ARP entries "invalid".  Your task is to remove such entries.  If there is an ongoing traffic (e.g., client still pinging the server), then the router should go through the standard mechanism to send ARP request and then cache the response.  If there is no ongoing traffic, then ARP cache should eventually become empty.

- The router should send an ARP request about once a second until an ARP reply comes back or the request has been sent out at least `5 times`.

  If your router didn't receive ARP reply after re-transmitting an ARP request 5 times, it should stop re-transmitting, remove the pending request, and any packets that are queued for the transmission that are associated with the request.

  <span class="label label-info">Extra Credit</span>
  Your router can also send an ICMP Destination Unreachable message to the source IP.

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


For your convenience, the starter code defines the IPv4 header as an `ip_hdr` structure in `core/protocol.hpp`:

```c++
struct ip_hdr
{
  unsigned int ip_hl:4;            /* header length */
  unsigned int ip_v:4;             /* version */
  uint8_t ip_tos;                  /* type of service */
  uint16_t ip_len;                 /* total length */
  uint16_t ip_id;                  /* identification */
  uint16_t ip_off;                 /* fragment offset field */
  uint8_t ip_ttl;                  /* time to live */
  uint8_t ip_p;                    /* protocol */
  uint16_t ip_sum;                 /* checksum */
  uint32_t ip_src, ip_dst;         /* source and dest address */
} __attribute__ ((packed));
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

      - `3`: Destination port unreachable


When an ICMP message is composed by a router, the source address field of the internet header can be the IP address of any of the router's interfaces, as specified in [RFC 792](https://tools.ietf.org/html/rfc792).

Note that `Time Exceeded` message is needed for `traceroute` to work properly.

For your convenience, the starter code defines the ICMP header as an `icmp_hdr` structure in `core/protocol.hpp`:

```c++
struct icmp_hdr {
  uint8_t icmp_type;
  uint8_t icmp_code;
  uint16_t icmp_sum;
} __attribute__ ((packed));
```

You may want to create additional structs for ICMP messages, but make sure to use the packed attribute so that the compiler doesn't try to align the fields in the struct to word boundaries (i.e., must use `__attribute__ ((packed))` annotation).

**Requirements**

Your router should properly generate the following ICMP messages, including proper ICMP header checksums:

- `Echo Reply` message (`type 0`):

    Sent in response to an incoming `Echo Request` message (ping) to one of the router's interfaces.
    Echo requests sent to other IP addresses should be forwarded to the next hop address as usual.

- `Time Exceeded` message (`type 11`, `code 0`):

    Sent if an IP packet is discarded during processing because the TTL field is 0. This is needed for traceroute to work.

- <span class="label label-info">Extra Credit</span>
  `Port Unreachable` message (`type 3`, `code 3`):

    Sent if an IP packet containing a UDP or TCP payload is sent to one of the router's interfaces. This is needed for traceroute to work.

<!-- ///////////// -->
<!-- ///////////// -->
<!-- ///////////// -->

## Environment Setup

### Initial Setup

For this project you should use the provided Vagrant environment, as it installs several critical dependencies and starts a daemon.  You can follow instructions in `Vagrantfile` to recreate environment natively, but do it on your own risk.


1. Clone project template

    ```bash
    git clone https://github.com/cs118/spring17-project3 ~/cs118-proj3
    cd ~/cs118-proj3
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
    sudo ./run.py # must be run as superuser
    ...
    mininet>
    ```

- To run your router

    ```bash
    vagrant ssh

    cd /vagrant
    #
    # implement router logic // see below
    #
    make
    ./router
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

## Starter Code Overview

Here is the overal structure of the starter code:

                        simple-router.hpp
                        +--------------+            core/protocol.hpp
                        |              |
                        | SimpleRouter |            core/utils.hpp
                        |              |
                        +---+-----+----+
             m_arp        1 | 1|  | 1       m_ifaces
           +----------------+  |  +------------------+
           |                   |                     |
           |                   |m_routingTable       |
           |                   |                     |
           |1                  |1                    | N (std::set)
           v                   v                     v
    +------+-----+      +------+-------+       +-----+-----+
    |            |      |              |       |           |
    |  ArpCache  |      | RoutingTable |       | Interface |
    |            |      |              |       |           |
    +------------+      +--------------+       +-----------+
     arp-cache.hpp      routing-table.hpp    core/interface.hpp

- `SimpleRouter`

    Main class for your simple router, encapsulating `ArpCache`, `RoutingTable`, and as set of `Interface` objects.

- `Interface`

    Class containing information about router's interface, including router interface name (`name`), hardware address (`addr`), and IPv4 address (`ip`).

- `RoutingTable` (`routing-table.hpp|cpp`)

    Class implementing a simple routing table for your router.  The content is automatically loaded from a text file with default filename is `RTABLE` (name can be changed using `RoutingTable` option in `router.config` config file)

- `ArpCache` (`arp-cache.hpp|cpp`)

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

- `ipToString(uint32_t ip)`, `ipToString(const in_addr& address)`

    Print out a formatted IP address from a `uint32_t` or `in_addr`. Make sure you are passing the IP address in the correct byte ordering

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

In our reference solution, we added less than 520 lines of C/C++ code, including whitespace and comments.
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

To submit your project, you need to prepare:

1. A `README.md` file placed in your code that includes:

    * Name and UID of each team member (up to 3 members in one team) and the contribution of each member
    * The high level design of your implementation
    * The problems you ran into and how you solved the problems
    * List of any additional libraries used
    * Acknowledgement of any online tutorials or code example (except class website) you have been using.

1. All your source code, `Makefile`, `README.md`, `Vagrantfile`, `confundo.lua`, and `.git` folder with your git repository history as a `.tar.gz` archive (and any files from extra credit part).

    To create the submission, **use the provided Makefile** in the starter code.  Just update `Makefile` to include your UCLA ID and then just type

        make tarball

    Then submit the resulting archive to Gradescope.

Before submission, please make sure:

1. Your code compiles
2. Your implementation conforms to the specification
3. `.tar.gz` archive does not contain temporary or other unnecessary files.  We will automatically deduct points otherwise.

Submissions that do not follow these requirements will not get any credit.
{: class="bs-callout bs-callout-danger" }

## Grading

### Grading Criteria

1. Miscellaneous tests

    * 1.1. (5 pts, public) At least 3 git commits (at least one from each group member)

2. Ping tests

    * 2.1. (5 pts, public) Pings from client to all other hosts (all pings expected to succeed), including non-existing host (error expected)

    * 2.2. (5 pts, public) Pings from server1 to all other hosts (all pings expected to succeed), including non-existing host (error expected)

    * 2.3. (5 pts, public) Pings from server2 to all other hosts (all pings expected to succeed), including non-existing host (error expected)

    * 2.4. (5 pts, public) Ping responses (from client) have proper TTLs

    * 2.5. (5 pts, public)  Ping from client to server1, check ARP cache, there should be two entries
 
    * 2.6. (10, private)  Ping from client to server1, after 40 seconds, the ARP cache should be empty (+ no segfaults)

    * 2.7. (10, private)  Ping from client a non-existing IP, router sends proper ARP requests (+ no segfaults)

    * 2.8. (Extra 1 pts, private)  Ping from client, receive host unreachable message

3. Traceroute tests

    * 3.1. (10 pts, public) Traceroute from client to all other hosts, including a non-existing host

    * 3.2. (10 pts, private) Traceroute from server1 to all other hosts, including a non-existing host

    * 3.3. (10 pts, private) Traceroute from server2 to all other hosts, including a non-existing host

    * 3.4. (Extra: 1 pts, private) Traceroute from client to router's interfaces (get 1 line)

4. File transfer tests

    * 4.1. (10 pts, public) Transfer a small file (50k) from server1 to client

    * 4.2. (5 pts, private) transfer a medium file (1M) from server1 to client

    * 4.3. (5 pts, private) transfer a large file (10M) from server1 to client

Note that the router should work in other single-router network topologies / with different routing tables

### Deductions

(-5 pts) The submission archive contains temporary or other non-source code file, except `README.md`, `Vagrantfile`, files under `.git` subfolder (and any files from extra credit part).

### Extra Credit (included in the above)

- (1 pt) ICMP destination unreachable

    * When no ARP reply after `5 requests` and enqueued packets are dropped.

- (1 pt) ICMP port unreachable

    * The router must handle TCP/UDP packets sent to one of its interfaces. In this case the router should respond with an ICMP port unreachable message.

## Acknowledgement

This project is based on the [CS144 class project](http://web.stanford.edu/class/cs144/) by Professor Philip Levis and Professor Nick McKeown, Stanford University.
