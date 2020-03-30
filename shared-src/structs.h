
#ifndef _STRUCTS_H_
#define _STRUCTS_H_

struct ipheader {
    unsigned char iph_ihl: 5, iph_ver: 4;
    unsigned char iph_tos;
    unsigned short int iph_len;
    unsigned short int iph_ident;
    unsigned char iph_flag;
    unsigned short int iph_offset;
    unsigned char iph_ttl;
    unsigned char iph_protocol;
    unsigned short int iph_chksum;
    unsigned int iph_sourceip;
    unsigned int iph_destip;
};

// UDP header's structure
struct udpheader {
    unsigned short int udph_srcport;
    unsigned short int udph_destport;
    unsigned short int udph_len;
    unsigned short int udph_chksum;
};

struct udppacket {
    struct ipheader iphdr;
    struct udpheader udphdr;
    char data[];
};

struct ini_info {
    char file_name[1024];
    struct in_addr server_ip;
    struct udpheader train_udp;
    unsigned short head_port;
    unsigned short tail_port;
    unsigned short server_port;
    unsigned short int payload_size;
    time_t meas_time;
    unsigned short int packet_num;
    unsigned short int packet_ttl;
};


#endif