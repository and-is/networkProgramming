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

