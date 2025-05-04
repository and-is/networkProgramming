// The function used here is used to convert names and ports into address (addrinfo) structure
// the inputs to this function are node(website name), service(port), hints and address to store 

#include "dns_chap_headers.h"

int main(){
struct addrinfo hints;
memset(&hints, 0, sizeof(hints));
hints.ai_flags = AI_ALL;
struct addrinfo *peer_address;
if(getaddrinfo("example.com", 0, &hints, &peer_address)){
	fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
	return 1;
}

struct addrinfo *address = peer_address;
do {
	printf("idk what to do here, so only to check.\n");
} while ((address = address->ai_next));
freeaddrinfo(peer_address);
return 0;
}
