#include "tcpserver.h"

TcpServer::TcpServer()
{

}

TcpServer::~TcpServer()
{

}

bool TcpServer::TcpServerInit(uint servport){
    this->port=servport;
}

