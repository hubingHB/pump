/************************************************************
** 版权所有 (C)2015上海颐坤自动化设备有限公司工程部
** 项目名称: 本安电力保护分站
*************************************************************
** 文件名称:SocketInit.c
** 文件标识:
** 摘要:TCP通信驱动
*************************************************************
** 作者：	hb
** 版本信息:V3.0
** 完成日期:2016-1-1
*************************************************************/

#include "modbustcp_base.h"

modbustcp_base::modbustcp_base()
{

}

int	modbustcp_base::TCPServerInit(int port, int *serverfd)
{
    struct sockaddr_in dest;
    *serverfd = socket(PF_INET,SOCK_STREAM,0);
    memset((void*)&dest,'\0',sizeof(dest));
    dest.sin_family = PF_INET;
    dest.sin_port = htons(port);
    dest.sin_addr.s_addr = INADDR_ANY;
    bind(*serverfd,(struct sockaddr*)&dest,sizeof(dest));

    return *serverfd;
}

int	modbustcp_base::TCPServerWaitConnection(int serverfd,int *clientfd,char *clientaddr)
{
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    listen(serverfd,20);
    *clientfd = accept(serverfd,(struct sockaddr*)&client_addr,&addrlen);
    strcpy(clientaddr,(const char *)(inet_ntoa(client_addr.sin_addr)));

    return *clientfd;
}

int	modbustcp_base::TCPClientInit(int *clientfd)
{
    *clientfd = socket(PF_INET,SOCK_STREAM,0);

    return *clientfd;
}

int	modbustcp_base::TCPClientConnect(const int clientfd,const char *addr,int port)
{
    struct sockaddr_in dest;
    memset(&dest, '\0', sizeof(dest));
    dest.sin_family = PF_INET;
    dest.sin_port = htons(port);
    inet_aton(addr,&dest.sin_addr);

    return connect(clientfd,(struct sockaddr*)&dest,sizeof(dest));
}

int	modbustcp_base::TCPNonBlockRead(int clientfd,unsigned char *buf,int size)
{
    int opts;
    opts = fcntl(clientfd,F_GETFL);
    opts = (opts | O_NONBLOCK);
    fcntl(clientfd,F_SETFL,opts);

    return recv(clientfd,buf,size,0);
}

int	modbustcp_base::TCPBlockRead(int clientfd,unsigned char *buf,int size)
{
    int opts;
    opts = fcntl(clientfd, F_GETFL);
    opts = (opts & ~O_NONBLOCK);
    fcntl(clientfd, F_SETFL, opts);

    return recv(clientfd,buf,size,0);
}

int	modbustcp_base::TCPWrite(int clientfd,unsigned char *buf,int size)
{
    int len= 0;
    len= send(clientfd,buf,size,MSG_NOSIGNAL);

    return len;
}

void modbustcp_base::TCPClientClose(int sockfd)
{
    close(sockfd);
}

void modbustcp_base::TCPServerClose(int sockfd)
{
    close(sockfd);
}

int  modbustcp_base::TCPReadData(int clientfd,unsigned char *buf,int size,int timeout)
{
    struct timeval time;
    time.tv_sec = timeout / 1000;
    time.tv_usec = (timeout % 1000) * 1000;
    fd_set fds;
    int rs;
    FD_ZERO(&fds);
    FD_SET(clientfd,&fds);
    rs = select(clientfd+1,&fds,NULL,NULL,&time);
    if(rs > 0)
    {
        if(FD_ISSET(clientfd,&fds))
        {
            return recv(clientfd,buf,size,0);
        }
    }
    return 0;
}


