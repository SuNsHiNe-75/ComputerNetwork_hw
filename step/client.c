#include "mytcp.h"

int fd,srv_len,newfd;
struct sockaddr_in cli;
struct sockaddr_in srv;
Segm seg;           // segment structure variable
PSegm pseg;         // pointer to a segment

int myreceive(char *name, short srv_port){
	char w_name[20]="receive_";
	strcat(w_name,name);
	printf("%s\n",w_name);
	FILE *fp = fopen(w_name, "w");
	if( fp == NULL) { printf("File: Can Not Open To Write\n"); exit(1);}
    	int len = 0,seq = 1;
	while (1){
        	len = recvfrom(fd, (char*)&pkt, sizeof(pkt), 0,  (struct sockaddr*)&srv, &srv_len);
		if (len>0){
            		printf("\t Receive a packet (seq_num = %hu , ack_num = %hu)\n",pkt.head.seq, pkt.head.ack_seq);
            		if(pkt.head.seq == seq){
                		seg.src_port = CLI_PORT;
                		seg.dest_port = srv_port;
                		seg.ack_seq = pkt.head.seq + pkt.head.d_size;
                		seq = pkt.head.seq + pkt.head.d_size;
                		seg.seq =  pkt.head.ack_seq; 
               			mk_pkg(pkg, seg);
                		fwrite(pkt.data, sizeof(char), pkt.head.d_size , fp);
                		if(sendto(fd, pkg, PKG_LEN, 0, (struct sockaddr*)&srv, sizeof(srv))<0) printf("error packet");
            		}
		}
		else{
            		fclose(fp);
            		printf("file successfully transmit\n");
            		break;
		}
	}
}

int myconnect(char* srv_ip, char *name, short srv_port){
	srv_len = sizeof(srv);
	srand(time(NULL));
	if (inet_pton(AF_INET, srv_ip, &srv.sin_addr) == 0) return -1;
	srv.sin_family = AF_INET;
	srv.sin_port = htons(srv_port);
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) { perror ("socket failed!"); exit(1);}
    	int cli_port =(rand()%1000)+10000;
    	memset(&cli, 0, sizeof(cli));
    	cli.sin_family = AF_INET;
    	cli.sin_addr.s_addr = htonl(INADDR_ANY);
    	cli.sin_port = htons(cli_port);

    	memset(&seg, 0, sizeof(seg));
    	seg.src_port = CLI_PORT;
    	seg.dest_port = srv_port;
    	seg.seq = (short)(rand() % 10000);
    	seg.syn = 1;
    	mk_pkg(pkg, seg);
    	printf("=====Start three-way handshake=====\n");
	//first handshake(set syn=1)
    	if (sendto(fd, pkg, PKG_LEN, 0, (struct sockaddr*)&srv, sizeof(srv))<0) return -1;
    	printf("Send a packet(SYN) to %s : %hu\n", srv_ip, srv_port);
	//twice handshake(catch pkg)
    	recvfrom(fd, pkg, PKG_LEN, 0, (struct sockaddr*)&srv, &srv_len);
    	printf("Receive a packet(SYN/ACK) from %s : %hd\n", inet_ntoa(srv.sin_addr), ntohs(srv.sin_port));
    	pseg = get_segm(pkg, header);
    	printf("\t Receive a packet (seq_num = %hu , ack_num = %hu)\n", pseg->seq, pseg->ack_seq);
    	if (pseg->syn && pseg->ack && pseg->ack_seq == seg.seq + 1){
        	seg.src_port = CLI_PORT;
        	seg.dest_port = srv_port;
        	seg.syn = 0;seg.ack = 1;
        	seg.ack_seq = pseg->seq + 1;
        	seg.seq = pseg->ack_seq; 
        	mk_pkg(pkg, seg);
		//third handshake (if catch syn=1 &ack=1, then set ack=1)
        	if(sendto(fd, pkg, PKG_LEN, 0, (struct sockaddr*)&srv, sizeof(srv))<0) return -1;
        	printf("Send a packet(ACK) to %s : %hu\n", srv_ip, srv_port);
    	}
    	else{fprintf(stderr, "WRONG SYN/ACK! \n");return -1;}
    	printf("=====Complete three-way handshake=====\n");
    	int FILE_NAME_MAX_SIZE=512;
    	char file_name[FILE_NAME_MAX_SIZE+1]; 
    	bzero(file_name, FILE_NAME_MAX_SIZE+1); 
    	strcpy(file_name,name); 
    	char buffer[BUF_SIZE]; 
    	bzero(buffer, BUF_SIZE); 
    	strncpy(buffer, file_name, strlen(file_name)>BUF_SIZE?BUF_SIZE:strlen(file_name));
	//send the file name to server
    	if(sendto(fd, buffer, BUF_SIZE,0,(struct sockaddr*)&srv, sizeof(srv)) < 0){ printf("Send File Name Failed.\n"); exit(1);}
    	myreceive(name,srv_port);
    	return fd;
}

int main(int argc, char *argv[]){
	if (argc<4){
        	fprintf(stderr, "WRONG ARGUMENT.\n");
        	exit(1);
	}
	else {for (int i=4;i<=argc;i++) if (myconnect(argv[1], argv[i-1], (short)atoi(argv[2]))<0) perror("myconnect");}
	return 0;
}
