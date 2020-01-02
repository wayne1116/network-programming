#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pcap.h>
#include<time.h>
#include<netinet/in.h>
#include<netinet/ip.h>
#include<netinet/tcp.h>
#include<netinet/udp.h>
#include<arpa/inet.h>
#include<netinet/if_ether.h>
#ifndef IPPROTO_OSPF
#define IPPROTO_OSPF 89
#endif
int number;
int ipv4Count;
int ipv6Count;
int arpCount;
int totalpacketCount;
// ipv6 header info(40 bytes)
struct ipv6{
	uint32_t vtcfl;         // version, traffic class, flow label
	uint16_t length;
	uint8_t next_header;
	uint8_t hop_limit;
	struct in6_addr ip_src;
	struct in6_addr ip_dst;
};
// ARP header info(28 bytes)
struct arp{
	uint16_t hardware_type;
	uint16_t protocol_type;
	uint8_t hardware_len;
	uint8_t pro_len;
	uint16_t opcode;
	u_char hard_addr_send[6];
	u_char proto_addr_send[4];
	u_char hard_addr_dest[6];
	u_char proto_addr_dest[4];
};
// OSPF header info(24 bytes)
struct ospf{
	uint8_t version;
	uint8_t type;
	uint16_t message_length;
	u_char routerid[4];
	u_char areaid[4];
	uint16_t checksum;
	uint16_t authtype;
	uint64_t authen;
};
// ICMPv6 header info
struct icmpv6{
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
};

struct ipv4_l{
	char source[INET_ADDRSTRLEN];
	char dest[INET_ADDRSTRLEN];
	int connect;
}ipv4_lnode[1024];
struct ipv6_l{
	char source[INET6_ADDRSTRLEN];
	char dest[INET6_ADDRSTRLEN];
	int connect;
}ipv6_lnode[1024];

void my_packet_handler(u_char *args, const struct pcap_pkthdr *packet_header, const u_char *packet);
char *get_protocol(int pro);

int main(int argc, char *argv[])
{
	char error_buffer[PCAP_ERRBUF_SIZE];
	pcap_t *handle;
	int i;
	
	if(argc!=3){
		fprintf(stderr, "Command is wrong.\n");
		exit(EXIT_FAILURE);
	}

	handle=pcap_open_offline(argv[2], error_buffer);
	if(handle==NULL){
		fprintf(stderr, "Error: %s.\n", error_buffer);
		exit(EXIT_FAILURE);
	}
	number=0;
	ipv4Count=0;
	ipv6Count=0;
	arpCount=0;
	totalpacketCount=0;
	pcap_loop(handle, -1, my_packet_handler, NULL);
	pcap_close(handle);

	printf("\n---------- Result ----------\n");
	printf("Total packet number: %d\n", totalpacketCount);
	if(ipv6Count>0){
		for(i=0; i<ipv6Count; i++) 
			printf("packet: (%s,%s) -> %d\n", ipv6_lnode[i].source, ipv6_lnode[i].dest, ipv6_lnode[i].connect);
	}
	if(ipv4Count>0){
		for(i=0; i<ipv4Count; i++)
			printf("packet: (%s,%s) -> %d\n", ipv4_lnode[i].source, ipv4_lnode[i].dest, ipv4_lnode[i].connect);
	}
	if(arpCount>0) printf("ARP packet -> %d\n", arpCount);	
	printf("\n");
	return 0;
}

