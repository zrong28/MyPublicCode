#pragma once
#ifndef COM_DRV_H
#define COM_DRV_H

#define ZLG_IMX 0
#define GEC6818 0
#define IOT9608 1

#include "msys/msysfunc.h"
#include "msys/CommandDeviece.h"

//#define MAXLEN 10240
#define DEBUG_PRINT 0

/**
 * 串口参数类型
 */
typedef enum SerialParamType{
    BAUD_RATE=0x01,
    DATA_BIT=0x02,
    PARITY=0x04,
    STOP_BIT=0x08,
    COM_NUM=0x16,
    SLAVE_COUNT=0x32,
    DEVICE_TYPE=0x64
}SerialParamType;

/**
  校验位
 */
typedef enum _ParityType
{
    None='N',
    Odd='O',
    Even='E',
    Space='S'
}_ParityType;

/**
  波特率
  */
typedef enum _BaudRate
{
    BR2400=B2400,
    BR4800=B4800,
    BR9600=B9600,
    BR19200=B19200,
    BR38400=B38400,
    BR115200=B115200
}_BaudRate;

/**
 * @brief The comNumber enum
 * 可使用串口号
 */
enum comNumber
{
    COM0=0,
    COM1=1,
    COM2=2,
    COM3=3,
    COM4=4
};

/**
 * 串口信息、参数
 */
typedef struct SerialInfo{
    comNumber comNum;
    _BaudRate baudRate;
    int dataBit;
    _ParityType parity;
    int stopBit;
    int slaveCount;
    int deviceType;
}SerialInfo;

//串口接口类
class ICom{
public:
    ICom();
	virtual ~ICom();

    static _BaudRate NumToBaudRate(int num);

    bool OpenCom(comNumber cnum);//打开com口，创建串口接受线程
    bool SetComParam(_BaudRate Baud_rate,const int Data_bits,
        _ParityType Parity,const int Stop_bits,const int slNum);//配置串口参数

    //获取串口号
    int GetComPortNum();

    bool isOpen();
    bool IsEnable();

    //线程自动循环读取数据设置
    void Enable();
    void DisEnable();

    //修改参数
    void ChangeParam(SerialParamType cparam,SerialInfo param);

    virtual int ComRead(char *ReadBuff,const int ReadLen); //读取函数
    virtual int ComWrite(const char *WriteBuff,size_t wrsize); //写入函数
    virtual int ComRead(char *ReadBuff);

    //自动询问线程
    static void *AutoAskSensorHandler(void *arg);

    virtual int GetSlaveNum();

    void CloseSerialThread();

    int GetThreadFlag();

    bool IsCloseThread();

    int ThreadCounts;   //服务于该串口的线程数量
	
    SerialInfo GetSerialInfo();

    virtual void ComClose();        //关闭串口

protected:

    int SlaveNum;                   //记录从机数量
    unsigned int ConnectDeviceType; //设备类型描述
    pthread_mutex_t mutex_lock;     //串口类的内部线程锁

    int fd;                     //文件描述符
    char *RecvBuff;
    int EnableFlag;             //自动读取使能标志
    struct SerialInfo *sinfo;   //记录串口信息主要
    bool pthreadflag;           //自动发送线程的开关状态

    virtual void AskSensorFunc();   //自动询问处理函数



private:

    struct termios newtio;

};


#endif
