# Learning to Code

### Byte Order
- Big Endian: Most significant byte first.
- Little Endian: Least significant byte first.

Network Bytes are transmitted in Big Endian format called the Network Byte Order.
Host Byte Order is how they are stored in the host machine, which can be either Big Endian or Little Endian.
Numbers are converted to Network Byte Order before they go out on the wire and back to Host Byte Order when they are received.

Interconversion done through these functions:
```
htons() -> host to network short
htonl() -> host to network long
ntohs() -> network to host short
ntohl() -> network to host long
```

### Data Types
1. a socket descriptor (int)
2. a socket address structure `struct addrinfo`
A call to getaddrinfo() returns a linked list of `struct addrinfo` structures, which contain information about the address family, socket type, protocol, and address.
   ```
    struct addrinfo {
    int ai_flags;          // AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST
    int ai_family;        // AF_INET, AF_INET6, AF_UNSPEC
    int ai_socktype;      // SOCK_STREAM, SOCK_DGRAM, SOCK_RAW
    int ai_protocol;      // 0 for any
    size_t ai_addrlen; // length of ai_addr in bytes
    struct sockaddr *ai_addr; // struct sockaddr_in or sockaddr_in6
    char *ai_canonname; // canonical name for the host
    struct addrinfo *ai_next; // next in linked list
    };
    ``` 
3. a socket address structure
`struct sockaddr`
```
   struct sockaddr {
    unsigned short int sa_family; // address family (AF_INET, AF_INET6)
    char sa_data[14]; // 14 bytes of protocol address, contains destination address and port number
}
```
4. a socket address structure `struct sockaddr` \
same as no. 3 but specifically for IPv4 addresses
   ```
    struct sockaddr_in {
    short int sin_family; // address family (AF_INET)
    unsigned short int sin_port; // port number
    struct in_addr sin_addr; // internet address
    unsigned char sin_zero[8]; // padding to make the structure the same size as struct sockaddr
    };
    struct in_add{
    uint32_t s_addr; // IPv4 address in Network Byte Order
    };

    
   ```
5. a socket address structure `struct sockaddr_in6`
```  
    struct sockaddr_in6 {
        u_int16_t sin6_family; // address family, AF_INET6
        u_int16_t sin6_port; // port number, Network Byte Order
        u_int32_t sin6_flowinfo; // IPv6 flow information
        struct in6_addr sin6_addr; // IPv6 address
        u_int32_t sin6_scope_id; // Scope ID
    };
    struct in6_addr {
        unsigned char
    };
 ```
6. a structure large enough to hold both ipv4 and ipv6 `struct sockaddr_storage`
```
    struct sockaddr_storage {
        sa_family_t ss_family; // address family
        // all this is padding, implementation specific, ignore it:
        char __ss_pad1[_SS_PAD1SIZE];
        int64_t __ss_align;
        char __ss_pad2[_SS_PAD2SIZE];
    };
```
### Conversion Functions
1. `inet_pton()` 
- converts an IP address in its standard form (presentation) into binary form (network).
- `int inet_pton(int af, const char *src, void *dst);`
   - af: address family (`AF_INET` for IPv4, `AF_INET6` for IPv6)
   - src: pointer to the null-terminated string containing the IP address in presentation format
   - dst: pointer to a buffer where the function will store the binary representation of the IP address
   - returns 1 on success, 0 if the input is not a valid IP address, and -1 if an error occurs.
2. `inet_ntop()`
- converts an IP address from its binary form (network) to its standard form (presentation).
- `const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);`
   - af: address family (`AF_INET` for IPv4, `AF_INET6` for IPv6)
   - src: pointer to the buffer containing the binary representation of the IP address
   - dst: pointer to a buffer where the function will store the null-terminated string containing the IP address in presentation format
   - size: size of the buffer pointed to by dst, can use `INET_ADDRSTRLEN` for IPv4 and `INET6_ADDRSTRLEN` for IPv6
   - returns a pointer to the destination buffer (dst) on success, or NULL if an error occurs.

### System Calls
1. `getaddrinfo()`
- DNS and service name lookups.
- `int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);`
   - node: hostname or IP address to be resolved, can be NULL if service is specified and `AI_PASSIVE` flag is set in hints
   - service: service name or port number to be resolved, can be NULL if node is specified
   - hints: pointer to a struct addrinfo that specifies criteria for selecting the socket address structures returned in res
   - res: pointer to a pointer that will point to a linked list of struct addrinfo structures containing the results of the lookup
   - returns 0 on success, or a non-zero error code on failure.

2. `socket()`
- creates a new socket.
- `int socket(int domain, int type, int protocol)`
- domain is `PF_INET` for IPv4, `PF_INET6` for IPv6
- type is `SOCK_STREAM` for TCP, `SOCK_DGRAM` for UDP
- protocol is usually set to 0 to select the default protocol for the given domain and type, can be TCP or UDP.
- returns a socket descriptor (a small integer) on success, or -1 on failure.

3. `bind()`
- assigns a local port address to a socket to listen for incoming connections or datagrams.- `int bind(int sockfd, struct sockaddr *addr, int addrlen);`
   - sockfd: socket descriptor returned by socket()
   - addr: pointer to a struct sockaddr containing the address to bind to (your address and port)
   - addrlen: length of the address structure pointed to by addr
   - returns 0 on success, or -1 on failure.

