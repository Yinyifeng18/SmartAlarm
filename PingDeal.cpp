/*!
 ***************************************************************************************
 *
 *  Copyright ? 2017-2018 yifneg. All Rights Reserved.
 *
 * \file      PingDeal.cpp
 * \author    yifeng
 * \version   1.0
 * \date      2023年3月16日
 * \brief     Ping处理源程序
 *
 *--------------------------------------------------------------------------------------
 * \attention
 *
 *
 ***************************************************************************************
 */

/*****************************************************************************************
 change history: 
    1.date  : 2023年3月16日
      author: yifeng
      change: create file

*****************************************************************************************/


/*========================================================================================
                                 本源程序包括的头文件                                     
建议：包含本项目的文件使用 #include "文件名.扩展名"，                                   
      包含系统库的文件使用 #include <文件名.扩展名> 。                                    
========================================================================================*/
#include "PingDeal.h"
#include "TimerDeal.h"

/*=======================================================================================
下面这三条指令是用于区分编译系统为 C++ 或 C 的预处理指令。                               
"__cplusplus" 为 C++ 预处理器名字，这三条指令表明接在其下面的那些指令为 C 程序特征.      
=======================================================================================*/
#ifdef __cplusplus
extern "C" {
#endif


/*=======================================================================================
                            本源程序文件内部使用的字符化常数定义                         
=======================================================================================*/
/*---------------------------------------------------------------------------------------
的字符化常数定义：
----------------------------------------------------------------------------------------*/


/*=======================================================================================
                             本源程序文件内部使用的函数原型声明                          
=======================================================================================*/


/*=======================================================================================
                       本源程序文件内部使用的局部常量、字符化常数定义                    
=======================================================================================*/
/*---------------------------------------------------------------------------------------
的局部常量、字符化常数定义：
---------------------------------------------------------------------------------------*/


/*=======================================================================================
                                         全局常量定义                                    
=======================================================================================*/
/*---------------------------------------------------------------------------------------
的全局常量定义：
---------------------------------------------------------------------------------------*/


/*=======================================================================================
                                         全局变量定义                                    
=======================================================================================*/
/*---------------------------------------------------------------------------------------
的全局变量定义：
---------------------------------------------------------------------------------------*/
struct timeval FirstSendTime; //用以计算总的时间
struct timeval LastRecvTime;
struct hostent *pHost = NULL;  //保存主机信息

PingDealStru gstvPing;

struct sockaddr_in dest_addr;  //IPv4专用socket地址,保存目的地址

/*=======================================================================================
                             本源程序文件内部使用的局部变量定义                          
=======================================================================================*/




/*
*****************************************************************************************
                            　       程序指令代码清单
*****************************************************************************************
*/





/*!
 * \fn     Compute_cksum
 * \brief  校验
 *          
 * \param  [in] struct icmp *pIcmp   #
 * 
 * \retval u16
 */
u16 Compute_cksum(struct icmp *pIcmp)
{
  u16 *data = (u16 *)pIcmp;
  int len = ICMP_LEN;
  u32 sum = 0;
  
  while (len > 1)
  {
    sum += *data++;
    len -= 2;
  }
  if (1 == len)
  {
    u16 tmp = *data;
    tmp &= 0xff00;
    sum += tmp;
  }
  
  //ICMP校验和带进位
  while (sum >> 16)
  sum = (sum >> 16) + (sum & 0x0000ffff);
  sum = ~sum;
  
  return sum;
}
 
/*!
 * \fn     SetICMP
 * \brief  设置ICMP
 *          
 * \param  [in] u16 seq   #
 * 
 * \retval void
 */
void SetICMP(u16 seq)
{
  struct icmp *pIcmp;
  struct timeval *pTime;
  
  pIcmp = (struct icmp*)&gstvPing.SendBuffer;
  
  /* 类型和代码分别为ICMP_ECHO,0代表请求回送 */
  pIcmp->icmp_type  = ICMP_ECHO;
  pIcmp->icmp_code  = 0;
  pIcmp->icmp_cksum = 0;  //校验和
  pIcmp->icmp_seq   = seq;  //序号
  pIcmp->icmp_id    = getpid(); //取进程号作为标志
  pTime = (struct timeval *)pIcmp->icmp_data;
  gettimeofday(pTime, NULL); //数据段存放发送时间
  pIcmp->icmp_cksum = Compute_cksum(pIcmp);
  
  if (1 == seq)
  FirstSendTime = *pTime;
}
 
/*!
 * \fn     SendPacket
 * \brief  发送数据报
 *          
 * \param  [in] int sock_icmp                   #
 * \param  [in] struct sockaddr_in *dest_addr   #
 * \param  [in] int nSend                       #
 * 
 * \retval void
 */
void SendPacket(int sock_icmp, struct sockaddr_in *dest_addr, int nSend)
{
  SetICMP(nSend);
  if (sendto(sock_icmp, (char *)&gstvPing.SendBuffer[0], ICMP_LEN, 0,
   (struct sockaddr *)dest_addr, sizeof(struct sockaddr_in)) < 0)
  {
    perror("sendto");
    return;
  }
}
 
/*!
 * \fn     GetRtt
 * \brief  获得RTT
 *          
 * \param  [in] struct timeval *RecvTime   #
 * \param  [in] struct timeval *SendTime   #
 * 
 * \retval double
 */
double GetRtt(struct timeval *RecvTime, struct timeval *SendTime)
{
  struct timeval sub = *RecvTime;
  
  if ((sub.tv_usec -= SendTime->tv_usec) < 0)
  {
    --(sub.tv_sec);
    sub.tv_usec += 1000000;
  }
  sub.tv_sec -= SendTime->tv_sec;
  
  return sub.tv_sec * 1000.0 + sub.tv_usec / 1000.0; //转换单位为毫秒
}

 
/*!
 * \fn     unpack
 * \brief  解析报文
 *          
 * \param  [in] struct timeval *RecvTime   #
 * 
 * \retval int
 */
int unpack(struct timeval *RecvTime)
{
  struct ip *Ip = (struct ip *)&gstvPing.RecvBuffer;
  struct icmp *Icmp;
  int ipHeadLen;
  double rtt;
  
  ipHeadLen = Ip->ip_hl << 2; //ip_hl字段单位为4字节
  Icmp = (struct icmp *)(gstvPing.RecvBuffer + ipHeadLen);
  
  //判断接收到的报文是否是自己所发报文的响应
  if ((Icmp->icmp_type == ICMP_ECHOREPLY) && Icmp->icmp_id == getpid())
  {
    struct timeval *SendTime = (struct timeval *)Icmp->icmp_data;
    rtt = GetRtt(RecvTime, SendTime);
  
    printf("%u bytes from %s: icmp_seq=%u ttl=%u time=%.1f ms\n",
    ntohs(Ip->ip_len) - ipHeadLen,
    inet_ntoa(Ip->ip_src),
    Icmp->icmp_seq,
    Ip->ip_ttl,
    rtt);
  
    if (rtt < gstvPing.min || 0 == gstvPing.min)
      gstvPing.min = rtt;
    if (rtt > gstvPing.max)
      gstvPing.max = rtt;
    gstvPing.avg += rtt;
    gstvPing.mdev += rtt * rtt;
   
    return 0;
  }
  
  return -1;
}
 
 
/*!
 * \fn     Statistics
 * \brief  参数
 *          
 * 
 * \retval int
 */
int Statistics(void)
{
  double tmp;
  gstvPing.avg /= gstvPing.nRecv;
  tmp = gstvPing.mdev / gstvPing.nRecv - gstvPing.avg * gstvPing.avg;
  gstvPing.mdev = sqrt(tmp);
  
  printf("%d packets transmitted, %d received, %d%% packet loss, time %dms\n"
    , gstvPing.nSend
    , gstvPing.nRecv
    , (gstvPing.nSend - gstvPing.nRecv) / gstvPing.nSend * 100
    , (int)GetRtt(&LastRecvTime, &FirstSendTime));
  printf("rtt min/avg/max/mdev = %.3f/%.3f/%.3f/%.3f ms\n",
    gstvPing.min, gstvPing.avg, gstvPing.max, gstvPing.mdev);
  
  close(gstvPing.sock_icmp);
  return (gstvPing.nSend - gstvPing.nRecv)==gstvPing.nSend?1:0;
}
 
/*!
 * \fn     RecvePacket
 * \brief  接收数据包
 *          
 * \param  [in] int sock_icmp                   #
 * \param  [in] struct sockaddr_in *dest_addr   #
 * 
 * \retval int
 */
int RecvePacket(int sock_icmp, struct sockaddr_in *dest_addr)
{
  int RecvBytes = 0;
  int addrlen = sizeof(struct sockaddr_in);
  struct timeval RecvTime;
  
  if((RecvBytes = recvfrom(sock_icmp, (char *)&gstvPing.RecvBuffer[0], RECV_BUFFER_SIZE, 0, (struct sockaddr *)dest_addr, (socklen_t *)&addrlen)) < 0)
  {
    perror("recvfrom");
    return 0;
  }
 
  gettimeofday(&RecvTime, NULL);
  LastRecvTime = RecvTime;
  
  if (unpack(&RecvTime) == -1)
  {
    return -1;
  }
  gstvPing.nRecv++;
}



/*!
 * \fn     PingDealHanle
 * \brief  Ping处理函数
 *          
 * \param  [in] char *ip   # 输入IP地址
 * 
 * \retval int
 */
int PingDealHanle(char *ip)
{
  int ret=-1;
  struct protoent *protocol;
  in_addr_t inaddr;  //ip地址（网络字节序）
  struct timeval timeout = {3, 0};
  int TimeLen = sizeof(timeout);
  u8    Step;
  int unpack_ret;

  Step = gstvPing.Step;

  switch(Step)
  {
    case 0:
    {
      gstvPing.nSend=0;
      gstvPing.nRecv = 0;
      pHost=NULL;
      gstvPing.min=0.0;
      gstvPing.avg=0.0;
      gstvPing.max=0.0;
      gstvPing.mdev=0.0;
      printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);
      ret = 2;
      Step++;
      break;
    }
    case 1:
    {
      printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);
      if ((protocol = getprotobyname("icmp")) == NULL)
      {
        perror("getprotobyname");
        gstvPing.Step = 88;
        return -1;
      }
      
      /* 创建ICMP套接字 */
      //AF_INET:IPv4, SOCK_RAW:IP协议数据报接口, IPPROTO_ICMP:ICMP协议
      if ((gstvPing.sock_icmp = socket(PF_INET, SOCK_RAW, protocol->p_proto/*IPPROTO_ICMP*/)) < 0)
      {
        perror("socket");
        gstvPing.Step = 88;
        return -1;
      }
      
      setsockopt(gstvPing.sock_icmp, SOL_SOCKET, SO_RCVTIMEO, &timeout, TimeLen);
      dest_addr.sin_family = AF_INET;
      
      /* 将点分十进制ip地址转换为网络字节序 */
      if ((inaddr = inet_addr(ip)) == INADDR_NONE)
      {
        /* 转换失败，表明是主机名,需通过主机名获取ip */
        if ((pHost = gethostbyname(ip)) == NULL)
        {
          herror("gethostbyname()");
          gstvPing.Step = 88;
          return -1;
        }
        memmove(&dest_addr.sin_addr, pHost->h_addr_list[0], pHost->h_length);
      }
      else
      {
        memmove(&dest_addr.sin_addr, &inaddr, sizeof(struct in_addr));
      }
      
      if (NULL != pHost)
      {
        printf("PING %s", pHost->h_name);
      }
      else
      {
        printf("PING %s", ip);
        printf("(%s) %d bytes of data.\n", inet_ntoa(dest_addr.sin_addr), ICMP_LEN);
      }
      
      Step++;
      ret = 2;
      break;
    }
    case 2:
    {
      printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);
      SendPacket(gstvPing.sock_icmp, &dest_addr, gstvPing.nSend);
      unpack_ret = RecvePacket(gstvPing.sock_icmp, &dest_addr);
      if (-1 == unpack_ret) //（ping回环时）收到了自己发出的报文,重新等待接收
        RecvePacket(gstvPing.sock_icmp, &dest_addr);
            
      Step++;
      gstvPing.Delay = TimerGet1msInc();

      ret = 2;
      
      break; 
    }
    case 3:
    {      
      if(TimerGet1msSubb(gstvPing.Delay, 1000))
      {
        printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);
        gstvPing.nSend++;
        if(gstvPing.nSend < SEND_NUM)
        {
          Step = 2;
        }
        else
        {
          Step++;
        }
      }

      ret = 2;
      break;
    }
    case 4:
    {
      ret=Statistics(); //输出信息，关闭套接字
      printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);
      break;
    }
    default:
    {
      PingDealParaInit();
      ret = -1;
      printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);
      break;
    }
  }
  
  gstvPing.Step = Step;

  return ret;
}
 
/*!
 * \fn     PingDealParaInit
 * \brief  参数初始化
 *          
 * \param  [in] void   #
 * 
 * \retval void
 */
void PingDealParaInit(void)
{
  memset(&gstvPing, 0, sizeof(PingDealStru));
  gstvPing.Step = 0;
}



/*=======================================================================================
下面这三条指令是用于与上面三条区分编译系统为 C++ 或 C 的预处理指令相对应。用于指定
 extern "C" 链接指示符作用域，此处的"}"与上面的"{"相对应，为链接指示符作用结束符。
=======================================================================================*/
#ifdef __cplusplus
}
#endif


/*
*****************************************************************************************
                                 本Ｃ语言源程序文件到此结束
*****************************************************************************************
*/

 
 

