## Networking programming homework3

### Implementation

- For each packet read in, the timestamp, source MAC address, destination MAC address are displayed.

- If the packet is an IP packet, the source IP address and destination IP address are displayed.

- If the packet is a TCP/UDP packet, the ports which TCP/UDP is using are displayed.(also display the TCP/UDP which is used.)

- Can count the number of packets for each pair (source IP, destination IP) (if it is not an IP packet, skip it), and display it.

### How to compile

● Compiling a pcap program requires linking with the pcap lib.

    sudo apt-get install libpcap-dev

● Generate executive file

    make all

### How to read the pcap file

● Read the arp pcap file

    make arp
    
● Read the http pcap file

    make http
    
● Read the v6-http pcap file

    make v6http

● Read the dns pcap file

    make dns

● Read the ospf pcap file
    
    make ospf

### Reference website

- Using libpcap in C(https://www.devdungeon.com/content/using-libpcap-c)
