// Data struct of a command between peers
struct cmd_head{
    int  _cmd;
    int  _data_sz;
};
struct peer_cmd{
    struct cmd_head _head;  
    char _data_buf[1];
};
struct peer_ipv4{
    int _port_no;
    char _ipv4_addr[16];
};
// Command values
#define PEER_CMD_NULL 0
#define PEER_CMD_REG_DEVICE 1
#define PEER_CMD_HOST_MCAST 2
#define PEER_CMD_END_SERVER 10

// Data struct for a multicast entity, for both tx and rx
struct mcast_task{
    // Informations to be multicasted
    int  _device_type;    // type number of device
    char _lisn_ip[16];    // listening on this ip address
    int  _lisn_port;      // listening port number
    // Physical multicasting parameters
    char _mcast_grp[16];  // multicast group
    int  _mcast_port;     // multicast port number
};