4. `connect()`
- establishes a connection to a remote socket.
- `int connect(int sockfd, struct sockaddr *addr, int addrlen);`
   - sockfd: socket descriptor returned by socket()
   - addr: pointer to a struct sockaddr containing the address of the remote socket to connect to (server's address and port)
   - addrlen: length of the address structure pointed to by addr
   - returns 0 on success, or -1 on failure.

5. `listen()`
- wait for incoming connections on a socket.
- int listen(int sockfd, int backlog);
   - sockfd: socket descriptor returned by socket() and bound to a local address using bind()
   - backlog: maximum number of pending connections that can be queued up before connections are refused. connections keep on being queued until they are `accept()`ed.
   - returns 0 on success, or -1 on failure.

6. `accept()`
- accepts an incoming connection on a listening socket.
- `int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);`
   - sockfd: socket descriptor returned by socket() and put into listening state using listen()
   - addr: pointer to a struct sockaddr that will be filled in with the address of the connecting client
   - addrlen: pointer to a `socklen_t` variable that initially contains the size of the buffer pointed to by addr, and will be updated to contain the actual size of the address returned
   - returns a new socket descriptor for the accepted connection on success, or -1 on failure. This is ready to send and receive data while the original keeps listening.

7. `send()` and `recv()`
- used to send and receive data on a connected socket.
- these are blocking calls by default, meaning they will wait until data is sent or received before returning.
- `ssize_t send(int sockfd, const void *buf, int len, int flags);`
   - sockfd: socket descriptor returned by socket() or accept()
   - buf: pointer to the buffer containing the data to be sent
   - len: length of the data in bytes
   - flags: usually set to 0 for default behavior
   - returns the number of bytes sent on success, or -1 on failure.
- `ssize_t recv(int sockfd, void *buf, int len, int flags);`
   - sockfd: socket descriptor returned by socket() or accept()
   - buf: pointer to the buffer where the received data will be stored
   - len: length of the buffer in bytes
   - flags: usually set to 0 for default behavior
   - returns the number of bytes received on success, 0 if the connection has been closed, or -1 on failure.

8. `sendto()` and `recvfrom()`
- used to send and receive data on a connectionless socket (UDP).
- since, datagram sockets aren't connected to a specific remote address, the destination/source address must be specified with each call.
- `int sendto(int sockfd, const void *buf, int len, unsigned int flags, const struct sockaddr *dest_addr, socklen_t addrlen);`
   - sockfd: socket descriptor returned by socket()
   - buf: pointer to the buffer containing the data to be sent
   - len: length of the data in bytes
   - flags: usually set to 0 for default behavior
   - `dest_addr`: pointer to a struct sockaddr containing the address of the destination
   - addrlen: length of the address structure pointed to by `dest_addr`
   - returns the number of bytes sent on success, or -1 on failure.
- `int recvfrom(int sockfd, void *buf, int len, unsigned int flags, struct sockaddr *src_addr, socklen_t *addrlen);`
    - sockfd: socket descriptor returned by socket()
    - buf: pointer to the buffer where the received data will be stored
    - len: length of the buffer in bytes
    - flags: usually set to 0 for default behavior
    - `src_addr`: pointer to a struct sockaddr that will be filled in with the address of the sender
    - addrlen: pointer to a `socklen_t` variable that initially contains the size of the buffer pointed to by `src_addr`, and will be updated to contain the actual size of the address returned
    - returns the number of bytes received on success, or -1 on failure.

9. `close()` and `shutdown()`
- used to close a socket when it is no longer needed.
- shutdown allows controlled closing of a socket.
- `int shutdown(int sockfd, int how);`
   - sockfd: socket descriptor returned by socket(), accept(), etc.
    - how: specifies how the socket should be shut down:
      - `SHUT_RD` or 0: Disables further receive operations.
      - `SHUT_WR` or 1: Disables further send operations.
      - `SHUT_RDWR` or 2: Disables both send and receive operations.
    - returns 0 on success, or -1 on failure.
- shutdown doesn't actually close the file descriptor, only changes its usability. Need to use `close()` to free up the descriptor.

10. `getpeername()`
- retrieves the address of the peer connected to a socket.
- `int getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen);`
   - sockfd: socket descriptor returned by socket(), accept(), etc.
   - addr: pointer to a struct sockaddr that will be filled in with the address of the connected peer
   - addrlen: pointer to a `socklen_t` variable that initially contains the size of the buffer pointed to by addr, and will be updated to contain the actual size of the address returned
   - returns 0 on success, or -1 on failure.

11. `gethostname()`
- retrieves the hostname of the current machine.
- can be used to `getaddrinfo()` to determine the IP of your local machine.
- `int gethostname(char *name, size_t len);`
   - name: pointer to a buffer where the hostname will be stored
   - len: length of the buffer in bytes
   - returns 0 on success, or -1 on failure.

### Client-Server
Server listens on a port waiting for clients to connect.
Client-Server pairs are telnet/telnetd, ftp/ftpd, Firefox/Apache and so on.
There will be one server on a machine and the server will handle multiple clients using `fork()` a child process to handle the incoming connection while the parent goes back to listening for new connections. 



