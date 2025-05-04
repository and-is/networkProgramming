#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)

#include <stdio.h>
#include <string.h>
#include <time.h>

int main(){
	printf("Configuring local address...\n");
	struct addrinfo hints;

/* hints is set as parameters for getaddrinfo
 * memset used to set hints as 0
 * family to be ipv6, socktype to be tcp and 
 * passive flags to bind to wildcard address
 */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
// this AF_INET6 means IPv6 and to go to loopback use [::1]:8080 in the browser.
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

// pointer initialized as addrinfo structure to hold return information
// from getaddrinfo
// 8080 is the port to listen to for our trial here
	struct addrinfo *bind_address;
	getaddrinfo(0, "8080", &hints, &bind_address);
	printf("Creating socket...\n");
	SOCKET socket_listen;
	socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
// socket() takes 3 parameters: family, type and protocol
	if (!ISVALIDSOCKET(socket_listen)){
		fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
		return 1;
	}
	
	printf("Binding socket to local address...\n");
	if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)){
		fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
		return 1;
	}
	freeaddrinfo(bind_address);

	printf("Listening...\n");
	if (listen(socket_listen, 10) < 0){
		fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
		return 1;
// 10 as argument to listen() is the number of connections allowed to queue up
	}
	printf("Waiting for connection...\n");
	struct sockaddr_storage client_address;

// to store address info for connecting client
	socklen_t client_len = sizeof(client_address);
	SOCKET socket_client = accept(socket_listen,
			(struct sockaddr*) &client_address, &client_len);

// accept() will fill the client_address and client_len variable upon returning	
// accept() will block the program until a new connection is made
// when it is made, accept() creates a new socket 
// new socket thus created can be used to send and received data over the new connection
	if (!ISVALIDSOCKET(socket_client)){
		fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO());
		return 1;
	}

	printf("Client is connected... ");
	char address_buffer[100];
	getnameinfo((struct sockaddr*) &client_address, client_len, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);

// first two arguments are client's address and length, then output buffer and its length
// then second buffer and its length, it outputs service name which we don't need here
// finally the flag to specify we want to see hostname as an IP address
	printf("%s\n", address_buffer);
	printf("Reading request...\n");
	char request[1024];
	int bytes_received = recv(socket_client, request, 1024, 0);

// request buffer to store browser's http request, allocated 1024 bytes
// recv() is called with client's socket, request buffer and its size, last is flags
// recv() returns the number of bytes that are received
// if nothing received, it blocks until it has something
	printf("Received %d bytes.\n", bytes_received);
	printf("%.*s", bytes_received, request);

// this format tells printf() to print a specific number of characters as specified by 
// bytes_received, printing directly as C string may give segmentation fault due to 
// request string may not being null terminated

// now sending the response back
	printf("Sending response...\n");
	const char *response = 
		"HTTP/1.1 200 OK\r\n"
		"Connection: close\r\n"
		"Content-Type: text/plain\r\n\r\n"
		"Local time is: ";
// response string set for default http response formatting
	int bytes_sent = send(socket_client, response, strlen(response), 0);
	printf("Sent %d of %d bytes.\n", bytes_sent, (int)strlen(response));
// now sending the time 
	time_t timer;
	time(&timer);
	char *time_msg = ctime(&timer);
	bytes_sent = send(socket_client, time_msg, strlen(time_msg), 0);
	printf("Sent %d of %d bytes.\n", bytes_sent, (int)strlen(time_msg));

// now closing connection on client side
	printf("Closing connection...\n");
	CLOSESOCKET(socket_client);	
	
// now closing connection for this server
	printf("Closing listening socket...\n");
	CLOSESOCKET(socket_listen);

	printf("Finished.\n");
	return 0;
}	
