/*!
 ***************************************************************************************
 *
 *  Copyright ? 2017-2018 yifneg. All Rights Reserved.
 *
 * \file      NetDeal.cpp
 * \author    yifeng
 * \version   1.0
 * \date      2023年3月11日
 * \brief     网络处理源程序
 *
 *--------------------------------------------------------------------------------------
 * \attention
 *
 *
 ***************************************************************************************
 */

/*****************************************************************************************
 change history: 
    1.date  : 2023年3月11日
      author: yifeng
      change: create file

*****************************************************************************************/


/*========================================================================================
                                 本源程序包括的头文件                                     
建议：包含本项目的文件使用 #include "文件名.扩展名"，                                   
      包含系统库的文件使用 #include <文件名.扩展名> 。                                    
========================================================================================*/
#include "NetDeal.h"
#include "TimerDeal.h"
#include "EepromDeal.h"
#include "PingDeal.h"
#include "NtpDeal.h"
#include "MqttDeal.h"

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
static char* eth0_ifname = "ens33";


/*=======================================================================================
                             本源程序文件内部使用的函数原型声明                          
=======================================================================================*/
static void *NetDeal_Thread(void *arg);
static void WiredLinkDetHandle(void);
static int WiredCheckNic(char *nic_name, int* sock);
static void WiredNetHandle(void);
static bool WiredInitBootHandle(void);
static bool WireIPParaSetHandle(void);
static bool WiredDHCPBootHandle(void);
static int GetIpAddress(char* ifaceName, char *ipaddr);


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
NetDealStru gstvNet;
NetDealBits gbitNet;


/*=======================================================================================
                             本源程序文件内部使用的局部变量定义                          
=======================================================================================*/




/*
*****************************************************************************************
                            　       程序指令代码清单
*****************************************************************************************
*/


/*!
 * \fn     Start_NetDeal_Monitor_Thread
 * \brief  网络处理操作
 *          
 * \param  [in] void   #
 * 
 * \retval void
 */
void Start_NetDeal_Monitor_Thread(void)
{
  pthread_t net_tidp;
  
  printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);
  
  if(pthread_create(&net_tidp, NULL, NetDeal_Thread, NULL) != 0)
  {
    perror("pthread_create: NetDeal_Thread");
    return ;
  }  
   
}


/*!
 * \fn     NetDeal_Thread
 * \brief  网络处理线程
 *          
 * \param  [in] void *arg   #
 * 
 * \retval static void *
 */
static void *NetDeal_Thread(void *arg)
{
  printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);

  pthread_detach(pthread_self());

  memset(&gstvNet, 0, sizeof(NetDealStru));
  memset(&gbitNet, 0, sizeof(NetDealBits));

  PingDealParaInit();

  while(!gstvSysRun.quit)
  {
     /* 检测网线 */
     WiredLinkDetHandle();     

     /* 网络处理任务 */
     WiredNetHandle();

     usleep(1000);
  }

   pthread_exit(0);
} 


/*!
 * \fn     WireLinkDetHandle
 * \brief  判断网线链接状态
 *          
 * \param  [in] void   #
 * 
 * \retval static void
 */
static void WiredLinkDetHandle(void)
{
  int sockfd = 0;
    
  /* 定时1秒检测网线 */
  if(TimerGet1msSubb(gstvNet.LinkTiming1s, 1000))
  {
    gstvNet.LinkTiming1s = TimerGet1msInc();
  
    if(WiredCheckNic(eth0_ifname, &sockfd) == 0)  // 有插上网线了
    {
      gstvNet.LinkDetHave++;
      gstvNet.LinkDetLoss = 0;

      printf("gstvWired.LinkDetHave is %d\n", gstvNet.LinkDetHave);
    }
    else  
    {
      gstvNet.LinkDetLoss++;
      gstvNet.LinkDetHave = 0;

      printf("gstvWired.LinkDetLoss is %d\n", gstvNet.LinkDetLoss);
    }

    if(gstvNet.LinkDetHave >= 3)
    {
      gstvNet.LinkDetHave = 0;

      if(gbitNet.LinkState == 0x00)
      {
        printf("PHP Link Success!\r\n");

        WiredStateInputHandle(EnWsInit, 0);

        NtpDealRestartHandle();

        MqttDealRestartHandle();
      }

      gbitNet.LinkState = 0x01;
    }
    else if(gstvNet.LinkDetLoss >= 3)
    {
      gstvNet.LinkDetLoss = 0;

      printf("PHP Link Error\r\n");
    }
  }
}




