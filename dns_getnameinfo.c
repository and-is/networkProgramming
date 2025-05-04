// The function used here is used to convert addrinfo structure back into a text format, 
// text meaning hostname and service name, can be numeric too
// parameters passed are sockaddr structure address, its len, hostname variable, length, service name variable, length and flags

#include "dns_chap_headers.h"

int main(){
	struct addrinfo *address;
	char host[100];
	char serv[100];
	getnameinfo(address->ai_addr, address->ai_addrlen, host, sizeof(host), serv, sizeof(serv), NI_NUMERICHOST | NI_NUMERICSERV);
	printf("%s %s\n", host, serv);

// those flags will make this send ip adres instead of the name, not having the flags(sending 0 instead) prints name and service.	
}

