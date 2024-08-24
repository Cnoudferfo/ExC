// Command values, from tcp interface functions to Caller handler
#define CMD_PREP_RX_BUF   0
#define CMD_RX_DATA_READY 1
#define CMD_PREP_TX_DATA  2
#define CMD_TX_DATA_DONE  3
// Data struct for a tcp entity, for both server & client
struct tcp_task{
    char _ipv4_ip[16];  // host's(server) ipv4 address
    int(*_handler)(int, char**,int*); // pointer to a handler function at the caller level
    int  _port_no;      // host's(server) port number,
    int  _filler;       // not yet useful
};
// TCP server flags
#define FLAG_ONE_TIME 0
#define FLAG_FOREVER  1

// Data struct for a multicast entity, for both tx and rx
struct mcast_task{
    char _ipv4_ip[16];    // ipv4 address to bind to
    char _mcast_grp[16];  // multicast group
    int(*_handler)(int, char**,int*); // pointer to a handler function at the caller level
    int  _mcast_port;        // multicast port number
    int  _lisn_port;         // listening port number
};

// Yet another socket interface data
struct socket_info{
    union {
        char _nic_addr[16];
        char _mcast_grp[16];
    } _ipv4;
    int  _port_no;
    int  _filler;
    int(*_handler)(int, char**, int*);
};

struct heap_mcast_rx{  // Work space
    SOCKET _sock;  // socket's id
    struct sockaddr_in _localAddr;  // bind to this interface
    struct ip_mreq _mcastReq;  // mcasr req
    char *_buf;      // buffer
    int   _bufLen;  // buffer length
    int   _addrLen; // local_addr's length
    int   _nbytes;   // number of bytes received
    int   _iret;     // returned int
};

struct heap_mcast_tx{ // Work space
    SOCKET _sock;
    struct sockaddr_in _mcastAddr;
    char *_buf;
    int   _bufLen;
    int   _iret;
};

struct heap_tcp_client{  // Work space
    SOCKET _connectSock;
    struct addrinfo _svr_hints;
    struct addrinfo *_p_svr_result;
    struct addrinfo *_p_ptr;  // For for loop usage
    char _portStr[8];
    char *_buf;
    int _bufLen;
    int _nbytes;
    int _iret;
};

struct heap_tcp_server{ // Work space
    SOCKET _listenSock;
    SOCKET _clientSock;
    struct sockaddr_in _localAddr;
    int _flag;  // input flag
    char *_buf;
    int  _bufLen;
    int  _nbytes;
    int  _iret;
};

struct a_sock{
    int(*_IF_handler)(int, char**, int*);   // Peer(centers & devices)s' interface with socket tools(mcast, tcp)...
    union {     // Work space of ech kind of socket tool
        struct heap_tcp_server _tcp_svr;
        struct heap_tcp_client _tcp_cli;
        struct heap_mcast_tx   _mcast_tx;
        struct heap_mcast_rx   _mcast_rx;
    } _heap;
    struct nic_info {   // Nic information to start a socket tool
        union {
            char _ip_addr[16];
            char _mcast_grp[16];
        } _ipv4;
        int _portNo;
    } _nic_info;
};