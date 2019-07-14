#include "ICom.h"

using namespace std;



ICom::ICom(){
	
    ConnectDeviceType=OTHERDEVICE;

	this->fd=0;

    this->SlaveNum=0;
    this->pthreadflag=true;
    this->ThreadCounts=0;

//    sinfo=(struct SerialInfo*)malloc(sizeof(struct SerialInfo)+1);

//    bzero(sinfo,sizeof(SerialInfo));
    sinfo=new SerialInfo;

    this->RecvBuff=(char *)std::malloc(100*sizeof(char));
    pthread_mutex_init(&mutex_lock,NULL);
}

/***************
功能：析构函数
参数：无
返回值；无
***************/
ICom::~ICom(){
	ComClose();

    free(RecvBuff);
    RecvBuff=NULL;

//    free(sinfo);
//    sinfo=NULL;
    delete sinfo;

    std::cout<<"～ICom"<<std::endl;
}

/*******************
功能：根据端口号打开对应的串口，并穿件数据处理线程
参数：const int cnum:串口号
返回值：0则失败，否则返回1；
*******************/
bool ICom::OpenCom(comNumber cnum)
{
	char pathname[20]={0};

    if(((cnum<0)||cnum>6))
	{
		printf("the port is out range");
        return false;
	}

#if ZLG_IMX
    sprintf(pathname,"/dev/ttySP%d",(int)cnum);
#elif GEC6818
    sprintf(pathname,"/dev/ttySAC%d",(int)cnum);
#elif IOT9608
    sprintf(pathname,"/dev/ttyO%d",(int)cnum);
#endif

    if(fd!=0)
	{
		printf("com is busying!\n");
        return false;
	}
	
    fd=open(pathname,O_RDWR|O_NOCTTY);

	if(fd<0)
	{
		printf("Can 't open serial port");
        return false;
	}

	if(isatty(fd)==0)
	{
		printf("isatty is not a terminal device");
        return false;
	}

    this->sinfo->comNum=(comNumber)cnum;

	return true;

}

/**********************
 * @功能：配置串口参数
 *
 * @参数：
 * 1)const int Baute_rate 波特率，
 * 2)const int Data_bits 数据位，
 * 3)char Parity 校验位，
 * 4)const int Stop_bits停止位
 *
 * @返回值：失败为0，否则为1
 *
 **********************/
bool ICom::SetComParam(_BaudRate Baud_rate,const int Data_bits,_ParityType Parity,
                       const int Stop_bits,const int slNum)
{	
	bzero(&newtio,sizeof(newtio));
	
	newtio.c_cflag|=CLOCAL|CREAD;
	newtio.c_cflag&=~CSIZE;

    cfsetispeed(&newtio,Baud_rate);
    cfsetospeed(&newtio,Baud_rate);
	
	switch(Data_bits) //设置数据位
	{
		case 5:
			newtio.c_cflag|=CS5;
			break;
		case 6:
			newtio.c_cflag|=CS6;
			break;
		case 7:
			newtio.c_cflag|=CS7;
			break;
		case 8:
			newtio.c_cflag|=CS8;
            std::cout<<"Data_bits:8 "<<std::endl;
			break;
        case NULL:
            break;
		default:
			printf("Unsupported Data_bits\n");
            return false;
		
	}
	
	switch(Stop_bits) //设置停止位 1or2
	{ 
		case 1:
			newtio.c_cflag&=~CSTOPB;
            std::cout<<"Stop_bits:1 "<<std::endl;
			break;
		case 2:
			newtio.c_cflag|=CSTOPB;
			break;
        case NULL:
            break;
		default:
			printf("Unsupported Stop_bits\n");
            return false;
	}
	
	switch(Parity)//设置校验位
	{
		default:    

		//无校验
		case 'N':      
			newtio.c_cflag&=~PARENB;
			newtio.c_iflag&=~INPCK;
			printf("Parity:None \n");
			break;
			
		//进行奇校验
		case 'O':
			newtio.c_cflag|=(PARODD|PARENB);
			newtio.c_iflag|=INPCK;
            printf("Parity:Odd \n");
			break;
		
		//进行偶校验
		case 'E':
			newtio.c_cflag|=PARENB;
			newtio.c_cflag&=~PARODD;
            printf("Parity:Even \n");
			break;
			
        //校验总和为0
		case 'S':
			newtio.c_cflag&=~PARENB;
			newtio.c_cflag&=~CSTOPB;
            printf("Parity:Space \n");
			break;
	}
	
	//设置最少字符和等待时间，对于接受字符和等待时间没有特别的要求时可以设为0

    newtio.c_cc[VTIME]=1; //当不使用O_NODELAY的时候，可以使用该参数调整串口读取的阻塞时间
    newtio.c_cc[VMIN]=0;//VMIN:当通道中的可读字符少于成c_cc[VMIM],read()就会一直阻塞
	
	//刷清输入和输出队列
	tcflush(0,TCIOFLUSH);
	
	//激活配置，TCSANOW表示更改后立即生效
	if((tcsetattr(fd,TCSANOW,&newtio))!=0)
	{
		printf("Com set error\n");
        return false;
	}
	
    sinfo->baudRate=Baud_rate;
    sinfo->dataBit=Data_bits;
    sinfo->parity=Parity;
    sinfo->stopBit=Stop_bits;
    sinfo->slaveCount=slNum;

    return true;
}

