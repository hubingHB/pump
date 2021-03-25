/************************************************************
** 版权所有 (C)2015上海颐坤自动化设备有限公司工程部
** 项目名称: 本安电力保护分站
*************************************************************
** 文件名称:SerialInit.c
** 文件标识:
** 摘要:串口通信驱动
*************************************************************
** 作者：	lann
** 版本信息:V3.1
** 完成日期:2018-8-27
** 按逻辑顺序对应的物理串口2，1，3，4，7，6
*************************************************************/
#include "SerialCommuBase.h"
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <poll.h>
#include <unistd.h>
#include <getopt.h>
#include <linux/serial.h>
#include <errno.h>
#include "DefineDb.h"
#include <QDebug>
SerialCommuBase::SerialCommuBase()
{

}

//static struct termios oldtio[MAX_PORT_NUM],newtio[MAX_PORT_NUM];
//static int fd_map[MAX_PORT_NUM]={-1,-1,-1,-1,-1,-1,-1,-1};

int	SerialCommuBase::SerialOpen(int port)
{
    int fd;
    char device[80];
    struct termios tio;

//    if(fd_map[port] != -1)
//        return SERIAL_ERROR_OPEN;
    switch(port)
    {
        case 0:sprintf(device,"/dev/ttymxc2");break;
        case 1:sprintf(device,"/dev/ttymxc1");break;
        case 2:sprintf(device,"/dev/ttymxc3");break;
        case 3:sprintf(device,"/dev/ttymxc4");break;
        case 4:sprintf(device,"/dev/ttymxc7");break;
        case 5:sprintf(device,"/dev/ttymxc6");break;
    }
    fd = open(device,O_RDWR | O_NONBLOCK | O_RDWR | O_NOCTTY);
    if(fd < 0)
        return SERIAL_ERROR_OPEN;

    memset(&tio,0x00,sizeof(tio));
//CREAD = 修改控制模式，保证程序不会占用串口
//CLOCAL= 修改控制模式，使得能够从串口中读取输入数据
    tio.c_iflag = 0;
    tio.c_oflag = 0;   /*Output*/
    tio.c_cflag = B9600|CS8|CREAD|CLOCAL;

    tio.c_lflag = 0;  /*Input*/
    tio.c_cc[VTIME] = 0;
    tio.c_cc[VMIN] = 1;

    tcflush(fd,TCIFLUSH);
    tcsetattr(fd,TCSANOW,&tio);

//setup RS485 auto switch voltage with
    struct serial_rs485 rs485;
    memset(&rs485,0x00,sizeof(rs485));
    if(ioctl(fd, TIOCGRS485, &rs485) < 0)
    {
        perror("Error getting RS-485 mode");
    }
    else
    {
        rs485.flags |= SER_RS485_ENABLED | SER_RS485_RTS_ON_SEND | SER_RS485_RTS_AFTER_SEND;
        rs485.delay_rts_after_send = 1;
        rs485.delay_rts_before_send = 0;
        if(ioctl(fd, TIOCSRS485, &rs485) < 0)
        {
            perror("Error setting RS-485 mode");
        }
    }
    return fd;
}

int	SerialCommuBase::SerialWrite(int fd,unsigned char *str,int len)
{
    if(fd < 0)
        return fd;
    else
        return write(fd,str,len);
}

int	SerialCommuBase::SerialNonBlockRead(int fd,unsigned char *buf,int len)
{
    int res = 0;
    if( fd < 0)
    {
        return fd;
    }
    else
    {
        fcntl(fd,F_SETFL,FNDELAY);
        res = read(fd,buf,len);
        return res;
    }
}

int	SerialCommuBase::SerialBlockRead(int fd,unsigned char *buf,int len)
{
    int res = 0;
    if( fd < 0)
    {
        return fd;
    }
    else
    {
        fcntl(fd,F_SETFL,0);
        res = read(fd,buf,len);
        return res;
    }
}

int	SerialCommuBase::SerialClose(int fd)
{
    if(fd < 0)
    {
        return fd;
    }
    else
    {
        close(fd);
        return SERIAL_OK;
    }
}

int	SerialCommuBase::SerialDataInInputQueue(int fd)
{
    int bytes = 0;
    if( fd < 0)
    {
        return fd;
    }
    else
    {
        ioctl(fd,FIONREAD,&bytes);
        return bytes;
    }
}

int	SerialCommuBase::SerialDataInOutputQueue(int fd)
{
    int bytes = 0;
    if( fd < 0)
    {
        return fd;
    }
    else
    {
        ioctl(fd,TIOCOUTQ,&bytes);
        return bytes;
    }
}


int	SerialCommuBase::SerialSetSpeed(int fd,unsigned int speed)
{
    int i,table_size = 23;
    int speed_table1[]={0,50,75,110,134,150,200,300,600,1200,1800,
                        2400,4800,9600,19200,38400,57600,115200,
                        230400,460800,500000,576000,921600};
    int speed_table2[]={B0,B50,B75,B110,B134,B150,B200,B300,B600,
                        B1200,B1800,B2400,B4800,B9600,B19200,B38400,
                        B57600,B115200,B230400,B460800,B500000,B576000,B921600};

    if( fd < 0)
        return fd;

    for(i = 0;i < table_size;i++)
        if(speed_table1[i] >= speed)
            break;


    struct termios options;
    memset(&options,0x00,sizeof(struct termios));
    //tcgetattr(fd,&options)得到与fd指向对象的相关参数，并将它们保存于options,
    // 该函数,还可以测试配置是否正确，该串口是否可用等。若调用成功，函数返回值为0，若调用失败，函数返回值为1.
    //
   if(tcgetattr(fd,&options)!=  0)
   {
       qDebug()<<"SetupSerial error";
       return -1;
   }
    //设置串口输入波特率和输出波特率
    cfsetispeed(&options,speed_table2[i]);
    cfsetospeed(&options,speed_table2[i]);
    tcsetattr(fd,TCSANOW,&options);

    return SERIAL_OK;
}

