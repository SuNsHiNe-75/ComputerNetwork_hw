#include "mytcp.h"

int RTT=20,threshold=65536,MSS=1024,buffer_size=524288;
int cwnd=1,rwnd=32768;
int cli_len,newfd;
pid_t pid;
Segm pack_info;
void mysend(int fd,char* name,short port);
struct sockaddr_in srv; 
struct sockaddr_in cli;
struct sockaddr_in new_srv;
Segm seg;			// segment structure variable
PSegm pseg;			// pointer to a segment

int mylisten(short port){
	int fd;
	cli_len = sizeof(cli);
	fd = socket(AF_INET, SOCK_DGRAM, 0);	//domain : Internet protocol ,type : UDP
	if (fd<0){perror ("socket failed");exit(1);}
	memset(&srv,0,sizeof(srv));
	srv.sin_family = AF_INET;
	srv.sin_port = htons(port);
    	inet_aton("127.0.0.1", &srv.sin_addr);	//local host 
	if (bind(fd, (struct sockaddr *)&srv, sizeof(srv)) < 0) {perror ("bind failed\n"); exit(1);}
	printf("=====Parameter=====\n");
    	printf("The RTT delay = %d ms\n",RTT);
    	printf("The threshold = %d bytes\n",threshold);
    	printf("The MSS = %d bytes\n",MSS);
    	printf("Buffer size = %d bytes\n", buffer_size);
    	printf("Server's ip is %s\n", inet_ntoa(srv.sin_addr));
    	printf("Server is listening on port %hu\n", port);
    	printf("===============\n");
    	printf("listening for client...\n");
    	/* establish connection */
	while (1){
		//first handshake receive from client
		recvfrom(fd,pkg,PKG_LEN,0,(struct sockaddr*)&cli,&cli_len);
		port++;
		newfd = socket(AF_INET, SOCK_DGRAM, 0);	//domain : Internet protocol ,type : UDP
		new_srv.sin_family = AF_INET;
		new_srv.sin_port = htons(port);
    		inet_aton("127.0.0.1", &srv.sin_addr);
		if (bind(newfd, (struct sockaddr *)&new_srv, sizeof(new_srv)) < 0) {perror ("bind failed\n"); exit(1);}
		pid=fork();
		if (pid==0){
			printf("=====Start three-way handshake=====\n");
			printf("Received a packet(SYN) from %s : %hu\n", inet_ntoa(cli.sin_addr), ntohs(cli.sin_port));
			pseg = get_segm(pkg, header);
			printf("\t Get a packet (seq = %hu , ack = %hu)\n", pseg->seq, pseg->ack_seq);
			if (pseg->syn){	//if the pkg's syn flag is true, then send a syn=1 and ack=1 's pkg to client(twice hand shake)
				seg.dest_port=pseg->src_port;	
				seg.src_port=port;
				seg.syn=1;
				seg.ack=1;
				seg.ack_seq=pseg->seq+1;
				seg.seq=(short)rand()%10000+1;
		        	mk_pkg(pkg, seg);
				if (sendto(newfd, pkg, PKG_LEN, 0, (struct sockaddr*)&cli, sizeof(cli))<0) return -1;
				printf("Send a packet(SYN/ACK) to %s : %hu\n", inet_ntoa(cli.sin_addr), pseg->dest_port);
			}
			else{fprintf(stderr, "segment SYN bit not set! \n");return -1;}
			//third handshake(catch pkg)
			recvfrom(newfd,pkg,PKG_LEN,0,(struct sockaddr*)&cli,&cli_len);
			printf("Receive an pakcet(ACK) from %s : %hu\n", inet_ntoa(cli.sin_addr), ntohs(cli.sin_port));	
			pseg = get_segm(pkg, header);
			printf("\t Get a packet (seq = %hu , ack = %hu)\n", pseg->seq, pseg->ack_seq);
			if (pseg->ack && (pseg->ack_seq==seg.seq+1)){
				printf("=====Complete three-way handshake=====\n");
		        	char buffer[BUF_SIZE]; 
		    		bzero(buffer, BUF_SIZE); 
				//when handshake is over, receive requsted data name from client
		    		if(recvfrom(newfd, buffer, BUF_SIZE,0, (struct sockaddr*)&cli, &cli_len) == -1) { 
					printf("Receive Data Failed:"); 
					exit(1);
				}
		    		int FILE_NAME_MAX_SIZE=512;
		    		char file_name [FILE_NAME_MAX_SIZE+1]; 
		    		bzero(file_name,FILE_NAME_MAX_SIZE+1); 
		    		strncpy(file_name, buffer, strlen(buffer)>FILE_NAME_MAX_SIZE?FILE_NAME_MAX_SIZE:strlen(buffer)); 
		        	printf("Start to send file %s to  %s : %hu\n",file_name,inet_ntoa(cli.sin_addr), pseg->src_port);
		        	mysend(newfd,file_name,port);
		        	return newfd;
			}
			else{ fprintf(stderr, "WRONG ACK! \n"); return -1;}
		}
	}
}

