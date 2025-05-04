#include "smtp_headers.h"

#include <ctype.h>
#include <stdarg.h>
#include <time.h>

#define MAXINPUT 512
#define MAXRESPONSE 1024

void get_input(const char *prompt, char *buffer){
	printf("%s", prompt);
	buffer[0] = 0;

	// Read a line from stdin, up to MAXINPUT characters, retains new line, so we just overwrite a termination character at the end
	fgets(buffer, MAXINPUT, stdin);
	const int read = strlen(buffer);
	if (read > 0)
		buffer[read-1] = 0;
}

void send_format(SOCKET server, const char *text, ...){
	char buffer[1024];
	va_list args;
	va_start(args, text);

// vsprintf() formats the text into the buffer, (buffer overflow risk) 
// variable length argument list is passed to vsprintf, va things earlier do the initialization
	vsprintf(buffer, text, args);
	va_end(args);

	// Send the formatted string to the server
	send(server, buffer, strlen(buffer), 0);
	printf("C: %s", buffer);
}

// response needs to be parsed correctly, understand codes, see dash to check multiline response
// if another msg sent without getting the response, connection is terminated by the server

int parse_response(const char *response){
	const char *k = response;
// check for termination character in the first three digits, if valid code, loop through response
	if (!k[0] || !k[1] || !k[2]) return 0;
	for (; k[3] ; ++k){
// loop through first 3 and 4th digit, if response reached or newline encountered prior, skip
		if (k == response || k[-1] == '\n'){
// check if all are digits, meaning status code
			if (isdigit(k[0]) && isdigit(k[1]) && isdigit(k[2])){
// check if 4th character is a dash, which implies multiline message
				if (k[3] != '-'){
// if no dash, k[0] represents beginning of last line of response, check line ending. if ended, convert response into an integer
					if (strstr(k, "\r\n")){
						return strtol(k, 0, 10);
					}
				}
			}
		}
	}
	return 0;
}

// wait until a response code is received
void wait_on_response(SOCKET server, int expecting){
	char response[MAXRESPONSE+1];
	char *p = response;
	char *end = response + MAXRESPONSE;
// pointers for buffer set, code = 0 indicates no response codes received yet
	int code = 0;

	do {
		int bytes_received = recv(server, p, end - p, 0);
		if (bytes_received <= 0){
			fprintf(stderr,"Connection dropper.\n");
			exit(1);
		}

		p += bytes_received;
		*p = 0;

		if (p == end){
			fprintf(stderr,"Server response too large:\n");
			fprintf(stderr,"%s", response);
			exit(1);
		}

		code = parse_response(response);
	} while (code == 0);

	if (code != expecting) {
		fprintf(stderr, "Error from server:\n");
		fprintf(stderr, "%s", response);
		exit(1);
	}

	printf("SL: %s", response);
}

SOCKET connect_to_host(const char *hostname, const char *port) {
	printf("Configuring remote address...\n");
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	struct addrinfo *peer_address;
	if (getaddrinfo(hostname, port, &hints, &peer_address)) {
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
	if (!ISVALIDSOCKET(server)) {
		fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
		exit(1);
	}

	printf("Connecting...\n");
	if (connect(server, peer_address->ai_addr, peer_address->ai_addrlen)) {
		fprintf(stderr, "connect() failed. (%d)\n", GETSOCKETERRNO());
		exit(1);
	}
	freeaddrinfo(peer_address);
	printf("Connected.\n\n");
	return server;
}

int main(){
	char hostname[MAXINPUT];
	get_input("mail server: ", hostname);

	printf("Connecting to host: %s:25\n", hostname);

	SOCKET server = connect_to_host(hostname, "25");

	wait_on_response(server, 220);
	send_format(server, "HELO HONPWC\r\n");
	wait_on_response(server, 250);
	
	char sender[MAXINPUT];
	get_input("from: ", sender);
	send_format(server, "MAIL FROM: <%s>\r\n", sender);
	wait_on_response(server, 250);

	char recipient[MAXINPUT];
	get_input("to: ", recipient);
	send_format(server, "RCPT TO: <%s>\r\n", recipient);
	wait_on_response(server, 250);

	send_format(server, "DATA\r\n");
	wait_on_response(server, 354);

	char subject[MAXINPUT];
	get_input("subject: ", subject);

	send_format(server, "From: <%s>\r\n", sender);
	send_format(server, "To: <%s>\r\n", recipient);
	send_format(server, "Subject: %s\r\n", subject);

	time_t timer;
	time(&timer);
	struct tm *timeinfo;
	timeinfo = gmtime(&timer);
	char date[128];
	strftime(date, 128, "%a, %d %b %Y %H:%M:%S +0000", timeinfo);
	send_format(server, "Date: %s\r\n", date);

	send_format(server, "\r\n");
	printf("Enter your email text, end with \".\" on a line by itself:\n");

	while (1){
		char body[MAXINPUT];
		get_input("> ", body);
		send_format(server, "%s\r\n", body);
		if (strcmp(body, ".") == 0) break;
	}

	wait_on_response(server, 250);
	send_format(server, "QUIT\r\n");
	wait_on_response(server, 221);

	printf("\n Closing socket...\n");
	CLOSESOCKET(server);

	printf("Finished.\n");
	return 0;
}