void my_packet_handler(u_char *args, const struct pcap_pkthdr *packet_header, const u_char *packet)
{
	int i;
	struct tm *tminfo;
	time_t t=packet_header->ts.tv_sec;
	struct ether_header *eth_header;
	u_char *source_mac_addr;
	u_char *destination_mac_addr;
	const int ethernet_header_len=14;
	const u_char *ip_header;
	const struct ip *ipv4_header;
	const struct ipv6 *ipv6_header;
	char ipv4_source_addr[INET_ADDRSTRLEN];
	char ipv4_dest_addr[INET_ADDRSTRLEN];
	char ipv6_source_addr[INET6_ADDRSTRLEN];
	char ipv6_dest_addr[INET6_ADDRSTRLEN];
	int ipv4_header_len;
	const struct tcphdr *tcp_header;
	const struct udphdr *udp_header;
	int source_port;
	int dest_port;

	const struct arp *arp_header;
	const struct ospf *ospf_header;
       	char routerid[INET_ADDRSTRLEN];
	char areaid[INET_ADDRSTRLEN];
	const struct icmpv6 *icmpv6_header;	

	printf("\n------------ The packet %d ------------\n", ++number);
	// reveal the timestamp of the packet
	tminfo=localtime(&t);
	printf("2. packet timestamp : %s", asctime(tminfo));

	// get the source Mac address and destination Mac address
	eth_header=(struct ether_header*)packet;
	source_mac_addr=eth_header->ether_shost;
	i=ETHER_ADDR_LEN;
	printf("   Source address: ");
	do{
		printf("%s%X", (i==ETHER_ADDR_LEN)?" ":":", *source_mac_addr++);
	}while(--i);

	destination_mac_addr=eth_header->ether_dhost;
	i=ETHER_ADDR_LEN;
	printf("\tDestination address: ");
	do{
		printf("%s%X", (i==ETHER_ADDR_LEN)?" ":":", *destination_mac_addr++);
	}while(--i);
	printf("\n");

	// if the packet is IPv4
	ip_header=packet+ethernet_header_len;
	if(ntohs(eth_header->ether_type)==ETHERTYPE_IP){          //0x0800
		ipv4_header=(struct ip*)ip_header;
		inet_ntop(AF_INET, &(ipv4_header->ip_src), ipv4_source_addr, INET_ADDRSTRLEN);
		inet_ntop(AF_INET, &(ipv4_header->ip_dst), ipv4_dest_addr, INET_ADDRSTRLEN);	
		printf("3. IPv4 source address: %s\tdestination address: %s\n", ipv4_source_addr, ipv4_dest_addr);
		
		for(i=0; i<ipv4Count; i++){
			if(strcmp(ipv4_lnode[i].source, ipv4_source_addr)==0 && strcmp(ipv4_lnode[i].dest, ipv4_dest_addr)==0){
				++ipv4_lnode[i].connect;
				break;
			}
		}
		if(i==ipv4Count){
			strcpy(ipv4_lnode[ipv4Count].source, ipv4_source_addr);
			strcpy(ipv4_lnode[ipv4Count].dest, ipv4_dest_addr);
			ipv4_lnode[ipv4Count].connect=1;
			++ipv4Count;
		}

		ipv4_header_len=(int)ipv4_header->ip_hl*4;
		printf("4. Protocol: %s\n", get_protocol(ipv4_header->ip_p));
		// tcp
		if(ipv4_header->ip_p==IPPROTO_TCP){
			tcp_header=(struct tcphdr*)(ip_header+ipv4_header_len);
			source_port=ntohs(tcp_header->source);
			dest_port=ntohs(tcp_header->dest);
			printf("   Source port: %d\tDestination port: %d\n", source_port, dest_port);
		}
		// udp
		else if(ipv4_header->ip_p==IPPROTO_UDP){
			udp_header=(struct udphdr*)(ip_header+ipv4_header_len);
			source_port=ntohs(udp_header->source);
			dest_port=ntohs(udp_header->dest);
			printf("   Source port: %d\tDestination port: %d\n", source_port, dest_port);
		}
		// ospf
		else if(ipv4_header->ip_p==IPPROTO_OSPF){
			ospf_header=(struct ospf*)(ip_header+ipv4_header_len);
			printf("+--OSPF packet info--+\n");
			printf("Version : %d\n", ospf_header->version);
			printf("request/reply code: ");
			switch(ospf_header->type){
				case 1: printf("1(Hello)\n");
					break;
				case 2: printf("2(Database description)\n");
					break;
				case 3: printf("3(Link state request)\n");
					break;
				case 4: printf("4(Link state update)\n");
					break;
				case 5: printf("5(Link state acknowledgement)\n");
					break;
				default: fprintf(stderr, "wrong!!");
			}
			inet_ntop(AF_INET, &ospf_header->routerid, routerid, sizeof(routerid));
			inet_ntop(AF_INET, &ospf_header->areaid, areaid, sizeof(areaid));
			printf("Router ID: %s\n", routerid);
			printf("Area ID: %s\n", areaid);
		}
		else printf("Other type(ipv4)\n");
	}
	// if the packet is IPv6
	else if(ntohs(eth_header->ether_type)==ETHERTYPE_IPV6){   //0x86DD
		ipv6_header=(struct ipv6*)ip_header;              // "::" stands for a string of 0 bits	
		inet_ntop(AF_INET6, &ipv6_header->ip_src, ipv6_source_addr, sizeof(ipv6_source_addr));
		inet_ntop(AF_INET6, &ipv6_header->ip_dst, ipv6_dest_addr, sizeof(ipv6_dest_addr));
		printf("3. IPv6 source address: %s\tdestination address: %s\n", ipv6_source_addr, ipv6_dest_addr);

		for(i=0; i<ipv6Count; i++){
			if(strcmp(ipv6_lnode[i].source, ipv6_source_addr)==0 && strcmp(ipv6_lnode[i].dest, ipv6_dest_addr)==0){
				++ipv6_lnode[i].connect;
				break;
			}
		}
		if(i==ipv6Count){
			strcpy(ipv6_lnode[ipv6Count].source, ipv6_source_addr);
			strcpy(ipv6_lnode[ipv6Count].dest, ipv6_dest_addr);
			ipv6_lnode[ipv6Count].connect=1;
			++ipv6Count;
		}

		printf("4. Protocol: %s\n", get_protocol(ipv6_header->next_header));
		// tcp
		if(ipv6_header->next_header==IPPROTO_TCP){
			tcp_header=(struct tcphdr*)(ip_header+40);
			source_port=ntohs(tcp_header->source);
			dest_port=ntohs(tcp_header->dest);
			printf("   Source port: %d\tDestination port: %d\n", source_port, dest_port);
		}
		// udp
		else if(ipv6_header->next_header==IPPROTO_UDP){
			udp_header=(struct udphdr*)(ip_header+40);
			source_port=ntohs(udp_header->source);
			dest_port=ntohs(udp_header->dest);
			printf("   Source port: %d\tDestination port: %d\n", source_port, dest_port);
		}
		// icmpv6
		else if(ipv6_header->next_header==IPPROTO_ICMPV6){
			icmpv6_header=(struct icmpv6*)(ip_header+40);
			printf("   Type: %d\n", icmpv6_header->type);
		}
		else printf("Other type(ipv6)\n");
	}
	// if the packet is ARP
	else if(ntohs(eth_header->ether_type)==ETHERTYPE_ARP){
		arp_header=(struct arp*)(ip_header);	
		printf("ARP header info\n");
		printf("hardware type: %d\n", ntohs(arp_header->hardware_type));
		printf("protocol type: %d\n", ntohs(arp_header->protocol_type));
		printf("operation type: %d\n", ntohs(arp_header->opcode));
		printf("+--Sender--+\n");
		printf("Hardware address: ");
		for(i=0; i<ETHER_ADDR_LEN-1; i++){
			printf("%02X:", arp_header->hard_addr_send[i]);
		}
		printf("%02X\n", arp_header->hard_addr_send[i]);
		
		inet_ntop(AF_INET, &(arp_header->proto_addr_send), ipv4_source_addr, INET_ADDRSTRLEN);
		printf("protocol address: %s\n", ipv4_source_addr);
		
		printf("+--Receiver--+\n");
		printf("Hardware address: ");
		for(i=0; i<ETHER_ADDR_LEN-1; i++){
			printf("%02X:", arp_header->hard_addr_dest[i]);
		}
		printf("%02X\n", arp_header->hard_addr_dest[i]);

		inet_ntop(AF_INET, &(arp_header->proto_addr_dest), ipv4_dest_addr, INET_ADDRSTRLEN);
		printf("protocol address: %s\n", ipv4_dest_addr);
		++arpCount;
	}
	else printf("Other type\n");
	++totalpacketCount;
}

char *get_protocol(int pro)
{
	switch(pro){
		case 0: return "IP(dummy for IP)";
		case 1: return "ICMP";
		case 6: return "TCP";
		case 17: return "UDP";
		case 58: return "ICMPv6";
		case 89: return "OSPF";
		case 255: return "Raw IP packet";
	}
	return "Unknow";
}
