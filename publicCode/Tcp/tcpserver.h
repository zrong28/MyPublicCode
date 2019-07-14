#ifndef TCPSERVER_H
#define TCPSERVER_H
#include "msys/CommandDeviece.h"

#include <string>
#include<map>

using std::map;
using std::iterator;
using std::string;

class TcpServer
{
    typedef string IPADDR_STRING;
    typedef uint PORT_UINT;

public:
    TcpServer();
    virtual ~TcpServer();
    bool TcpServerInit(uint servport);

    bool TcpServerStart();

    bool Write(const char *wbuff,size_t len);
    bool Read(char *rbuff);

    bool GetNetStatus();

    bool CloseTcpServer();
private:
    static void *TcpServerHandle(void *arg);

    map<IPADDR_STRING,PORT_UINT> clientmap;
    int connectCount;
    int socket;
    PORT_UINT port;
    IPADDR_STRING ip_addr;

    bool NetStatus;
};

#endif // TCPSERVER_H
