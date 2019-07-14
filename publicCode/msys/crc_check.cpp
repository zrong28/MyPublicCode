#include "crc_check.h"

/*****CRC校验产生*****/
unsigned short CRC_check(const unsigned char *puchMsg,
                                unsigned short usDataLen)
{
    unsigned char uchCRCHi=0xFF;
    unsigned char uchCRCLo=0xFF;
    unsigned short uIndex = 0;

    while(usDataLen--)
    {
        uIndex=uchCRCHi^ *puchMsg++;
        uchCRCHi=(unsigned char)(uchCRCLo^auchCRCHi[uIndex]);
        uchCRCLo=auchCRCLo[uIndex];
    }
    return (unsigned short)(uchCRCHi<<8|uchCRCLo);
}


/*****CRC校验码产生*****/
unsigned short CRC_check_tw(const unsigned char *puchMsg,
                                unsigned short usDataLen,
                                int usHiLo)
{
    unsigned char uchCRCHi = 0xff;
    unsigned char uchCRCLo = 0xff;
    unsigned short uIndex = 0;
    while(usDataLen--)
    {
        uIndex=uchCRCHi^*puchMsg++;
        uchCRCHi=(unsigned char)(uchCRCLo^auchCRCHi[uIndex]);
    }
    if(usHiLo==1)
    {
        return (unsigned short)(uchCRCHi<<8|uchCRCLo);

    }
    else
        return (unsigned short)(uchCRCLo<<8|uchCRCHi);
}

/**
 * @brief Checking_CRC_CODE CRC校对
 * @param pData             数据
 * @param nLen              数据长度
 * @return         校对结果，正确返回true，错误返回false
 */
bool Checking_CRC_CODE(const unsigned char *pData,int nLen)
{
    unsigned short CheckCode=CRC_check_tw(pData,nLen-2,1);
    unsigned char cHi=CheckCode>>8&0xff;
    unsigned char cLo=CheckCode&0xff;

    if(cHi!=pData[nLen-2]||cLo!=pData[nLen-1])
        return false;
    return true;
}
