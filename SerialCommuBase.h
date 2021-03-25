#ifndef SERIALCOMMUBASE_H
#define SERIALCOMMUBASE_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <asm/ioctls.h>
#include "DefineDb.h"

#define MAX_PORT_NUM				8

#define NO_FLOW_CONTROL				0
#define HW_FLOW_CONTROL				1
#define SW_FLOW_CONTROL				2

#define	SERIAL_OK				     0
#define SERIAL_ERROR_FD				-1
#define SERIAL_ERROR_OPEN			-2
#define SERIAL_PARAMETER_ERROR	    -3

class SerialCommuBase
{
public:
    SerialCommuBase();

    int SerialOpen(int port);
    int SerialSetParam(int fd,int speed,int parity,int databits,int stopbit);
    int SerialClose(int fd);   // ok
    int SerialWrite(int fd, unsigned char *str, int len); // ok
    int SerialNonBlockRead(int fd,unsigned char *buf,int len); // ok
    int SerialBlockRead(int fd, unsigned char *buf, int len);   // ok
    int SerialDataInInputQueue(int fd);   // ok
    int SerialDataInOutputQueue(int fd);  // ok

    int SerialReadData(int fd, unsigned char *buf, int len, int timeout); // ok
    void SerialFlushBuffer(int fd);  //ok

    int SerialSetSpeed(int fd,unsigned int speed);




};

#endif // SERIALCOMMUBASE_H
