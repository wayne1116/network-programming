## Networking programming homework3

### How to compile

Compiling a pcap program requires linking with the pcap lib.

    sudo apt-get install libpcap-dev

● generate executive file

    make all

### How to read the pcap file

● read the arp pcap file

    make arp
    
● read the http pcap file

    make http
    
● read the v6-http pcap file

    make v6http

● read the dns pcap file

    make dns

● read the ospf pcap file
    
    make ospf

### Reference website

- Using libpcap in C(https://www.devdungeon.com/content/using-libpcap-c)
