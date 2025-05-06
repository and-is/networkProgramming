#include "ssh_headers.h"

int main(int argc, char *argv[]){
	const char *hostname = 0;
	int port = 22;
	if (argc < 2) {
		fprintf(stderr, "Usage: ssh_connect hostname port\n");
		return 1;
	}
	hostname = argv[1];
	if (argc > 2) {
		port = atol(argv[2]);
	}
// hostname and port resolved, now on to creating an ssh session object 
	
	ssh_session ssh = ssh_new();
	if (!ssh){
		fprintf(stderr, "ssh_new() failed\n");
		return 1;
	}

// set the options for the ssh session
	ssh_options_set(ssh, SSH_OPTIONS_HOST, hostname);
	ssh_options_set(ssh, SSH_OPTIONS_PORT, &port);
	
	int verbosity = SSH_LOG_PROTOCOL;
	ssh_options_set(ssh, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
// this enables libssh to print everything it does, log all info for debugging
	
// initiate ssh connection
	int ret = ssh_connect(ssh);
	if (ret != SSH_OK) {
		fprintf(stderr, "ssh_connect() failed: %s\n", ssh_get_error(ssh));
		return 1;
	}

	printf("Connected to %s on port %d.\n", hostname, port);

// ssh protocol allows servers to send a message to clients upon connecting called banner
	printf("Banner:\n%s\n", ssh_get_serverbanner(ssh));

// this much for the connection, now disconnecting

	ssh_disconnect(ssh);
	ssh_free(ssh);
	return 0;
}

