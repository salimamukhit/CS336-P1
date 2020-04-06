
#ifndef _STRUCTS_H_
#define _STRUCTS_H_

#define UDPHLEN 8

// UDP info
struct udpinfo {
    unsigned short int udph_srcport;
    unsigned short int udph_destport;
    unsigned short int udph_len;
    unsigned short int udph_chksum;
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
};

struct tcphdr {
    __u16 	source;
    __u16 	dest;
    __u32 	seq;
    __u32 	ack_seq;
    __u16 	res1;
    __u16 	doff;
    __u16 	fin;
    __u16 	syn;
    __u16 	rst;
    __u16 	psh;
    __u16 	ack;
    __u16 	urg;
    __u16 	ece;
    __u16 	cwr;
    __u16 	window;
    __u16 	check;
    __u16 	urg_ptr;

}


#endif