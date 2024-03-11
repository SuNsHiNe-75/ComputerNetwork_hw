#include "mytcp.h"

PSegm get_segm(char *pkg, char *header){
	memcpy(header, pkg, HEAD_LEN);	//copy the package's header to header
	return (PSegm)header;	
}

void mk_pkg(char *pkg, Segm s){
	memcpy(pkg, &s, sizeof(s));		//copy the segment information to the package
}