/*int SerialSetMode(int port,unsigned int mode)
{
    char device[80];
    int ret = 0,fd = FindFD(port);
    if(fd < 0)
    {
        sprintf(device,"/dev/ttyM%d",port);
        fd = open(device, O_RDWR|O_NOCTTY);
        if(fd < 0)
            return SERIAL_ERROR_OPEN;
    }

    ret= ioctl(fd,MOXA_SET_OP_MODE,&mode);
    if(FindFD(port) < 0)
        close(fd);
    return ret;
}*/
int	SerialCommuBase::SerialSetParam(int fd,int speed,int parity,int databits,int stopbit)
{
    int result = 0;
    int index = 0;
    int ArraySize = 23;
    int NameArr[23]= {0,50,75,110,134,150,200,300,600,1200,1800,2400,4800,9600,19200,38400,
                    57600,115200,230400,460800,500000,576000,921600};
    int SpeedArr[23]={B0,B50,B75,B110,B134,B150,B200,B300,B600,B1200,B1800,B2400,B4800,B9600,B19200,B38400,
                    B57600,B115200,B230400,B460800,B500000,B576000,B921600};

    if(fd < 0)
    {
        result = -1;
        return result;
    }

    struct termios options;
    memset(&options,0x00,sizeof(struct termios));

    if(tcgetattr(fd,&options)!=  0)
    {
       qDebug()<<"SetupSerial error";
       result = -2;
       return result;
    }
    else
    {
        //设置串口输入波特率和输出波特率
        for(index = 0;index < ArraySize;index++)
            if(NameArr[index] >= speed)
                break;

        //tcgetattr(fd,&options)得到与fd指向对象的相关参数，并将它们保存于options,
        // 该函数,还可以测试配置是否正确，该串口是否可用等。若调用成功，函数返回值为0，若调用失败，函数返回值为1.
        //
       if(tcgetattr(fd,&options)!=  0)
       {
           qDebug()<<"SetupSerial error";
           return -1;
       }

       cfsetispeed(&options,SpeedArr[index]);
       cfsetospeed(&options,SpeedArr[index]);
      // tcsetattr(fd,TCSANOW,&options);

       //parity 0: none, 1: odd, 2: even, 3: space, 4: mark
       switch(parity)
       {
       case 0: //none
           options.c_cflag &= ~PARENB;
           options.c_iflag &= ~INPCK;
           break;
       case 1: //odd 奇校验
           options.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/
           options.c_iflag |= INPCK;             /* Disnable parity checking */
           break;
       case 2://even 偶校验
           options.c_cflag |= PARENB;     /* Enable parity */
           options.c_cflag &= ~PARODD;   /* 转换为偶效验*/
           options.c_iflag |= INPCK;       /* Disnable parity checking */
           break;
//       case 3:// space 空格
//           options.c_cflag &= ~PARENB;
//           options.c_cflag &= ~CSTOPB;
//           break;
//        case 4:// mark
//            options.c_cflag |= PARENB | CMSPAR | PARODD;
//            break;
       default:
           qDebug()<<"Unsupport parity";
           result = -3;
           break;
       }
       options.c_cflag &= ~CSIZE; // 蔽其他标志位

       switch(databits)
       {
       case 5:
           options.c_cflag |= CS5;
           break;
       case 6:
           options.c_cflag |= CS6;
           break;
       case 7:
           options.c_cflag |= CS7;
           break;
       case 8:
           options.c_cflag |= CS8;
           break;
       default:
           qDebug()<<"Unsupport databits";
           result = -4;
           break;
       }

       switch(stopbit)
       {
       case 1:
           options.c_cflag &= ~CSTOPB;
           break;
       case 2:
           options.c_cflag |= CSTOPB;
           break;
       default:
           qDebug()<<"unsupport stopbit";
           result = -5;
           break;
       }

       tcflush(fd, TCIOFLUSH);
       tcsetattr( fd, TCSANOW, &options);

       return result;
    }
}

//parity	0: none, 1: odd, 2: even, 3: space, 4: mark

//int SerialCommuBase::SerialSetParam(int fd,int speed,int parity,int databits,int stopbit)
//{
//    int rs;
//    rs = SerialSetSpeed(fd,speed);
//    if(rs < 0)
//        return rs;
//    rs = SetParam(fd,parity,databits,stopbit);
//    if(rs < 0)
//        return rs;
//    return SERIAL_OK;
//}

int SerialCommuBase::SerialReadData(int fd,unsigned char *buf,int len,int timeout)
{
    if( fd < 0)
        return fd;

    struct timeval time;
    time.tv_sec = timeout / 1000;
    time.tv_usec = (timeout % 1000) * 1000;
    fd_set fds;
    int rs;
    FD_ZERO(&fds);
    FD_SET(fd,&fds);
    rs = select(fd+1,&fds,NULL,NULL,&time);
    if(rs > 0)
    {
        if(FD_ISSET(fd,&fds))
        {
            return read(fd,buf,len);
        }
    }
    return 0;
}

void SerialCommuBase::SerialFlushBuffer(int fd)
{
    if( fd < 0)
        return;
    tcflush(fd,TCIOFLUSH);
}