/*!
 * \fn     check_nic
 * \brief  检查网络是否插上
 *          
 * \param  [in] char *nic_name   #
 * \param  [in] int* sock        #
 * 
 * \retval static int
 */
static int WiredCheckNic(char *nic_name, int* sock)
{
  struct ifreq ifr;
  int skfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (-1 == skfd)
  {
    fprintf(stderr, "%s,%s,%d,check_nic sock error\n", __FILE__, __FUNCTION__, __LINE__);
    return -1;
  }
  *sock = skfd;
  strcpy(ifr.ifr_name, nic_name);
  if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)
  {
    close(skfd);
    return -1;
  }

  if(ifr.ifr_flags &  IFF_RUNNING)
  {
    close(skfd);
    return 0;  // 网卡已插上网线
  }
  else
  {
    close(skfd);
    return -1;
  }
}


/*!
 * \fn     WiredNetHandle
 * \brief  有线网络操作处理
 *          
 * \param  [in] void   #
 * 
 * \retval static void
 */
static void WiredNetHandle(void)
{
  int ret = -1;

  if(gbitNet.LinkState == 0x00)
  {
    return ;
  }

  switch(gstvNet.WiredState)
  {
    case EnWsInit:   // 初始化
    {
      ret = WiredInitBootHandle();
      if(ret == true)
      {
        WiredStateInputHandle(EnWsLinkDet, 1);
      }
      else
      {
        WiredStateInputHandle(EnWsFail, 0);
      }
      break;
    }
    case EnWsLinkDet:  // 检测网线
    {
      if(TimerGet1sSubb(gstvNet.StateDelay, 10))
      {
        WiredStateInputHandle(EnWsFail, 0);
      }
      else
      {
        if(gbitNet.LinkState == 0x01)  // 有连接
        {
          WiredStateInputHandle(EnWsIpParaSet, 0);
        }
      }
      break;
    }
    case EnWsIpParaSet:  // 设置IP, 静态或动态
    {
      ret = WireIPParaSetHandle();
      if(ret == true)
      {
        WiredStateInputHandle(EnWsDHCP, 0);
      }
      else
      {
        WiredStateInputHandle(EnWsFail, 0);
      }
      break;
    }
    case EnWsDHCP:  // 获取IP
    {
      ret = WiredDHCPBootHandle();
      if(ret == true)
      {
        WiredStateInputHandle(EnWsPing, 0);
      }
      else
      {
        WiredStateInputHandle(EnWsFail, 0);
      }
      break;
    }
    case EnWsPing:  // ping baidu
    {
      ret = PingDealHanle("www.baidu.com");
      if(ret == 0)
      {
        WiredStateInputHandle(EnWsSucc, 1);
      }
      else if(ret == 2)
      {
      }
      else 
      {
        WiredStateInputHandle(EnWsFail, 1);
      }
      break;
    }
    case EnWsFail:
    {
      gbitNet.WiredOk = CLEAR;
      
      gstvNet.ErrorTry++;
      if(gstvNet.ErrorTry >= 5)
      {
        WiredStateInputHandle(EnWsInit, 0);
      }
      else
      {
        WiredStateInputHandle(EnWsFailDelay, 1);
      }
      break;
    }
    case EnWsFailDelay:
    {
      if(TimerGet1sSubb(gstvNet.StateDelay, 10))
      {
        if(gbitNet.LinkState == 0x01)
        {
          WiredStateInputHandle(gstvNet.WiredStateBak, 0);
        }
      }
      break;
    }
    case EnWsSucc:
    {
      gbitNet.WiredOk = 0x01;
      gstvNet.ErrorTry = 0;
      
      break;
    }
    default:
    {
      WiredStateInputHandle(EnWsInit, 0);
      
      break;
    }
  }
}

