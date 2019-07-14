#include "CServSocket.h"

CServSocket::CServSocket():port(0),sockfd(-1),connsockfd(-1),maxConn(100),connectCounts(0)
{
    memset(ip,0,20);
    this->rdheader=new RdNodeHeader;
    rdheader->nextnode=NULL;
    //epoll机制参数初始化
    this->epollfd=epoll_create(28);
}

CServSocket::~CServSocket()
{
    Close();

}

void CServSocket::Close(){

    vector<clientInfo>::iterator iter=this->cliInfoVct.begin();

    for(;iter!=cliInfoVct.end();){
        if((*iter).sockfd>0){
            close((*iter).sockfd);
            iter=cliInfoVct.erase(iter);
        }else{
            iter++;
        }
    }

    if(sockfd>0)
        close(sockfd);

    if(connsockfd>0)
        close(connsockfd);
}

/**
 * @brief CServSocket::Listen 监听参数设置
 * @param sport 监听端口
 * @param clinum 监听最大数量
 * @return 如果listen操作失败，返回false，否则返回true
 */
bool CServSocket::Listen(int sport,int clinum){

    if(sport<0)
    {
        std::cout<<"the port err\n";
        return false;
    }

    sockfd=::socket(AF_INET,SOCK_STREAM,0);

    if(sockfd<0)
    {
        std::cout<<"create socket err\n";
        return false;
    }

    getLocalAddr();

    servAddr.sin_family=AF_INET;

    if(ip)
    {
        servAddr.sin_addr.s_addr=inet_addr(ip);

    }else{
        servAddr.sin_addr.s_addr=htonl(INADDR_ANY);
    }

    servAddr.sin_port=htons(sport);
    setSoLinger(false,0);

    setIntOptions(SO_REUSEADDR,-1);
    setIntOptions(SO_KEEPALIVE,1);
    setIntOptions(SO_SNDBUF,640000);    //确定发送缓冲区大小
    setIntOptions(SO_RCVBUF,640000);    //确定接收缓冲区大小

    setNoDelay(true);

    if(::bind(sockfd,(struct sockaddr*)&servAddr,sizeof(servAddr))<0)
    {
        std::cout<<"Bind err\n";
        return false;
    }

    int listenr=::listen(sockfd,clinum);
//    printf("%d",listenr);
    if(listenr<0)
    {
        perror("listen err:");
        return false;
    }

    ev.data.fd=sockfd;
    ev.events=EPOLLIN;
    epoll_ctl(this->epollfd,EPOLL_CTL_ADD,sockfd,&ev);

    return true;
}

/**
 * @brief CServSocket::Accecpt  接收单个客户端连接
 * @return
 */
bool CServSocket::Accecpt()
{
    socklen_t len = sizeof(cliAddr);
    if((connsockfd = ::accept(sockfd, (struct sockaddr*)&cliAddr, &len)) <0)
    {
        return false;
    }
    return true;
}

/**
 * @brief CServSocket::AcceptMore 用于接受客户端连接，并将连接进来客户端信息存放于一个容器中。
 * @return 本次接收到的客户端连接的套接字
 */
int CServSocket::AcceptMore()
{
    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
    int newfd = 0;

    newfd=::accept(sockfd,(struct sockaddr*)&clientaddr,&len);

    std::cout<<"Accept a client"<<std::endl;

    if(newfd<0)
    {
        perror("accept fail");
        return -1;
    }
    if(connectCounts==MAXCLIENT)
    {
        std::cout<<"Connecting counts is max,can't accept more clients again!"<<std::endl;
        return -1;
    }

    connectCounts++;

    size_t i=0;
    clientInfo newclient;

    newclient.sockfd=newfd;
    newclient.addrstr=clientaddr;
    cliInfoVct.push_back(newclient);

    ev.data.fd=newfd;
    ev.events=EPOLLIN|EPOLLET|EPOLLRDHUP;
    epoll_ctl(this->epollfd,EPOLL_CTL_ADD,newfd,&ev);

    return newfd;
}

/**
 * @brief CServSocket::RecvEpoll
 */
