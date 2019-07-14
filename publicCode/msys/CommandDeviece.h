#ifndef COMMAND_DEVIECE_H
#define COMMAND_DEVIECE_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>

#define RECVSIZE        256
#define MODBUSLEN       8

#define OTHERDEVICE     0x00
#define WSTHE_MODBUS    1
#define DEVICE_ACM30U1  2

#define MAXLEN 10240

#define PRINTJSON

#endif
