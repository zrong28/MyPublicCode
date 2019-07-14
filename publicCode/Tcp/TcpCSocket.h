/**
  *TcpCSocket类是一个基于TCP通信的一个套接字类，它用于创建一个TCP通信的客户端对象。
  *
  */

#pragma once

#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H
	   	
#include "msys/CommandDeviece.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <msys/msysfunc.h>
#include <pthread.h>
#include <errno.h>

using std::string;
using std::to_string;
using std::cout;
using std::endl;
	 
typedef struct DataNode{
    string datastr;
    DataNode* next;
}DataNode;

typedef struct DataNodeHeader{
    DataNode* next;
}DataNodeHeader;

class TcpCSocket{
public:
	TcpCSocket();
	~TcpCSocket();
    bool ConnectToServer(const char * sevrip,const int _port);
			
    int SendData(const char *sendbuf,int len);
    int SendData(const char *sendbuff);
    int RecvData(char *recvbuf,int len);
    bool Close();
//    virtual void ReadDataProc(char *DataBuff,const int BuffLen);

    bool IsConnect();

    bool AddSendNode(DataNode* newnode);
    DataNode* PopTopSendNode();
    bool DelDataNodeLine();

    bool tcpsendthread;
private:
    int port;

    char addr[16];

    bool connectStatus;

	int sockfd;
	char recvbuf[RECVSIZE];

    DataNodeHeader* header;
//    pthread_t ccpid;


};
#endif
