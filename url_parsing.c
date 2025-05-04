#include "httpclient_headers.h"

void parse_url(char *url, char **hostname, char **port, char **path);

void parse_url(char *url, char **hostname, char **port, char **path) {
	printf("URL: %s\n", url);

	char *p;
	p = strstr(url, "://");
// strstr() searches for :// in the url, if not found p = 0,
// else p points to one after ://

	char *protocol = 0;
	if (p) {
		protocol = url;
// protocol points to beginning of the url here.

		*p = 0;
		p += 3;	
	} else {
		p = url;
	}

	if (protocol) {
		if (strcmp(protocol, "http")){
			fprintf(stderr, "Unknown protocol '%s'. Only 'http' is supported.\n", protocol);
			exit(1);
		}
	}

// extracting hostname	
	*hostname = p;
	while (*p && *p != ':' && *p != '/' && *p != '#') ++p;

// extracting port
	*port = "80";
	if (*p == ':') {
		*p++ = 0;
		*port = p;
	}
	while (*p && *p != '/' && *p != '#') ++p;

// extracting path
	*path = p;
	if (*p == '/') {
		*path = p + 1;
	}
	*p = 0;

// terminating hash because it isn't send to the server
	while (*p && *p != '#') ++p;
	if (*p == '#') *p = 0;

}

int main(){
    	char url[] = "http://example.com:8080/path/to/resource";
    	char *hostname = NULL;
    	char *port = NULL;
    	char *path = NULL;

    	parse_url(url, &hostname, &port, &path);
    	printf("Parsed Result:\n");
    	printf("Hostname: %s\n", hostname);
    	printf("Port: %s\n", port);
    	printf("Path: %s\n", path);
    	return 0;
}


