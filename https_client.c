#include "https_headers.h"

int main(int argc, char *argv[]){
	
// initialize OpenSSL library, add algorithms and load error strings
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();

// create a new SSL_CTX object as a framework for TLS/SSL
	SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
	if (!ctx){
		fprintf(stderr, "SSL_CTX_new failed\n");
		return 1;
	}

// certificate verification not included for now
	
	if (argc < 3){
		fprintf(stderr, "Usage: https_simple hostname port\n");
		return 1;
	}

	char *hostname = argv[1];
	char *port = argv[2];

	printf("Configuring remote address...\n");
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));	
	hints.ai_socktype = SOCK_STREAM;
	struct addrinfo *peer_address;
	if (getaddrinfo(hostname, port, &hints, &peer_address) != 0){
		fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
		exit(1);
	}

	printf("Remote address is: ");
	char address_buffer[100];
	char service_buffer[100];
	getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen, address_buffer, sizeof(address_buffer), service_buffer, sizeof(service_buffer), NI_NUMERICHOST);
	printf("%s:%s\n", address_buffer, service_buffer);
	printf("Creating socket...\n");
	SOCKET server;
	server = socket(peer_address->ai_family, peer_address->ai_socktype, peer_address->ai_protocol);
	if (!ISVALIDSOCKET(server)){
		fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
		exit(1);
	}

	printf("Connecting to server...\n");
	if (connect(server, peer_address->ai_addr, peer_address->ai_addrlen)){
		fprintf(stderr, "connect() failed. (%d)\n", GETSOCKETERRNO());
		exit(1);
	}
	freeaddrinfo(peer_address);
	printf("Connected to server.\n");

// TCP done, now onto TLS/SSL
	SSL *ssl = SSL_new(ctx);
	if (!ssl){
		fprintf(stderr, "SSL_new failed.\n");
		return 1;
	}

// For SNI if connecting to a server hosting multiple domains.
	if (!SSL_set_tlsext_host_name(ssl, hostname)){
		fprintf(stderr, "SSL_set_tlsext_host_name() failed.\n");
		ERR_print_errors_fp(stderr);
		return 1;
	}
	SSL_set_fd(ssl, server);
	if (SSL_connect(ssl) == -1){
		fprintf(stderr, "SSL_connect() failed.\n");
		ERR_print_errors_fp(stderr);
		return 1;
	}

	printf("SSL/TLS using %s\n", SSL_get_cipher(ssl));

// Getting server certificate
	X509 *cert = SSL_get_peer_certificate(ssl);
	if (!cert) {
		fprintf(stderr, "SSL_get_peer_certificate() failed.\n");
		return 1;
	}
	char *tmp;
	if ((tmp = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0))) {
		printf("subject: %s\n", tmp);
		OPENSSL_free(tmp);
	}
	if ((tmp = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0))) {
		printf("issuer: %s\n", tmp);
		OPENSSL_free(tmp);
	}
	X509_free(cert);

// send the https request now

	char buffer[2048];
        sprintf(buffer, "GET / HTTP/1.1\r\n");
	sprintf(buffer + strlen(buffer), "Host: %s:%s\r\n", hostname, port);
	sprintf(buffer + strlen(buffer), "Connection: close\r\n");
	sprintf(buffer + strlen(buffer), "User-Agent: https_simple\r\n");	
	sprintf(buffer + strlen(buffer), "\r\n");

	SSL_write(ssl, buffer, strlen(buffer));
	printf("Sent Headers:\n%s", buffer);

// read the response
	while(1){
		int bytes = SSL_read(ssl, buffer, sizeof(buffer) - 1);
		if (bytes < 1){
			printf("\nConnection closed by peer.\n");
			break;
		}
		printf("Received (%d bytes): '%.*s'\n", bytes, bytes, buffer);
	}

	printf("\nClosing socket...\n");
	SSL_shutdown(ssl);
	CLOSESOCKET(server);
	SSL_free(ssl);
	SSL_CTX_free(ctx);

	printf("Finished.\n");
	return 0;
}
