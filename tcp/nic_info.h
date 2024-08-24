#define CMD_PREP_RX_BUF   0
#define CMD_RX_DATA_READY 1
#define CMD_PREP_TX_DATA  2
#define CMD_TX_DATA_DONE  3
struct tcp_task{
    char _ipv4_ip[16];  // host's(server) ipv4 address
    int(*_handler)(int,char**,int*); // pointer to a handler function at the caller level
    int  _port_no;      // host's(server) port number,
    int  _filler;
};