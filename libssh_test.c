#include "ssh_headers.h"

int main(){
	printf("libssh version: %s\n", ssh_version(0));
	return 0;
}
