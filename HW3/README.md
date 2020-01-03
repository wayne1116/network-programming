## Networking programming homework3

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