/*************
功能：串口数据接收线程函数
参数：对象指针
返回值：无
**************/
/***
void *ICom::ReadThreadFunction(void *arg){
	printf("------Starting Serial ReadTread------\n");
	ICom *com=(ICom*)arg;
	
	//epoll设置
	com->event.data.fd=com->fd;
	com->event.events=EPOLLET|EPOLLIN;
	if(epoll_ctl(com->epid,EPOLL_CTL_ADD,com->fd,&com->event)!=0)
	{
		printf("set com's epoll error!\n");
		return NULL;
	}
	
	printf("-------set epoll ok!------\n");
	
	//下面开始epoll等待
	int i=0,waitNum=0;
	while(true){
		
		waitNum=epoll_wait(com->epid,com->events,3,-1);
		printf("EventNum:%d \n",waitNum);
		for(i=0;i<waitNum;i++)
		{
			if(com->events[i].events&EPOLLIN)
			{
				printf("RecvSize:%d\n",sizeof(com->RecvBuff));
				com->ComRead(com->RecvBuff,MAXLEN);
				
//				com->ReadDataProc(com->RecvBuff,MAXLEN);
			}
			memset(com->RecvBuff,0,sizeof(com->RecvBuff));
		}
	}
}	
***/

/***********
功能：串口读取函数
参数：char * ReadBuff：数据读取缓存区；int ReadLen:准备读取总字节长度
返回值：len：操作中所读到的字节数
************/
int ICom::ComRead(char *ReadBuff,const int ReadLen)
{
    if(fd<0)  //检查设备是否打开
    {
        printf("Com error\n");
        return -1;
    }
    //    printf("Ready read\n");

    int len=0;

//    tcflush(this->fd,TCIFLUSH);

    len=read(fd,ReadBuff,ReadLen);


    //    tcflush(this->fd,TCIFLUSH);
//    if(len>0)
//    {
//        std::cout<<"------Get a serial's' data------"<<std::endl;
//        printf("read size:%d \n",len);
//    }
#if DEBUG_PRINT
    std::cout<<"Com["<<this->GetComPortNum()<<"] read size:"<<len<<std::endl;
#endif
    tcflush(this->fd,TCIFLUSH);
    return len;
}

/**
 * @brief ICom::ComRead 无需手动设定读取字节数的串口读方法，默认读取字节为1024个
 * @param ReadBuff      读取到的字节的存放区
 * @return              返回读取到的字节数
 */
int ICom::ComRead(char *ReadBuff)
{
    if(fd<0)
    {
        printf("Com error\n");
        return -1;
    }

    int rlen=0;
    int len=0;

//    tcflush(this->fd,TCIFLUSH);

    len=read(fd,ReadBuff,MAXLEN);
    rlen=len;

    while(len==8)   //MAX等待最小字节为0的情况下，串口一次只会读8个字节。
    {
        len=read(fd,ReadBuff+rlen,MAXLEN);
//        if(len<8)
//            break;

        rlen+=len;

    }
#if DEBUG_PRINT
    std::cout<<"Com["<<this->GetComPortNum()<<"] read size:"<<rlen<<std::endl;
#endif
    tcflush(this->fd,TCIFLUSH);

    return rlen;
}

/**
 * @brief ICom::ComWrite  串口写入函数
 * @param WriteBuff       写入内容缓存区
 * @param wrsize          写入字节数(Bytes)
 * @return                返回写入字节数
 */