void CServSocket::RecvEpoll()
{
    int nfds=0;
//    usleep(10000*1);
    nfds=epoll_wait(this->epollfd,this->events,24,1000);

    for(int i=0;i<nfds;i++)
    {
/*
        std::cout<<events[i].events<<std::endl;

        switch (events[i].events) { //调试时，打印发生的事件
        case EPOLLIN:
            std::cout<<"EPOLLIN"<<std::endl;
            break;

        case EPOLLOUT:
            std::cout<<"EPOLLOUT"<<std::endl;
            break;

        case EPOLLPRI:
            std::cout<<"EPOLLPRI"<<std::endl;
            break;

        case EPOLLERR:
            std::cout<<"EPOLLERR"<<std::endl;
            break;

        case EPOLLHUP:
            std::cout<<"EPOLLHUP"<<std::endl;
            break;

        case EPOLLET:
            std::cout<<"EPOLLET"<<std::endl;
            break;

        case EPOLLONESHOT:
            std::cout<<"EPOLLONESHOT"<<std::endl;
            break;

        case EPOLLRDHUP:
            std::cout<<"EPOLLRDHUP"<<std::endl;
            break;

        case EPOLLMSG:
            std::cout<<"EPOLLMSG"<<std::endl;
            break;

        case EPOLLRDBAND:
            std::cout<<"EPOLLRDBAND"<<std::endl;
            break;

        case EPOLLRDNORM:
            std::cout<<"EPOLLRDNORM"<<std::endl;
            break;

        case EPOLLWRBAND:
            std::cout<<"EPOLLWRBAND"<<std::endl;
            break;

        case EPOLLWRNORM:
            std::cout<<"EPOLLWRNORM"<<std::endl;
            break;

        case EPOLL_CLOEXEC:
            std::cout<<"EPOLL_CLOEXEC"<<std::endl;
            break;

//        case EPOLL_NONBLOCK:
//            std::cout<<"EPOLL_NONBLOCK"<<std::endl;
//            break;

        case EBADF:
            std::cout<<"EBADF"<<std::endl;
            break;

        case EINVAL:
            std::cout<<"EINVAL"<<std::endl;
            break;

        default:
            break;
        }

        if(events[i].events==8193){
            printf("%d \n",events[i].data.fd);
        }
*/
        if(events[i].data.fd==this->sockfd){

            std::cout<<"A client want to connect"<<std::endl;
            AcceptMore();

        }
        else if(events[i].events==EPOLLRDHUP||events[i].events==8217){

            std::cout<<"A client was downline"<<std::endl;
            this->CliDownLine(events[i].data.fd);

        }
        else if(this->events[i].events==EPOLLIN)
        {
            std::cout<<"The server recv a data!"<<std::endl;

            char *buff=new char[1024];
            memset(buff,0,1024);

            int len=::recv(events[i].data.fd,buff,1024,0);

            if(len>0){
                RecvDataNode* newnode=new RecvDataNode;
                newnode->data=buff;
                newnode->len=len;
                newnode->nextnode=NULL;
                newnode->sfd=events[i].data.fd;

                if(rdheader->nextnode==NULL){
                    rdheader->nextnode=newnode;
                    continue;
                }

                RecvDataNode* tmpnode=rdheader->nextnode;

                while(tmpnode!=NULL){
                    tmpnode=tmpnode->nextnode;
                }

                tmpnode->nextnode=newnode;

            }else if(len==0){
                delete[] buff;
                buff=NULL;
            }else if(len<0){
                this->CliDownLine(events[i].data.fd);
            }
        }else{
//            std::cout<<nfds<<std::endl;
        }
    }
}

/**
 * @brief CServSocket::CliDownLine 客户端掉线处理
 * @param fd
 */
void CServSocket::CliDownLine(int fd){

    epoll_ctl(this->epollfd,EPOLL_CTL_DEL,fd,NULL);

    vector<clientInfo>::iterator cliter=cliInfoVct.begin();

    for(;cliter!=cliInfoVct.end();)
    {
        if((*cliter).sockfd==fd&&fd>0){
            std::cout<<"Closed a client connection,socket:"<<std::to_string(fd)<<std::endl;

            cliter=cliInfoVct.erase(cliter);
            if(close(fd)==-1){
                perror("Close client socket error");
            }

            this->connectCounts--;

            break;

        }else{

            cliter++;

        }
    }
}

