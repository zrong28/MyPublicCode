#ifndef MSYSFUNC_H
#define MSYSFUNC_H
#include <signal.h>
#include <iostream>
#include <setjmp.h>
#include <math.h>
#include <memory.h>
#include "CommandDeviece.h"

#define DEBUG_MSG_PRINT 0

void stop_process(int signo);

void segv_default(int signo);

int DectoHex(int dec, unsigned char *hex, int length);
bool DecNumToBCDCode(int number, unsigned char *bcdcode, size_t numlen,size_t bcdlen);

long ByteToHexNum(unsigned char *cbyte,size_t len);

long ByteToDecNum(unsigned char *cbyte,size_t len);

long nByteToDecNum(u_char *cbyte,size_t begin,size_t len);

void LnxCreateThread(void*(*funchandle)(void *),void *tp);

char *mystrncpy(char *dest,const char *source,size_t len);

std::string ByteToString(u_char *cbyte,size_t len);

std::string GetHostAddress();

std::string getCurrentTime();

void debugPrint(const char *debug_msg,...);
#endif // MSYSFUNC_H
