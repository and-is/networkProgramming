#include "https_server_headers.h"

int main(){
// ssl library initialization
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();

// create SSL context object
	SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
	if (!ctx) {
		fprintf(stderr, "SSL_CTX_new() failed.\n");
		return 1;
	}

// load server certificate and private key
	if (!SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) || 
	    !SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM)) {
		fprintf(stderr, "SSL_CTX_use_certificate_file() failed.\n");
		ERR_print_errors_fp(stderr);
		return 1;
	}

// create socket
	printf("Configuring local address...\n");
	struct addrinfo hints, *bind_address;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_flags = AI_PASSIVE; 

	getaddrinfo(0, "8080", &hints, &bind_address);

	printf("Creating socket...\n");
	SOCKET socket_listen;
	socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);
	if (!ISVALIDSOCKET(socket_listen)) {
		fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
		return 1;
	}

	printf("Binding socket...to local address...\n");
	if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen)) {
		fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
		return 1;
	}
	freeaddrinfo(bind_address);

	printf("Listening...\n");
	if (listen(socket_listen, 10) < 0){
		fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
		return 1;
	}

// accept incoming connections
	while (1) {
		printf("Waiting for connection...\n");
		struct sockaddr_storage client_address;
		socklen_t client_len = sizeof(client_address);
		SOCKET socket_client = accept(socket_listen, (struct sockaddr *)&client_address, &client_len);
		if (!ISVALIDSOCKET(socket_client)) {
			fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO());
			return 1;
		}

		printf("Client is connected...\n");
		char address_buffer[100];
		getnameinfo((struct sockaddr *)&client_address, client_len, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
		printf("Client address: %s\n", address_buffer);

		SSL *ssl = SSL_new(ctx);
		if (!ssl) {
			fprintf(stderr, "SSL_new() failed.\n");
			return 1;
		}

		SSL_set_fd(ssl, socket_client);
		if (SSL_accept(ssl) <= 0) {
			fprintf(stderr, "SSL_accept() failed.\n");
			ERR_print_errors_fp(stderr);
			SSL_shutdown(ssl);
			CLOSESOCKET(socket_client);
			SSL_free(ssl);
			continue;
		}

		printf("SSL connection using %s\n", SSL_get_cipher(ssl));

		printf("Reading request...\n");
		char request[1024];
		int bytes_received = SSL_read(ssl, request, 1024);
		printf("Received %d bytes\n", bytes_received);

		printf("Sending response...\n");
		const char *response = "HTTP/1.1 200 OK\r\n"
		                       "Content-Type: text/plain\r\n"
		                       "Connection: close\r\n"
		                       "\r\n"
		                       "Local time is: ";
		int bytes_sent = SSL_write(ssl, response, strlen(response));
		printf("Sent %d bytes of %d bytes.\n", bytes_sent, (int)strlen(response));		
		time_t timer;
		time(&timer);
		char *time_msg = ctime(&timer);
		bytes_sent = SSL_write(ssl, time_msg, strlen(time_msg));
		printf("Sent %d bytes of %d bytes.\n", bytes_sent, (int)strlen(time_msg));

		printf("Closing connection...\n");
		SSL_shutdown(ssl);
		CLOSESOCKET(socket_client);
		SSL_free(ssl);
	}	

	printf("Closing listening socket...\n");
	CLOSESOCKET(socket_listen);
	SSL_CTX_free(ctx);

	printf("Finished.\n");
	return 0;
}
