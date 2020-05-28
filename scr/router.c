#include "skel.h"
#define SIZE 1800
struct route_table_entry {
	uint32_t prefix;
	uint32_t next_hop;
	uint32_t mask;
	int interface;
}rtable;

struct arp_entry {
	__u32 ip;
	uint8_t mac[6];
}arp;

struct route_table_entry *in;  // tabela de rutare 
int rtable_size;

struct arp_entry *arp_table;  // tabela arp 
int arp_table_size;

//cautarea binara a celei mai bune rute pe tabela de rutare 
int binarySearch(int left,int right, __u32 dest_ip) {
  int m;
  if (left<=right) {
    m=(left+right)/2;
    if ((in[m].mask & dest_ip) == in[m].prefix){
    	while((in[m].mask & dest_ip) == in[m].prefix)
    		m++;
      return m-1;
    }
    else if ((in[m].mask & dest_ip) < in[m].prefix)
      return binarySearch(left,m-1, dest_ip);
    else
      return binarySearch(m+1,right, dest_ip);
   }
   return -1;
}
//cautarea unei rute arp dupa ip
struct arp_entry *get_arp_entry(__u32 ip) {
	int index = -1;
    for (int i = 0; i < arp_table_size; i++) {
    	if (ip == arp_table[i].ip) {
    		index = i;
    	}
    }
    if (index != -1)
    	return &arp_table[index];
    return NULL;
}
//citirea din fisierul arp_table.txt si completarea tabelei de arp
void read_arp(){
	int i = 0; 
	char ip[SIZE];
	char mac[SIZE];
	int alloc = 10;
	arp_table = (struct arp_entry *)malloc(alloc * sizeof(arp));
	FILE * file = fopen("arp_table.txt" , "r");

	if (file) {
	    while (fscanf(file, "%s %s", ip, mac) != EOF) {
	        inet_pton(AF_INET, ip, &arp_table[i].ip);
	        hwaddr_aton(mac, arp_table[i].mac);
	        i++;
	        if(i > alloc - 1) {
	        	alloc += 10;
	        	arp_table = realloc(arp_table, alloc * sizeof(arp));
	        }
	    }
	    arp_table_size = i;
	    fclose(file);

	}else printf("Eroare file\n");
}
//comparator pentru qsort a tabelei de rutare
int comparator(const void *p1, const void *p2) {
    const struct route_table_entry *elem1 = p1;    
    const struct route_table_entry *elem2 = p2;

   if (elem1->prefix < elem2->prefix)
      return -1;
   else if (elem1->prefix > elem2->prefix)
      return 1;
   else if (elem1->mask < elem2->mask)
      		return -1;
      	else return 1;
}

//calcularea sumei de control
uint16_t ip_checksum(void* vdata,size_t length) {

	char* data=(char*)vdata;

	uint64_t acc=0xffff;

	unsigned int offset=((uintptr_t)data)&3;
	if (offset) {
		size_t count=4-offset;
		if (count>length) count=length;
		uint32_t word=0;
		memcpy(offset+(char*)&word,data,count);
		acc+=ntohl(word);
		data+=count;
		length-=count;
	}


	char* data_end=data+(length&~3);
	while (data!=data_end) {
		uint32_t word;
		memcpy(&word,data,4);
		acc+=ntohl(word);
		data+=4;
	}
	length&=3;


	if (length) {
		uint32_t word=0;
		memcpy(&word,data,length);
		acc+=ntohl(word);
	}


	acc=(acc&0xffffffff)+(acc>>32);
	while (acc>>16) {
		acc=(acc&0xffff)+(acc>>16);
	}


	if (offset&1) {
		acc=((acc&0xff00)>>8)|((acc&0x00ff)<<8);
	}

	return htons(~acc);
}

//citirea din fisierul rtable.txt si completarea tabelei de rutare
void read_rtable(){
	int i = 0; 
	char prefix[SIZE];
	char next_hop[SIZE];
	char mask[SIZE];
	int alloc = 10;
	in = (struct route_table_entry *)malloc(alloc * sizeof(rtable));
	FILE * file = fopen("rtable.txt" , "r");

	if (file) {
	    while (fscanf(file, "%s %s %s %d", prefix, next_hop, mask, &in[i].interface) != EOF) {
	        inet_pton(AF_INET, prefix, &in[i].prefix);
	        inet_pton(AF_INET, next_hop, &in[i].next_hop);
	        inet_pton(AF_INET, mask, &in[i].mask);
	        i++;
	        if(i > alloc - 1) {
	        	alloc += 10;
	        	in = realloc(in, alloc * sizeof(rtable));
	        }
	    }
	    rtable_size = i;
	    fclose(file);
	} else printf("Eroare file\n");
}