/**
 * @brief CServSocket::PopTopRecvNode 把当前队列中最早进入队列的节点弹出
 * @return
 */
RecvDataNode* CServSocket::PopTopRecvNode(){
    if(rdheader==NULL||rdheader->nextnode==NULL){
        return NULL;
    }

    RecvDataNode* temp=rdheader->nextnode;
    rdheader->nextnode=temp->nextnode;
    return temp;
}

string CServSocket::getLocalAddr()
{
    char ipaddr[20]={'\0'};
    const char* shellstr = "ifconfig | sed -n '2p' | awk -F'[ :]+' '{printf $4}'";
    FILE *fp = popen(shellstr, "r");
    fread(ipaddr, sizeof(char), sizeof(ipaddr), fp);
    if(ipaddr)
    {
        strcpy(ip, ipaddr);
    }
//    std::cout<<std::string(ip)<<std::endl;
    pclose(fp);

    return std::string(ip);

}

string CServSocket::getCliAddr()
{
    char cliip[16];
    socklen_t size=sizeof(cliAddr);
    if(getpeername(sockfd,(sockaddr*)&cliAddr,&size)==-1)
    {
        printf("getpeername\n");
        strcpy(cliip,"0.0.0.0");
    }
    else
    {
        sprintf(cliip,"%d.%d.%d.%d",
                ((unsigned char*)&cliAddr.sin_addr)[0],
                ((unsigned char*)&cliAddr.sin_addr)[1],
                ((unsigned char*)&cliAddr.sin_addr)[2],
                ((unsigned char*)&cliAddr.sin_addr)[3]);
    }
    printf("\n");
    return string(cliip);
}

/**
 * @brief CServSocket::getCliAddr
 * @param id
 * @return
 */
string CServSocket::getCliAddr(int id)
{
    char cliip[16];

    socklen_t size=sizeof(cliAddr);

    vector<clientInfo>::iterator cliter=cliInfoVct.begin();

    for(;cliter!=cliInfoVct.end();cliter++)
    {
        if((*cliter).id==id)
        {
            struct sockaddr_in cliaddress=(*cliter).addrstr;

            if(getpeername(sockfd,(sockaddr*)&cliaddress,&size))
            {
                strcpy(cliip,"0.0.0.0");
            }
            else
            {
                sprintf(cliip,"%d.%d.%d.%d",
                        ((unsigned char*)&cliaddress.sin_addr)[0],
                        ((unsigned char*)&cliaddress.sin_addr)[1],
                        ((unsigned char*)&cliaddress.sin_addr)[2],
                        ((unsigned char*)&cliaddress.sin_addr)[3]);
            }
            return string(cliip);
        }
    }
    strcpy(cliip,"0.0.0.0");
    return string(cliip);
}

/**
 * @brief CServSocket::getCliAddr
 * @param addrin
 * @return
 */
string CServSocket::getCliAddr(sockaddr_in addrin)
{
    char cliip[16];
    socklen_t size=sizeof(addrin);
    if(getpeername(sockfd,(sockaddr*)&cliAddr,&size))
    {
        strcpy(cliip,"0.0.0.0");
    }
    else
    {
        sprintf(cliip,"%d.%d.%d.%d",
                ((unsigned char*)&cliAddr.sin_addr)[0],
                ((unsigned char*)&cliAddr.sin_addr)[1],
                ((unsigned char*)&cliAddr.sin_addr)[2],
                ((unsigned char*)&cliAddr.sin_addr)[3]);
    }
    return string(cliip);
}

/**
 * @brief CServSocket::getAcceptSock
 * @return
 */
int CServSocket::getAcceptSock()
{
    return connsockfd;
}

/**
 * @brief CServSocket::setIntOptions
 * @param option
 * @param value
 * @return
 */
bool CServSocket::setIntOptions(int option, int value)
{
    bool res=false;
    if(sockfd)
    {
        res=(setsockopt(sockfd,SOL_SOCKET,option,(const char*)&value,sizeof(value))==0);
    }
    return res;
}

/**
 * @brief CServSocket::setSoLinger
 * @param dolinger
 * @param seconds
 * @return
 */