int ICom::ComWrite(const char *WriteBuff,size_t wrsize)
{
    if(fd<0)
    {
        std::cout<<"Com error"<<std::endl;
        return -1;
    }

    int wlen;

    tcflush(this->fd,TCIOFLUSH);

    wlen=write(fd,WriteBuff,wrsize);

#if DEBUG_PRINT
    std::cout<<"Com["<<this->sinfo->comNum<<"] Write End"<<endl;
#endif

    return wlen;
}

/********************
 * 功能：判断串口是否已经打开
 * 参数表：无
 * 返回值:如果已经打开则返回true,否则返回false
********************/
bool ICom::isOpen()
{
    if(this->fd>0)
        return true;
    else
        return false;
}

int ICom::GetComPortNum()
{
    return this->sinfo->comNum;
}

/***********
功能：关闭串口
参数：无
返回值：无
************/
void ICom::ComClose(){

    if(this->fd>0)
		close(this->fd);	

}

/**
 * @brief ICom::Enable  自动询问线程激活
 */
void ICom::Enable()
{
    std::cout<<"enable autoread"<<std::endl;
    this->EnableFlag=1;
}

/**
 * @brief ICom::DisEnable   自动询问线程暂停
 */
void ICom::DisEnable()
{

    std::cout<<"disenable auto read"<<std::endl;
    this->EnableFlag=0;
}

bool ICom::IsEnable()
{
    if(EnableFlag<=0)
        return false;
    else
        return true;
}

/**
 * @brief ICom::GetSlaveNum (当前连接的)从机数量
 * @return
 */
int ICom::GetSlaveNum()
{
    return sinfo->slaveCount;
}

/**
 * @brief ICom::AutoAskSensorHandler    自动询问线程函数（业务依赖于ICom::AskSensorFunc）
 * @param arg
 * @return
 */
void *ICom::AutoAskSensorHandler(void *arg)
{
    ICom *com=(ICom *)arg;

    com->ThreadCounts++;

    //业务函数入口
    com->AskSensorFunc();

//    std::cout<<"Serial["<<com->GetComPortNum()<<"]'s thread which auto asking will exit"<<std::endl;
    com->ThreadCounts--;

    pthread_exit(NULL);
}

/**
 * @brief ICom::AskSensorFunc   自动询问具体方法
 */
void ICom::AskSensorFunc()
{
    while(1);
}

/**
 * @brief ICom::CloseSerialThread
 */
void ICom::CloseSerialThread()
{
    this->pthreadflag=false;
}


bool ICom::IsCloseThread()
{
    return !pthreadflag;
}

SerialInfo ICom::GetSerialInfo()
{
//    static SerialInfo si;
//    si=*this->sinfo;

    return (*this->sinfo);
}

/**
 * @brief ICom::ChangeParam 修改串口参数
 * @param cparam    需要修改的参数的类型
 * @param param     提供一组串口参数，当中和cparam表示的类型所对应的值将作为新的参数值，替换掉对应的参数的当前值。
 */
void ICom::ChangeParam(SerialParamType cparam,SerialInfo param)
{
    switch(cparam)
    {
    case BAUD_RATE:
        this->SetComParam(param.baudRate,sinfo->dataBit,sinfo->parity,sinfo->stopBit,sinfo->slaveCount);
        break;

    case DATA_BIT:
        this->SetComParam(sinfo->baudRate,param.dataBit,sinfo->parity,sinfo->stopBit,sinfo->slaveCount);
        break;

    case PARITY:
        this->SetComParam(sinfo->baudRate,sinfo->dataBit,param.parity,sinfo->stopBit,sinfo->slaveCount);
        break;

    case STOP_BIT:
        this->SetComParam(sinfo->baudRate,sinfo->dataBit,sinfo->parity,param.stopBit,sinfo->slaveCount);
        break;

    case COM_NUM:
        break;

    case SLAVE_COUNT:
        this->SetComParam(sinfo->baudRate,sinfo->dataBit,sinfo->parity,sinfo->stopBit,param.slaveCount);
        break;

    case DEVICE_TYPE:
        break;

    default:
        break;

    }
    std::cout<<"change com param over"<<std::endl;
}

_BaudRate ICom::NumToBaudRate(int num){
    _BaudRate bd;
    switch (num) {
    case 2400:
        bd=::BR2400;
        break;

    case 4800:
        bd=::BR4800;
        break;

    case 9600:
        bd=::BR9600;
        break;

    case 19200:
        bd=::BR19200;
        break;

    case 38400:
        bd=::BR38400;
        break;

    case 115200:
        bd=::BR115200;
        break;

    default:
        bd=::BR9600;
        break;
    }
    return bd;
}