//Inițializați headerul de ICMP si IPV4 cu informațiile necesare.
void setIphdr_Icmphdr(struct iphdr *ip_hdr, struct icmphdr *icmp, struct ether_header *eth_hdr, packet m, int type) {
		    u_char temp[6];
	        memcpy(temp ,eth_hdr->ether_dhost,6);
	        memcpy(eth_hdr->ether_dhost, eth_hdr->ether_shost, 6);
	        memcpy(eth_hdr->ether_shost, temp,6);

	        uint32_t s_addr = ip_hdr->saddr;
	        ip_hdr->saddr = ip_hdr->daddr;
	        ip_hdr->daddr = s_addr;
	        eth_hdr->ether_type = htons(ETHERTYPE_IP);
			ip_hdr->version = 4;
			ip_hdr->ihl = 5;
			ip_hdr->tos = 0;
			ip_hdr->tot_len = htons(sizeof(struct iphdr)+sizeof(struct icmphdr));
			ip_hdr->id = htons(getpid());
			ip_hdr->frag_off = 0;
			ip_hdr->ttl = 64;
			ip_hdr->protocol = IPPROTO_ICMP;

			ip_hdr->check = 0;
			ip_hdr->check = ip_checksum(ip_hdr,sizeof(struct iphdr));

			icmp->code = 0;
			icmp->type = type;
			
			icmp->un.echo.id=htons(getpid());
			icmp->checksum=0;
			icmp->checksum=ip_checksum(icmp, sizeof(struct icmphdr));
}

int main(int argc, char *argv[]) {
	packet m;
	int rc;
	read_rtable();
	read_arp();
	//sortare tabela de rutare pentru cautarea binara
	qsort(in, rtable_size, sizeof(rtable), comparator);
	init();
 	setvbuf(stdout, NULL, _IONBF, 0);

	while (1) {
	    rc = get_packet(&m);
	    DIE(rc < 0, "get_message");
	    struct ether_header *eth_hdr = (struct ether_header *)m.payload;
	    struct iphdr *ip_hdr = (struct iphdr *)(m.payload + sizeof(struct ether_header));
	    struct icmphdr *icmp = (struct icmphdr *)(m.payload + sizeof(struct ether_header) + sizeof(struct iphdr));
	    m.len = sizeof(struct ether_header) + sizeof(struct iphdr) + sizeof(struct icmphdr);

	    int k = 0;
	    //Echo reply
	    if(icmp->type == ICMP_ECHO) {
	    	for(int i = 0; i < ROUTER_NUM_INTERFACES && k == 0; i++) {
	    		uint32_t temp;
	    		inet_pton(AF_INET, get_interface_ip(i), &temp);
	    		if(temp == ip_hdr->daddr) {
	    			setIphdr_Icmphdr(ip_hdr, icmp, eth_hdr, m, ICMP_ECHOREPLY);
					send_packet(m.interface, &m);
					k++;
	    		}
	    	}
	    }

	    if(k == 1){
	    	continue;
	    }

		if (ip_checksum(ip_hdr, sizeof(struct iphdr)) != 0) {
	        printf("Wrong checksum\n");
	        continue; 
	    }

	    //Time exceeded
	    if (ip_hdr->ttl <= 1) {
	        printf("Time to live!\n");
	        setIphdr_Icmphdr(ip_hdr, icmp, eth_hdr, m, 11);
			send_packet(m.interface, &m);
	        continue;
	    }

	    int kl = binarySearch(0, rtable_size, ip_hdr->daddr);
	    struct route_table_entry *route = NULL;
	    if(kl != -1) {
	    	route = &in[kl];
		}

		//Destination unreachable
	    if (route == NULL) {
	        printf("No router found!\n");
	       	setIphdr_Icmphdr(ip_hdr, icmp, eth_hdr, m, 3);
			send_packet(m.interface, &m);
	        continue;
	    }

	    ip_hdr->ttl--;
	    ip_hdr->check = 0;
	    ip_hdr->check = ip_checksum(ip_hdr, sizeof(struct iphdr));
	    

	    struct  arp_entry *arpp = get_arp_entry(ip_hdr->daddr);
	    if (arpp == NULL) {
	        printf("No ARP entry found!\n");
	        continue;
	    }
	    //updat adresa Ethernet
	    memcpy(eth_hdr->ether_dhost, arpp->mac, sizeof(arpp->mac) + 1);

	    send_packet(route->interface, &m);
	}
}


