#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>

#define PORT 4950

#define MAXBUFLEN 100

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
	return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(void){
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN];
    char s[INET6_ADDRSTRLEN];
    socklen_t addr_len;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, "4950", &hints, &servinfo)) != 0) {
	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
	if ((sockfd = socket(p->ai_family, p->ai_socktype,
			     p->ai_protocol)) == -1) {
	    perror("server: socket");
	    continue;
	}
	if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
	    close(sockfd);
	    perror("server: bind");
	    continue;
	}
	break;
    }

    if (p == NULL) {
	fprintf(stderr, "server: failed to bind socket\n");
	return 2;
    }

    freeaddrinfo(servinfo);

    printf("server: waiting for connections...\n");

    addr_len = sizeof their_addr;
    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
	     (struct sockaddr *)&their_addr, &addr_len)) == -1) {
	perror("recvfrom");
	exit(1);
    }

    printf("server: got packet from %s\n",
	   inet_ntop(their_addr.ss_family,
		     get_in_addr((struct sockaddr *)&their_addr),
		     s, sizeof s));
    printf("server: packet is %d bytes long\n", numbytes);
    buf[numbytes] = '\0';
    printf("server: packet contains \"%s\"\n", buf);

    close(sockfd);

    return 0;

}
