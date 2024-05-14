/*!
 ***************************************************************************************
 *
 *  Copyright ? 2017-2018 yifneg. All Rights Reserved.
 *
 * \file      NetDeal.cpp
 * \author    yifeng
 * \version   1.0
 * \date      2023��3��11��
 * \brief     ���紦��Դ����
 *
 *--------------------------------------------------------------------------------------
 * \attention
 *
 *
 ***************************************************************************************
 */

/*****************************************************************************************
 change history: 
    1.date  : 2023��3��11��
      author: yifeng
      change: create file

*****************************************************************************************/


/*========================================================================================
                                 ��Դ���������ͷ�ļ�                                     
���飺��������Ŀ���ļ�ʹ�� #include "�ļ���.��չ��"��                                   
      ����ϵͳ����ļ�ʹ�� #include <�ļ���.��չ��> ��                                    
========================================================================================*/
#include "NetDeal.h"
#include "TimerDeal.h"
#include "EepromDeal.h"
#include "PingDeal.h"
#include "NtpDeal.h"
#include "MqttDeal.h"

/*=======================================================================================
����������ָ�����������ֱ���ϵͳΪ C++ �� C ��Ԥ����ָ�                               
"__cplusplus" Ϊ C++ Ԥ���������֣�������ָ������������������Щָ��Ϊ C ��������.      
=======================================================================================*/
#ifdef __cplusplus
extern "C" {
#endif


/*=======================================================================================
                            ��Դ�����ļ��ڲ�ʹ�õ��ַ�����������                         
=======================================================================================*/
/*---------------------------------------------------------------------------------------
���ַ����������壺
----------------------------------------------------------------------------------------*/
static char* eth0_ifname = "ens33";


/*=======================================================================================
                             ��Դ�����ļ��ڲ�ʹ�õĺ���ԭ������                          
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
                       ��Դ�����ļ��ڲ�ʹ�õľֲ��������ַ�����������                    
=======================================================================================*/
/*---------------------------------------------------------------------------------------
�ľֲ��������ַ����������壺
---------------------------------------------------------------------------------------*/


/*=======================================================================================
                                         ȫ�ֳ�������                                    
=======================================================================================*/
/*---------------------------------------------------------------------------------------
��ȫ�ֳ������壺
---------------------------------------------------------------------------------------*/


/*=======================================================================================
                                         ȫ�ֱ�������                                    
=======================================================================================*/
/*---------------------------------------------------------------------------------------
��ȫ�ֱ������壺
---------------------------------------------------------------------------------------*/
NetDealStru gstvNet;
NetDealBits gbitNet;


/*=======================================================================================
                             ��Դ�����ļ��ڲ�ʹ�õľֲ���������                          
=======================================================================================*/




/*
*****************************************************************************************
                            ��       ����ָ������嵥
*****************************************************************************************
*/


/*!
 * \fn     Start_NetDeal_Monitor_Thread
 * \brief  ���紦�����
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
 * \brief  ���紦���߳�
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
     /* ������� */
     WiredLinkDetHandle();     

     /* ���紦������ */
     WiredNetHandle();

     usleep(1000);
  }

   pthread_exit(0);
} 


/*!
 * \fn     WireLinkDetHandle
 * \brief  �ж���������״̬
 *          
 * \param  [in] void   #
 * 
 * \retval static void
 */
static void WiredLinkDetHandle(void)
{
  int sockfd = 0;
    
  /* ��ʱ1�������� */
  if(TimerGet1msSubb(gstvNet.LinkTiming1s, 1000))
  {
    gstvNet.LinkTiming1s = TimerGet1msInc();
  
    if(WiredCheckNic(eth0_ifname, &sockfd) == 0)  // �в���������
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
 * \brief  ��������Ƿ����
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
    return 0;  // �����Ѳ�������
  }
  else
  {
    close(skfd);
    return -1;
  }
}


/*!
 * \fn     WiredNetHandle
 * \brief  ���������������
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
    case EnWsInit:   // ��ʼ��
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
    case EnWsLinkDet:  // �������
    {
      if(TimerGet1sSubb(gstvNet.StateDelay, 10))
      {
        WiredStateInputHandle(EnWsFail, 0);
      }
      else
      {
        if(gbitNet.LinkState == 0x01)  // ������
        {
          WiredStateInputHandle(EnWsIpParaSet, 0);
        }
      }
      break;
    }
    case EnWsIpParaSet:  // ����IP, ��̬��̬
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
    case EnWsDHCP:  // ��ȡIP
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
 * \brief  ��ʼ��
 *          
 * \param  [in] void   #
 * 
 * \retval static void
 */
static bool WiredInitBootHandle(void)
{  
  printf("file: %s, line: %d, function: %s !\n", __FILE__, __LINE__, __FUNCTION__);

  gstvNet.WiredStateBak = EnWsInit;

  /* ��ʼ�����в��� */
  memset(&gstvNet, 0, sizeof(NetDealStru));
  memset(&gbitNet, 0, sizeof(NetDealBits));

  gbitNet.LinkState = 0x01;
  gbitNet.InitBoot = 0x01;
    
  return true;
}


/*!
 * \fn     WireIPParaSetHandle
 * \brief  ���þ�̬IP��̬IP
 *          
 * \param  [in] void   #
 * 
 * \retval static void
 */
static bool WireIPParaSetHandle(void)
{
  printf("file: %s, line: %d, function: %s !\n", __FILE__, __LINE__, __FUNCTION__);

  if(gstvEeNet.DhcpMode == 0x00)  /* �Զ���ȡ */
  {
    //system("udhcpc");
  }
  else
  {
    /* ��̬����IP */

    /* ����IP��ַ */

    /* ����Mac��ַ */

    /* ����Bcast��ַ */

    /* ����Mask��ַ */
  }

  printf("IpParaSet succeed!\r\n");
  
  gbitNet.DHCPBoot = 0x01;
  gstvNet.WiredStateBak = EnWsIpParaSet;
  
  return true;
}



/*!
 * \fn     WiredDHCPBootHandle
 * \brief  DHCP��ȡ�������
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

    /* һֱʧ�ܾ������������� */

    return false;
  }

  return true;
}


/**********************************************************************
 * Function:      GetIpAddress
 * Description:   ���ָ���˿ڵ�ip��ַ
 * Input:         ifaceName ������ӿ����� �磺eth0��IpAddress���õ���ip��ַ
 * Return:        �ɹ� 1��ʧ�� 0
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
 * \brief   ���������������״̬
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
����������ָ���������������������ֱ���ϵͳΪ C++ �� C ��Ԥ����ָ�����Ӧ������ָ��
 extern "C" ����ָʾ�������򣬴˴���"}"�������"{"���Ӧ��Ϊ����ָʾ�����ý�������
=======================================================================================*/
#ifdef __cplusplus
}
#endif


/*
*****************************************************************************************
                                 ��������Դ�����ļ����˽���
*****************************************************************************************
*/
