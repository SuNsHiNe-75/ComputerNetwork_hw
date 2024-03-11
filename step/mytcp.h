#ifndef MYTCP_H_
#define MYTCP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <math.h>

#define SRV_PORT 8000
#define CLI_PORT 8001
#define HEAD_LEN 32
#define PKG_LEN  544
#define BUF_SIZE 1024

char pkg[PKG_LEN];
char header[HEAD_LEN];

typedef struct{
	short src_port;		// 16-bit source port
	short dest_port;	// 16-bit destination port
	int   seq;		// 32-bit sequence number
	int   ack_seq;		// 32-bit acknowledgement number
	short head_len:4,	//  4-bit header length
	      not_use:6,	//  6-bit reserved
	      urg:1,		//  1-bit urgent flag
	      ack:1,		//  1-bit acknowledgement flag
	      psh:1,		//  1-bit push flag
	      rst:1,		//  1-bit reset flag
	      syn:1,		//  1-bit syn flag
	      fin:1;		//  1-bit finish flag
	short rcv_win;		// 16-bit receive window
	short checksum;		// 16-bit checksum
	short urg_ptr;		// 16-bit urgent pointer
	int   d_size;
}Segm, *PSegm;

PSegm get_segm(char *pkg, char *header);
void mk_pkg(char *pkg, Segm s);
struct sendpkt{
	Segm head;
	char data[BUF_SIZE];
}pkt;

#endif