bool CServSocket::setSoLinger(bool dolinger, int seconds)
{
    bool res=false;
    if(sockfd){
        struct linger ling;
        ling.l_onoff=dolinger?1:0;
        ling.l_linger=seconds;
        res=(setsockopt(sockfd,SOL_SOCKET,SO_LINGER,(const char*)&ling,sizeof(struct linger))==0);
        res=true;
    }
    return res;
}

bool CServSocket::setTimeOut(int option, int milliseconds)
{
    bool res=false;

    if(sockfd)
    {
        struct timeval timeout;
        timeout.tv_sec=milliseconds/1000;
        timeout.tv_usec=(milliseconds%1000)*1000000;
        res=(setsockopt(sockfd,SOL_SOCKET,option,(const void *)&timeout,sizeof(timeout))==0);
        res =true;
    }

    return res;
}

bool CServSocket::setNonBlock(bool isnonsocket)
{
    bool res=false;

    if(isnonsocket&&sockfd)
    {
        int oldfd=fcntl(sockfd,F_GETFL);
        res=(fcntl(sockfd,F_SETFL,oldfd|O_NONBLOCK)<0);
        res=true;
    }

    return res;
}

bool CServSocket::setNoDelay(bool nodelay)
{
    bool res=false;
    if(sockfd)
    {
        int ndelay=nodelay?1:0;
        res=(setsockopt(sockfd,IPPROTO_TCP,TCP_NODELAY,(const void*)&nodelay,sizeof(ndelay))==0);
        res=true;
    }
    return res;
}

bool CServSocket::Send(int sfd,char *sendbuff, int len)
{
    if(sockfd<0||sendbuff==NULL||len<0)
        return false;

    int dataleft=len,total=0,ret=0;
    for(;dataleft>0;)
    {
        ret=::send(sfd,sendbuff+total,dataleft,0);
        if(ret<0)
        {
            if(errno==EAGAIN||errno==EWOULDBLOCK||errno==EINTR)
            {
                usleep(50000);
                ret=0;
            }
        }
        total+=ret;
        dataleft=len-total;
    }
    return total==len;
}

size_t CServSocket::Recv(char *recvbuf, int len, int timeout)
{
    if(sockfd <0 || recvbuf==NULL || len < 0)
        return false;

    fd_set fds;
    struct timeval interval;
    interval.tv_sec = timeout;
    interval.tv_usec = 0;
    int recvlen = 0;

    for(;;){

        FD_ZERO(&fds);
        FD_SET(connsockfd, &fds);

        int res = ::select(connsockfd+1, &fds, NULL, NULL, &interval);

        if(res == 0)
            continue;

        if(res < 0){
            ::close(connsockfd);
            connsockfd = -1;
            return -1;
        }else{
            if(FD_ISSET(connsockfd, &fds))
            {
                recvlen = ::recv(connsockfd, recvbuf, len, 0);
                break;
            }
        }

    }
    return recvlen;
}

size_t CServSocket::RecvAddpoitSockfd(char *recvbuff, int len, int sockfd)
{
    return 0;
}


bool CServSocket::AddSendNode(SendDataNode *newnode)
{
    if(header==NULL){
        this->header=new SendDataHeader;
        header->next=NULL;
    }

    newnode->next=NULL;

    SendDataNode* temp=NULL;
    if(header->next!=NULL){
        temp=header->next;
        while(1)
        {
            if(temp->next!=NULL){
                temp=temp->next;
            }
            else{
                temp->next=newnode;
                break;
            }
        }
    }
    else{
        header->next=newnode;
    }
    temp=NULL;

    return true;
}

SendDataNode* CServSocket::PopTopSendNode(){
    if(header==NULL||header->next==NULL)
        return NULL;

    SendDataNode* temp=header->next;
    header->next=temp->next;

    return temp;
}

bool CServSocket::DelSendNodeLine()
{
    SendDataNode* temp=NULL;

    while(1)
    {
        if(header->next!=NULL){
            temp=header->next;
            delete temp;
        }
        else{
            delete header;
            break;
        }
    }
    header=NULL;
    temp=NULL;
    return true;
}


/**
 * @brief CabinetServer::AcceptHandle
 * @param arg
 * @return
 */
void* CServSocket::AcceptHandle(void *arg)
{
    CServSocket* cserv=(CServSocket*)arg;
    while(1)
    {
        cserv->AcceptMore();
    }
    pthread_exit(0);
}
