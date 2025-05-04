#include "httpclient_headers.h"
#include "url_parsing.h"

#define TIMEOUT 5.0

#define RESPONSE_SIZE 8192

void parse_url(char *url, char **hostname, char **port, char **path);

void send_request (SOCKET s, char *hostname, char *port, char *path){
	char buffer[2048];

	sprintf(buffer, "GET /%s HTTP/1.1\r\n", path);
	sprintf(buffer + strlen(buffer), "Host: %s:%s\r\n", hostname, port);
	sprintf(buffer + strlen(buffer), "Connection: close\r\n");
	sprintf(buffer + strlen(buffer), "User-Agent: honpwc web_get 1.0\r\n");
	sprintf(buffer + strlen(buffer), "\r\n");

	send(s, buffer, strlen(buffer), 0);
	printf("Sent Headers:\n&s", buffer);
}

SOCKET connect_to_host (char *hostname, char *port){
	printf("Configuring remote address...\n");
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	struct addrinfo *peer_address;
	if (getaddrinfo(hostname, port, &hints, &peer_address)){
		fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
		exit(1);
	}
	
	printf("Remote address is: ");
	char address_buffer[100];
	char service_buffer[100];
	getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen, address_buffer, sizeof(address_buffer), service_buffer, sizeof(service_buffer), NI_NUMERICHOST);
	printf("%s %s\n", address_buffer, service_buffer);

	printf("Creating socket...\n");
	SOCKET server;
	server = socket(peer_address->ai_family, peer_address->ai_socktype, peer_address->ai_protocol);
	if (!ISVALIDSOCKET(server)){
		fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
		exit(1);
	}

	printf("Connecting...\n");
	if (connect(server, peer_address->ai_addr, peer_address->ai_addrlen)){
		fprintf(stderr, "connect() failed. (%d)\n", GETSOCKETERRNO());
		exit(1);
	}
	freeaddrinfo(peer_address);

	printf("Connected. \n\n");

	return server;
}

int main(int argc, char *argv[]){
	if (argc<2){
		fprintf(stderr, "usage: web_get url\n");
		return 1;
	}
	char *url = argv[1];
	
	char *hostname, *port, *path;
	parse_url(url, &hostname, &port, &path);
	SOCKET server = connect_to_host(hostname, port);
	send_request(server, hostname, port, path);

	const clock_t start_time = clock();
	char response[RESPONSE_SIZE+1];	
	char *p = response, *q;
	char *end = response + RESPONSE_SIZE;
	char *body = 0;

	enum {length, chunked, connection};
	int encoding = 0;
	int remaining = 0;

	while (1){
		if ((clock() - start_time) / CLOCKS_PER_SEC > TIMEOUT) {
			fprintf(stderr, "timeout after %.2f seconds\n", TIMEOUT);
			return 1;
		}

		if (p == end){
			fprintf(stderr, "out of buffer space\n");
			return 1;
		}

		fd_set reads;
		FD_ZERO(&reads);
		FD_SET(server, &reads);

		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 200000;

		if (select(server+1, &reads, 0, 0, &timeout) < 0) {
			fprintf(stderr, "select() failed. (%d)\n", GETSOCKETERRNO());
			return 1;
		}
		if (FD_ISSET(server, &reads)){
			int bytes_received = recv(server, p, end - p, 0);
			if (bytes_received < 1){
				if (encoding == connection && body){
					printf("%.*s", (int)(end-body), body);
				}
				printf("\n Connection closed by peer.\n");
				break;
			}

		/*	printf("Received (%d bytes): '%.*s'", bytes_received, bytes_received, p);*/
			p += bytes_received;
			*p = 0;
			if (!body && (body = strstr(response, "\r\n\r\n"))) {
				*body = 0;
				body += 4;
				printf("Received Headers:\n%s\n", response);
				q = strstr(response, "\n Content-length: ");
				if (q) {
					encoding = length;
					q = strchr(q, ' ');
					q+=1;
					remaining = strtol(q, 0, 10);
				} else {
					q = strstr(response, "\n Transfer-Encoding: chunked");
					if (q){
						encoding = chunked;
						remaining = 0;
					} else {
						encoding = connection;
					}
				}
				printf("\nReceived Body: \n");
			}
			if (body){
				if (encoding == length) {
					if (p - body >= remaining) {
						printf("%.*s", remaining, body);
						break;
					}
				} else if (encoding == chunked) {
					do {
						if (remaining==0){
							if ((q = strstr(body, "\r\n"))){
								remaining = strtol(body, 0, 16);
								if (!remaining) goto finish;
								body = q + 2;
							} else {
								break;
							}
						}
						if (remaining && p - body >= remaining) {
							printf("%.*s", remaining, body);
							body += remaining + 2;
							remaining = 0;
						}
					} while (!remaining);	
				}
			}
		}
	}
finish:
	printf("\nClosing socket...\n");
	CLOSESOCKET(server);

	printf("Finished.\n");
	return 0;
}
