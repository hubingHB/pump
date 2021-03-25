/************************************************************
** 版权所有 (C)2020上海颐坤自动化设备有限公司工程部
** 项目名称: 本安电力保护分站
*************************************************************
** 文件名称:SocketInit.h
** 文件标识:
** 摘要:TCP通信驱动
*************************************************************
** 作者：	HB
** 版本信息:V3.0
** 完成日期:2020-7-4
*************************************************************/
#ifndef MODBUSTCP_BASE_H
#define MODBUSTCP_BASE_H

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <resolv.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#define MAX_CONNECTION 20


class modbustcp_base
{
public:
    modbustcp_base();

    int	 TCPServerInit(int port,int *serverfd);
    int	 TCPServerWaitConnection(int serverfd,int *clientfd,char *clientaddr);
    int	 TCPClientInit(int *clientfd);
    int	 TCPClientConnect(const int clientfd, const char *addr, int port);
    int	 TCPNonBlockRead(int clientfd,unsigned char *buf,int size);
    int  TCPBlockRead(int clientfd, unsigned char *buf, int size);
    int	 TCPWrite(int clientfd, unsigned char *buf, int size);
    void TCPClientClose(int sockfd);
    void TCPServerClose(int sockfd);
    int  TCPReadData(int clientfd, unsigned char *buf, int size, int timeout);

};

#endif // MODBUSTCP_BASE_H
