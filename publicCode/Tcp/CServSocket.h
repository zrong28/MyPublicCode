/**
  *CServSocket类是一个基于TCP通信的一个套接字类，它用于创建一个TCP通信的服务端对象。
  *
  */

#ifndef CSERVSOCKET_H
#define CSERVSOCKET_H
#include "msys/CommandDeviece.h"
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <netinet/tcp.h>
#include <string>

#include <pthread.h>
#include <vector>

#define MAXCLIENT 128

using std::vector;
using std::iterator;
using std::string;

typedef struct SendDataNode{
    string datastr;
    SendDataNode* next;
    int destsfd;
}SendDataNode;

typedef struct SendDataHeader{
    SendDataNode* next;
}SendDataHeader;


typedef struct RecvDataNode{
    int len;
    char *data;
    int sfd;
    RecvDataNode* nextnode;
}RecvDataNode;

typedef struct RdNodeHeader{
    RecvDataNode* nextnode;
}RdNodeHeader;

class CServSocket
{
    typedef struct clientInfo{
        int id;
        int sockfd;
        struct sockaddr_in addrstr;
    }clientInfo;

public:
    bool AddSendNode(SendDataNode* newnode);
    SendDataNode* PopTopSendNode();
    bool DelSendNodeLine();

    virtual void RecvEpoll();
    RecvDataNode* PopTopRecvNode();

    CServSocket();
    virtual ~CServSocket();
    string getLocalAddr();
    string getCliAddr();
    string getCliAddr(int id);
    string getCliAddr(struct sockaddr_in addrin);

    virtual void CliDownLine(int fd);

    int getAcceptSock();
    bool Accecpt();
    virtual int AcceptMore();


    bool Listen(int sport,int clinum);
    virtual bool Send(int sfd,char *sendbuff,int len);
    virtual size_t Recv(char *recvbuff,int len,int timeout);
    virtual size_t RecvAddpoitSockfd(char *recvbuff, int len, int sockfd);

protected:
    uint maxConn;
    char ip[20];
    int port;
    int sockfd;
    int connectCounts;
    int connsockfd;

    vector<clientInfo> cliInfoVct;
    RdNodeHeader* rdheader;

    void Close();
    bool setSoLinger(bool dolinger,int seconds);
    bool setIntOptions(int option,int value);
    bool setTimeOut(int option,int milliseconds);
    bool setNonBlock(bool isnonsocket);
    bool setNoDelay(bool nodelay);

    struct sockaddr_in servAddr,cliAddr;
    SendDataHeader* header;

    int epollfd;
    struct epoll_event ev,events[24];

    static void* AcceptHandle(void *arg);
};

#endif // CSERVSOCKET_H
