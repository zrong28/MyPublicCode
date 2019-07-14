#include "TcpCSocket.h"

TcpCSocket::TcpCSocket():tcpsendthread(true),connectStatus(false),port(0),sockfd(-1){

    this->header=new DataNodeHeader;
    header->next=NULL;
}

TcpCSocket::~TcpCSocket(){
    if(!Close())
	{
        cout<<"close error"<<endl;
	}
    else
        cout<<"~TcpSocket"<<endl;

    this->DelDataNodeLine();
}

/**
 * @brief TcpCSocket::ConnectToServer 连接服务器
 * @param sevrip 服务器ip
 * @param port 端口号
 * @return 成功返回true，失败返回false
 */
bool TcpCSocket::ConnectToServer(const char *sevrip,const int _port){

    if(sevrip==NULL||_port<=0)
    {
        cout<<"No have server ip or port"<<endl;
        return false;
    }

    this->port=_port;
    strcpy(this->addr,sevrip);

    if(this->sockfd>0){
        close(this->sockfd);
        sockfd=-1;
    }

    if((this->sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
    {
        perror("socket error\n");
        return false;
    }

	struct sockaddr_in sevrddr;
	sevrddr.sin_family=AF_INET;
	sevrddr.sin_addr.s_addr=inet_addr(sevrip);
	sevrddr.sin_port=htons(port);
    if(::connect(sockfd,(struct sockaddr*)&sevrddr,sizeof(sevrddr))<0)
	{
//        cout<<"connect to server faile"<<endl;
        ::close(sockfd);
		return false;
	}else{
        cout<<"connect to server successed"<<endl;
        this->connectStatus=true;
        this->tcpsendthread=true;
    }

	return true;
}

/**
 * @brief TcpCSocket::SendData 发送数据
 * @param sendbuf 需要发送的字符串
 * @param len 发送长度
 * @return 成功返回true，失败返回false
 */
int TcpCSocket::SendData(const char *sendbuf,int len)
{
	int ret=0;

    cout<<endl;
    cout<<"------Sending a data to net------"<<endl;
    ret=::send(sockfd,sendbuf,len,0);
    if(ret<0)
    {
        perror("send error:");
        return ret;
    }

    return ret;
}

int TcpCSocket::SendData(const char *sendbuff)
{

    int ret=0;

    cout<<endl;
    cout<<"------Sending a data to net------"<<endl;
    ret=::send(sockfd,sendbuff,strlen(sendbuff),0);

    if(ret<0)
    {
        perror("send error:");
        return ret;
    }

    return ret;
}

/**
 * @brief TcpCSocket::RecvData 接收数据
 * @param recvbuff 接收缓冲区
 * @param len 接收长度
 * @return 成功返回true，失败返回false
 */
int TcpCSocket::RecvData(char *recvbuff,int len)
{
    ssize_t ret=0;
    ret=::recv(sockfd,recvbuff,len,0);
    if(ret>0)
    {
        return ret;
    }

    else if(ret==0)
    {
        perror("connect end:");
        return ret;
    }
    else if(ret<0)
    {
        perror("recv error:");
        return ret;
    }

}

/**
 * @brief TcpCSocket::ReadDataProc 数据处理函数
 * @param DataBuff 需要处理的数据
 * @param BuffLen 数据长度
 */
//void TcpCSocket::ReadDataProc(char *DataBuff,const int BuffLen)
//{

//}

/**
 * @brief TcpCSocket::Close 关闭这个套接字
 * @return 成功返回true，失败返回false
 */
bool TcpCSocket::Close()
{
	if(sockfd<0)
	{
        return true;
	}
	else
	{
		::close(sockfd);
        this->connectStatus=false;
        this->tcpsendthread=false;
	}

	return true;
}

bool TcpCSocket::IsConnect()
{
    return connectStatus;
}

bool TcpCSocket::AddSendNode(DataNode *newnode)
{
    if(header==NULL){
        this->header=new DataNodeHeader;
        header->next=NULL;
    }

    DataNode* temp=NULL;
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

DataNode* TcpCSocket::PopTopSendNode(){
    if(header==NULL||header->next==NULL)
        return NULL;

    DataNode* temp=header->next;
    header->next=temp->next;

    return temp;
}

bool TcpCSocket::DelDataNodeLine()
{
    DataNode* temp=NULL;

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
