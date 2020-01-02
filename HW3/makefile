all: hw3.c
	gcc hw3.c -lpcap -o hw3
arp:  
	./hw3 -r arp-storm.pcap
http:
	./hw3 -r http.cap
v6http:
	./hw3 -r v6-http.cap
dns:
	./hw3 -r dns.cap
ospf:
	./hw3 -r ospf.cap
clean:
	rm -rf hw3