/*!
 * \fn     WiredInitBootHandle
 * \brief  初始化
 *          
 * \param  [in] void   #
 * 
 * \retval static void
 */
static bool WiredInitBootHandle(void)
{  
  printf("file: %s, line: %d, function: %s !\n", __FILE__, __LINE__, __FUNCTION__);

  gstvNet.WiredStateBak = EnWsInit;

  /* 初始化所有参数 */
  memset(&gstvNet, 0, sizeof(NetDealStru));
  memset(&gbitNet, 0, sizeof(NetDealBits));

  gbitNet.LinkState = 0x01;
  gbitNet.InitBoot = 0x01;
    
  return true;
}


/*!
 * \fn     WireIPParaSetHandle
 * \brief  设置静态IP或动态IP
 *          
 * \param  [in] void   #
 * 
 * \retval static void
 */
static bool WireIPParaSetHandle(void)
{
  printf("file: %s, line: %d, function: %s !\n", __FILE__, __LINE__, __FUNCTION__);

  if(gstvEeNet.DhcpMode == 0x00)  /* 自动获取 */
  {
    //system("udhcpc");
  }
  else
  {
    /* 静态设置IP */

    /* 设置IP地址 */

    /* 设置Mac地址 */

    /* 设置Bcast地址 */

    /* 设置Mask地址 */
  }

  printf("IpParaSet succeed!\r\n");
  
  gbitNet.DHCPBoot = 0x01;
  gstvNet.WiredStateBak = EnWsIpParaSet;
  
  return true;
}



/*!
 * \fn     WiredDHCPBootHandle
 * \brief  DHCP获取网络参数
 *          
 * \param  [in] void   #
 * 
 * \retval static void
 */
static bool WiredDHCPBootHandle(void)
{
  int ret = 0;
  char msg[64];

  printf("file: %s, line: %d, function: %s !\n", __FILE__, __LINE__, __FUNCTION__);

  gstvNet.WiredStateBak = EnWsDHCP;
    
  ret = GetIpAddress(eth0_ifname, msg);
  if(ret == 0)
  {
    printf("GetIpAddress: %s, ret = %d\n", msg, ret);

    gbitNet.DHCPBoot = 0x01;
  }
  else 
  {
    printf("GetIpAddress failed!\n");

    gbitNet.DHCPBoot = 0x00; 

    /* 一直失败就重新设置网卡 */

    return false;
  }

  return true;
}


/**********************************************************************
 * Function:      GetIpAddress
 * Description:   获得指定端口的ip地址
 * Input:         ifaceName ：网络接口名字 如：eth0，IpAddress：得到的ip地址
 * Return:        成功 1，失败 0
 * Others:        
**********************************************************************/
static int GetIpAddress(char* ifaceName, char *ipaddr)
{
  int fd;
  char buffer[20];
  size_t len = 16;
  struct ifreq ifr;
  struct sockaddr_in *addr;

  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
  {
    strncpy(ifr.ifr_name, ifaceName, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    if (ioctl(fd, SIOCGIFADDR, &ifr) == 0)
    {
      addr = (struct sockaddr_in *)&(ifr.ifr_addr);
      inet_ntop(AF_INET, &addr->sin_addr, buffer, 20);
    }
    else
    {
      close(fd);
      
      return(-1);
    }
  }
  else
  {
    return(-1);
  }

  if (strlen(buffer) > len-1)
  {
    return(-1);
  }
  
  strncpy(ipaddr, buffer, len);
  
  close(fd);
  
  return(0);
}


/*!
 * \fn     WiredStateInputHandle
 * \brief   进入有线网络操作状态
 *          
 * \param  [in] WiredStateEnum WiredState   #
 * 
 * \retval void
 */
void WiredStateInputHandle(WiredStateEnum WiredState, unsigned char DelayMode)
{
  gstvNet.WiredState = WiredState;

  printf("file: %s, line: %d, function: %s !\n", __FILE__, __LINE__, __FUNCTION__);
  printf("WiredStateInputHandle: %d\n", WiredState);
  
  if(DelayMode == 0)
    gstvNet.StateDelay = TimerGet1msInc();
  else 
    gstvNet.StateDelay = TimerGet1sInc();
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
