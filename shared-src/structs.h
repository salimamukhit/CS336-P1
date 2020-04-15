#ifndef _STRUCTS_H_
#define _STRUCTS_H_

#define UDPHLEN 8

#include <net/ethernet.h>

// UDP info
struct udpinfo {
    unsigned short int udph_srcport;
    unsigned short int udph_destport;
};

struct ini_info {
    unsigned int size;
    char file_name[1024];
    struct in_addr server_ip;
    struct udpinfo train_udp;
    unsigned short head_port;
    unsigned short tail_port;
    unsigned short server_port;
    unsigned short int payload_size;
    time_t meas_time;
    unsigned short int packet_num;
    unsigned short int packet_ttl;
    char interface[20];
    char client_ip[17];
    char standalone_dst[17];
};

struct ipheader {
 unsigned char      iph_ihl:5, /* Little-endian */
                    iph_ver:4;
 unsigned char      iph_tos;
 unsigned short int iph_len;
 unsigned short int iph_ident;
 unsigned char      iph_flags;
 unsigned short int iph_offset;
 unsigned char      iph_ttl;
 unsigned char      iph_protocol;
 unsigned short int iph_chksum;
 unsigned int       iph_sourceip;
 unsigned int       iph_destip;
};

struct tcpheader {
 unsigned short int tcph_srcport;
 unsigned short int tcph_destport;
 unsigned int       tcph_seqnum;
 unsigned int       tcph_acknum;
 unsigned char      tcph_reserved:4, tcph_offset:4;
 // unsigned char tcph_flags;
    unsigned int
       tcp_res1:4,      /*little-endian*/
       tcph_hlen:4,     /*length of tcp header in 32-bit words*/
       tcph_fin:1,      /*Finish flag "fin"*/
       tcph_syn:1,       /*Synchronize sequence numbers to start a connection*/
       tcph_rst:1,      /*Reset flag */
       tcph_psh:1,      /*Push, sends data to the application*/
       tcph_ack:1,      /*acknowledge*/
       tcph_urg:1,      /*urgent pointer*/
       tcph_res2:2;
 unsigned short int tcph_win;
 unsigned short int tcph_chksum;
 unsigned short int tcph_urgptr;
};

/* Ethernet header */
struct sniff_ethernet {
    unsigned char ether_dhost[ETHER_ADDR_LEN]; /* Destination host address */
    unsigned char ether_shost[ETHER_ADDR_LEN]; /* Source host address */
    unsigned short ether_type; /* IP? ARP? RARP? etc */
};

/* IP header */
struct sniff_ip {
    unsigned char ip_vhl;		/* version << 4 | header length >> 2 */
    unsigned char ip_tos;		/* type of service */
    unsigned short ip_len;		/* total length */
    unsigned short ip_id;		/* identification */
    unsigned short ip_off;		/* fragment offset field */
#define IP_RF 0x8000		/* reserved fragment flag */
#define IP_DF 0x4000		/* dont fragment flag */
#define IP_MF 0x2000		/* more fragments flag */
#define IP_OFFMASK 0x1fff	/* mask for fragmenting bits */
    unsigned char ip_ttl;		/* time to live */
    unsigned char ip_p;		/* protocol */
    unsigned short ip_sum;		/* checksum */
    struct in_addr ip_src,ip_dst; /* source and dest address */
};
#define IP_HL(ip)		(((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)		(((ip)->ip_vhl) >> 4)

/* TCP header */
typedef unsigned int tcp_seq;

struct sniff_tcp {
    unsigned short th_sport;	/* source port */
    unsigned short th_dport;	/* destination port */
    tcp_seq th_seq;		/* sequence number */
    tcp_seq th_ack;		/* acknowledgement number */
    unsigned char th_offx2;	/* data offset, rsvd */
#define TH_OFF(th)	(((th)->th_offx2 & 0xf0) >> 4)
    unsigned char th_flags;
#define TH_FIN 0x01
#define TH_SYN 0x02
#define TH_RST 0x04
#define TH_PUSH 0x08
#define TH_ACK 0x10
#define TH_URG 0x20
#define TH_ECE 0x40
#define TH_CWR 0x80
#define TH_FLAGS (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
    unsigned short th_win;		/* window */
    unsigned short th_sum;		/* checksum */
    unsigned short th_urp;		/* urgent pointer */
};

#endif