void mysend(int fd, char* name, short port){
	FILE *fp = fopen(name,"r");
	if(fp==NULL) printf("File:%s Not Found.\n",name);
	else{
		printf("======slow start======\n");
    		int len=0,seq=1;
        	bzero((char *)&pkt,sizeof(pkt));
		while(1){
			if (cwnd>32768) cwnd=32768;
        		printf("cwnd = %d, rwnd = %d, threshold = %d \n",cwnd, rwnd ,threshold);
        		int temp=cwnd;	//when cwnd > 1024, cut cwnd into segment, and every segment's size is 1 MSS
			if (cwnd>1024){
				while (temp>0){
					printf("\tSend a packet at %d byte\n",seq);
					if((len = fread(pkt.data, sizeof(char), MSS, fp))>0){
						pkt.head.seq=seq;
						seq+=len;
						pkt.head.d_size=len;
			            		pkt.head.dest_port = pseg->src_port;
			        		pkt.head.src_port = port;
			        		pkt.head.ack_seq = pseg->seq + 1;
			            		if(sendto(fd, (char*)&pkt, sizeof(pkt),0, (struct sockaddr*)&cli, sizeof(cli)) < 0){ 
			              			printf("Send File Failed:\n"); 
			              			break; 
			            		}
						if(recvfrom(fd, pkg, PKG_LEN, 0, (struct sockaddr*)&cli, &cli_len)<0) printf("receive Failed:\n"); 
						pseg = get_segm(pkg,header);
						if(pseg->seq >= pkt.head.ack_seq){
			            			printf("\t Receive a packet (seq_num = %hu , ack_num = %hu)\n", pseg->seq, pseg->ack_seq);
			            			if(rwnd-MSS<0) rwnd+=32768-MSS;
			            			else rwnd-=MSS;
			            		}
						temp-=MSS;
					}
					else{
		        			sendto(fd, (char*)&pkt, 0, 0, (struct sockaddr*)&cli, sizeof(cli));
		        			printf("file successfully transmit\n");
		        			return;
					}
				}
				cwnd*=2;
			}
			else if (cwnd<=1024){
				printf("\tSend a packet at %d byte\n",seq);
				if((len = fread(pkt.data, sizeof(char), cwnd, fp))>0){
					pkt.head.seq=seq;
					seq+=len;
					pkt.head.d_size=len;
		            		pkt.head.dest_port = pseg->src_port;
		        		pkt.head.src_port = port;
		        		pkt.head.ack_seq = pseg->seq + 1;
		            		if(sendto(fd, (char*)&pkt, sizeof(pkt),0, (struct sockaddr*)&cli, sizeof(cli)) < 0){ 
		              			printf("Send File Failed:\n"); 
		              			break; 
		            		}
					if(recvfrom(fd, pkg, PKG_LEN, 0, (struct sockaddr*)&cli, &cli_len)<0) printf("receive Failed:\n"); 
					pseg = get_segm(pkg,header);
					if(pseg->seq >= pkt.head.ack_seq){
		            			printf("\t Receive a packet (seq_num = %hu , ack_num = %hu)\n", pseg->seq, pseg->ack_seq);
		            			if(rwnd-cwnd<0) rwnd+=32768-cwnd;
		            			else rwnd-=cwnd;
		            		}
					cwnd*=2;
				}
	        		else{
		        		sendto(fd, (char*)&pkt, 0, 0, (struct sockaddr*)&cli, sizeof(cli));
		        		printf("file transmit success\n");
		        		return;
		       		}
			}
	        	else{
	        		sendto(fd, (char*)&pkt, 0, 0, (struct sockaddr*)&cli, sizeof(cli));
	        		fclose(fp);
	        		printf("file transmit success\n");
	        		return;
	        	}
		}
	}
	
}
int main(int argc,char *argv[]){
	srand(time(NULL));
	if (mylisten((short)atoi(argv[1]))<0) perror("mylisten");
	return 0;
}
