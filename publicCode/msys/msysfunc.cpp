#include "msysfunc.h"

/**
 * @brief stop_process 当在键盘上按下Ctrl+C的时候，调用该函数进行收尾处理
 * @param signo
 */
void stop_process(int signo)
{
    std::cout<<std::endl;
    std::cout<<"Ctrl+C exit!"<<std::endl;
    exit(0);
}

/**
 * @brief segv_default 当程序出现段错误导致崩溃的时候，调用该函数进行收尾
 * @param signo
 */
void segv_default(int signo)
{
    std::cout<<"segmentation violation"<<std::endl;

    exit(0);
}

/**
 * @brief DectoHex 将十进制转成十六进制
 * @param dec 十进制数
 * @param hex 转换结果的存放区
 * @param length 结果的格式长度
 * @return
 */
int DectoHex(int dec, unsigned char *hex, int length)
{
    int i;
    for (i = length - 1; i >= 0; i--)
    {
        hex[i] = (dec % 256) & 0xFF;
        dec /= 256;
    }
    return 0;
}


/**
 * @brief Interchange::startThread      创建一个分离的线程
 * @param funchandle                    函数指针
 * @param tp                            指针参数
 */
void LnxCreateThread(void*(*funchandle)(void *),void *tp){
    pthread_t pid;
    pthread_attr_t pattr;
    pthread_attr_init(&pattr);
    pthread_attr_setscope(&pattr,PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setdetachstate(&pattr,PTHREAD_CREATE_DETACHED);

    pthread_create(&pid,&pattr,funchandle,tp);
}

/**
 * @brief mystrncpy 无视'\0'结束符的字符串拷贝方法
 * @param dest  目的缓存
 * @param source    源缓存
 * @param len   拷贝长度
 * @return
 */
char *mystrncpy(char *dest, const char *source, size_t len)
{
    unsigned int i;
    for(i=0;i<len;i++)
    {
        dest[i]=source[i];
    }
    return dest;
}

/**
 * @brief ByteToHexNum  "char字节"转"int十六进制"
 * @param cbyte
 * @return
 */
long ByteToHexNum(unsigned char *cbyte,size_t len)
{
    int p=len/sizeof(unsigned char)-1;

    long num;
    for(int i=0;i<p;i++)
    {
        num+=cbyte[i]<<((p-i)*8);
    }

    return num;
}

/**
 * @brief ByteToDecNum  把BCD码转换成为整型的10进制数
 * @param cbyte         bcd码串
 * @return
 */
long ByteToDecNum(unsigned char *cbyte,size_t len)
{
    long dec;
    int hight_dec,low_dec;
    int p=len/sizeof(unsigned char)-1;

    if(p>0&&p%2==0)
    {
        //(BCD码格式)地址抽取成十进制
        for(int i=p;i>=0;i--)
        {
            hight_dec=cbyte[i]>>4;           //高四位抽取成十进制
            low_dec=cbyte[i]-hight_dec*0x10;   //低四位抽取为十进制

            dec+=hight_dec*pow(10,(p-i)*2)+low_dec*pow(10,(p-i)*2-1);   //按位数顺序合并
        }

    }
    return dec;
}

long nByteToDecNum(u_char *cbyte, size_t begin,size_t len)
{
    long dec;
    int hight_dec,low_dec;

    u_char bcdbuff[len];

    memcpy(bcdbuff+begin,cbyte,len);

    int p=sizeof(bcdbuff)/sizeof(u_char)-1;

    if(p>0&&p%2==0)
    {
        //(BCD码格式)地址抽取成十进制
        for(int i=p;i>=0;i--)
        {
            hight_dec=bcdbuff[i]>>4;           //高四位抽取成十进制
            low_dec=bcdbuff[i]-hight_dec*0x10;   //低四位抽取为十进制

            dec+=hight_dec*pow(10,(p-i)*2)+low_dec*pow(10,(p-i)*2-1);   //按位数顺序合并
        }

    }
    return dec;
}

std::string ByteToString(u_char *cbyte,size_t len)
{
    std::string str;

    for(size_t i=0;i<len/sizeof(char);i++)
    {
        u_int temp=ByteToDecNum(&cbyte[i],1);
        str+=std::to_string(temp);
    }

    return str;
}

/**
 * @brief DecNumToBCDCode   将整形十进制数字转换成为bcd码
 * @param number    十进制数字
 * @param bcdcode   bcd码存放区
 * @param numlen    十进制数的位数（位数是指个，十，百，千.....进位）
 */
bool DecNumToBCDCode(int number, unsigned char *bcdcode, size_t numlen,size_t bcdlen)
{
    unsigned int h,l;

    if(bcdlen*2<numlen){    //防止内存越界访问
        return false;
    }

    for(size_t i=0;i<4;i++)
    {
        h=number/(int)pow(10,(numlen-1));   //number除以10^numlen;
        number=number-(int)pow(10,(numlen-1))*h;

        l=number/(int)pow(10,(numlen-2));
        number=number-(int)pow(10,(numlen-2))*l;

        numlen-=2;

        bcdcode[i]=(h<<4)+l;
        printf("%d,%d,%x\n",h,l,bcdcode[i]);
    }
    return true;
}

/**
 * @brief GetHostAddress 获取本地的ip地址
 * 笔记：
 * 函数中尝试了两种方法，
 * 第一种是通过gethostname()以及gethostbyname(),不过该方法失败了，网上有不少帖子都说该方法其实并不适用于Linux，gethostbyname()经常都会返回NULL
 * 第二种可以成功获取到本地（局域网分配的）IP地址
 * @return
 */
std::string GetHostAddress()
{
    std::string ipstr="Got localhost IP error!";

    //方法一(失败)：
    /**
    printf("poit1\n");
    char hostname[16]={0};
    struct hostent *he=NULL;
    gethostname(hostname,sizeof(hostname));
    printf("%s\n",hostname);

    if((he=gethostbyname(hostname))==0)
        return ip;


    printf("poit2\n");

    ip=inet_ntoa(*(struct in_addr*)(he->h_addr));

    printf("poit3\n");

    return ip;

    */

    //方法二：
    int  MAXINTERFACES=16;

    int fd, intrface;
    struct ifreq buf[MAXINTERFACES]; ///if.h
    struct ifconf ifc; ///if.h

    if ((fd = socket (AF_INET, SOCK_DGRAM, 0)) >= 0) //socket.h
    {
        ifc.ifc_len = sizeof buf;
        ifc.ifc_buf = (caddr_t) buf;
        if (!ioctl (fd, SIOCGIFCONF, (char *) &ifc)) //ioctl.h
        {
            intrface = ifc.ifc_len / sizeof (struct ifreq);

            while (intrface-->0)
            {
                if (!(ioctl (fd, SIOCGIFADDR, (char *) &buf[intrface])))
                {
                    ipstr=(inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr));//types
                    break;
                }
            }
        }
        close (fd);
    }

    return ipstr;
}

/**
 * @brief getCurrentTime 获取当前系统时间
 * @return
 */
std::string getCurrentTime(){
    time_t timep;
    time(&timep);
    std::string time=ctime(&timep);
    return time;
}

void debugPrint(const char *debug_msg,...){

#if DEBUG_MSG_PRINT
    printf("%s\n",debug_msg)
#endif

}
