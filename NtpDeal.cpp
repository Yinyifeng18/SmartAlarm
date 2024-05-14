/*!
 ***************************************************************************************
 *
 *  Copyright ? 2017-2018 yifneg. All Rights Reserved.
 *
 * \file      NtpDeal.cpp
 * \author    yifeng
 * \version   1.0
 * \date      2023年3月11日
 * \brief     NTP处理源程序
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
#include "NtpDeal.h"
#include "TimerDeal.h"
#include "EepromDeal.h"


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
static void *NtpDeal_Thread(void *arg);
static int send_packet(int fd);
static int get_server_time(int sock, struct timeval *newtime);
static int mod_localtime(struct timeval newtime);

static void NtpDealRunTask(void);

static void NtpDealInputRunState(NtpStateEnum  NtpState);

/*=======================================================================================
                       本源程序文件内部使用的局部常量、字符化常数定义                    
=======================================================================================*/
/*---------------------------------------------------------------------------------------
的局部常量、字符化常数定义：
---------------------------------------------------------------------------------------*/
int sock;

int addr_len = sizeof(struct sockaddr_in);
struct sockaddr_in addr_src; //本地 socket  <netinet/in.h>
struct sockaddr_in addr_dst; //服务器 socket



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
NtpDealStru gstvNtp;
NtpDealBits gbitNtp;


/*=======================================================================================
                             本源程序文件内部使用的局部变量定义                          
=======================================================================================*/




/*
*****************************************************************************************
                            　       程序指令代码清单
*****************************************************************************************
*/


/*!
 * \fn     Start_NtpDeal_Monitor_Thread
 * \brief  网络时间获取
 *          
 * \param  [in] void   #
 * 
 * \retval void
 */
void Start_NtpDeal_Monitor_Thread(void)
{
  pthread_t ntp_tidp;
  
  printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);
  
  if(pthread_create(&ntp_tidp, NULL, NtpDeal_Thread, NULL) != 0)
  {
    perror("pthread_create: NtpDeal_Thread");
    return ;
  }
}

/*!
 * \fn     NtpDealRestartHandle
 * \brief  重启Ntp
 *          
 * \param  [in] void   #
 * 
 * \retval void
 */
void NtpDealRestartHandle(void)
{
  /* 先关闭sock */
  if(gbitNtp.HaveSocket == SET)
  {
    gbitNtp.HaveSocket = CLEAR;
    close(sock);
  }
  
  memset(&gstvNtp, 0, sizeof(NtpDealStru));
  memset(&gbitNtp, 0, sizeof(NtpDealBits));

  NtpDealInputRunState(EnNrInit);
}


/*!
 * \fn     NtpDeal_Thread
 * \brief  定时获取网络时间
 *          
 * \param  [in] void *arg   #
 * 
 * \retval static void *
 */
static void *NtpDeal_Thread(void *arg)
{
  printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);

  pthread_detach(pthread_self());

  memset(&gstvNtp, 0, sizeof(NtpDealStru));
  memset(&gbitNtp, 0, sizeof(NtpDealBits));

  while(!gstvSysRun.quit)
  {   
    /* 自动获取时间 */
    gstvEeNtp.NtpMode = '0';
    if(gstvEeNtp.NtpMode == '0')
    {
      NtpDealRunTask();
    }
    else 
    {
    }
    
    sleep(1);
  }

  pthread_exit(0);
}

/*!
 * \fn     send_packet
 * \brief  发送数据包
 *          
 * \param  [in] int fd   #
 * 
 * \retval static int
 */
static int send_packet(int fd)
{
  printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);

  unsigned int data[12];
  int ret;
  struct timeval now;
  if (sizeof(data) != 48)
  {
    PDEBUG("data 长度小于48!");
    exit(1);
  }
  memset((char *)data, 0, sizeof(data));
  data[0] = htonl((LI << 30) | (VN << 27) | (MODE << 24) | (STRATUM << 16) | (POLL << 8) | (PREC & 0xff));
  data[1] = htonl(1 << 16);
  data[2] = htonl(1 << 16);
  //获得本地时间
  gettimeofday(&now, NULL);

  data[10] = htonl(now.tv_sec + JAN_1970);
  data[11] = htonl(NTPFRAC(now.tv_usec));

  ret = send(fd, data, 48, 0);
  PDEBUG("发送请求数据包到服务器, ret: %d\n", ret);
}

/*!
 * \fn     get_server_time
 * \brief  获取服务器时间
 *          
 * \param  [in] int sock                  #
 * \param  [in] struct timeval *newtime   #
 * 
 * \retval static int
 */
static int get_server_time(int sock, struct timeval *newtime)
{
  printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);

  int ret;
  unsigned int data[12];
  NtpTime oritime, rectime, tratime, destime;
  struct timeval offtime, dlytime, new_1;
  struct timeval now;
  
  bzero(data, sizeof(data));
  ret = recvfrom(sock, data, sizeof(data), 0, NULL, 0);
  if (ret == -1)
  {
    PDEBUG("读取返回数据失败\n");
    exit(1);
  }
  else if (ret == 0)
  {
    PDEBUG("读取到速度长度: 0!\n");
    return 1;
  }

  //1970逆转换到1900
  gettimeofday(&now, NULL);
  destime.integer = now.tv_sec + JAN_1970;
  destime.fraction = NTPFRAC(now.tv_usec);

  //字节序转换
  oritime.integer = DATA(6);
  oritime.fraction = DATA(7);
  rectime.integer = DATA(8);
  rectime.fraction = DATA(9);
  tratime.integer = DATA(10);
  tratime.fraction = DATA(11);

  long long orius, recus, traus, desus, offus, dlyus;

  orius = TTLUSEC(MKSEC(oritime), MKUSEC(oritime));
  recus = TTLUSEC(MKSEC(rectime), MKUSEC(rectime));
  traus = TTLUSEC(MKSEC(tratime), MKUSEC(tratime));
  desus = TTLUSEC(now.tv_sec, now.tv_usec);

  offus = ((recus - orius) + (traus - desus)) / 2;
  dlyus = (recus - orius) + (desus - traus);

  offtime.tv_sec = GETSEC(offus);
  offtime.tv_usec = GETUSEC(offus);
  dlytime.tv_sec = GETSEC(dlyus);
  dlytime.tv_usec = GETUSEC(dlyus);

  new_1.tv_sec = destime.integer - JAN_1970 + offtime.tv_sec;
  new_1.tv_usec = USEC(destime.fraction) + offtime.tv_usec;

  //提取现在好的时间
  *newtime = new_1;
  return 0;
}


/*!
 * \fn     mod_localtime
 * \brief  更新本地时间
 *          
 * \param  [in] struct timeval newtime   #
 * 
 * \retval int
 */
static int mod_localtime(struct timeval newtime)
{
  printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);

  struct tm *tm_p;
  char time_buff[100];
  memset(time_buff, 0, 100);
  time_t time_sec = newtime.tv_sec + 28800;
  tm_p = gmtime(&time_sec);

  sprintf(time_buff, "date -s \"%d-%d-%d %d:%d:%d\"", tm_p->tm_year + 1900, tm_p->tm_mon + 1, tm_p->tm_mday, tm_p->tm_hour, tm_p->tm_min, tm_p->tm_sec);
  printf("mod_localtime: %s\n", time_buff);
  //system(time_buff);

  return 0;
}


/*!
 * \fn     NtpDealRunTask
 * \brief  Ntp运行任务
 *          
 * \param  [in] void   #
 * 
 * \retval static void
 */
static void NtpDealRunTask(void)
{
  struct timeval newtime;
  struct timeval timeout; //<sys/time.h>
  fd_set fds_read;
  int ret;

  switch(gstvNtp.State)
  {
    case EnNrInit:
    {
      printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);
      memset(&addr_dst, 0, addr_len);
      addr_dst.sin_family = AF_INET;
      addr_dst.sin_port = htons(DEF_NTP_PORT);
      addr_dst.sin_addr.s_addr = inet_addr(DEF_NTP_SERVER_IP);

      NtpDealInputRunState(EnNrConnect);
      
      //UDP数据报套接字
      sock = socket(PF_INET, SOCK_DGRAM, 0);
      if (sock == -1)
      {
        PDEBUG("套接字创建失败，被迫终止 ! \n");
        NtpDealInputRunState(EnNrFail);
      }
      else
      {
        gbitNtp.HaveSocket = 0x01;
      }
      
      memset(&addr_src, 0, addr_len);
      addr_src.sin_family = AF_INET;
      addr_src.sin_port = htons(0);
      addr_src.sin_addr.s_addr = htonl(INADDR_ANY); //<arpa/inet.h>
      //绑定本地地址
      if (-1 == bind(sock, (struct sockaddr *)&addr_src, addr_len))
      {
        PDEBUG("绑定失败，被迫终止 !\n");
        NtpDealInputRunState(EnNrFail);
      }

      break;
    }
    case EnNrConnect:
    {
      printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);
      if (-1 == connect(sock, (struct sockaddr *)&addr_dst, addr_len))
      {
        PDEBUG("连接服务器失败，被迫终止 !\n");
        NtpDealInputRunState(EnNrFail);
      }
      else
      {
        send_packet(sock);
        NtpDealInputRunState(EnNrRun);
        gbitNtp.RefreshNtp = SET;
      }
      break;
    }
    case EnNrRun:
    {
      printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);
      if((TimerGet1sSubb(gstvNtp.Delay, 3600))
      ||(gbitNtp.RefreshNtp == SET))
      {
        gbitNtp.RefreshNtp = CLEAR;        
        gstvNtp.Delay = TimerGet1sInc();
        
        FD_ZERO(&fds_read);
        FD_SET(sock, &fds_read);
  
        timeout.tv_sec = 30;
        timeout.tv_usec = 0;
        
        //发送 ntp 包
        send_packet(sock);
  
        ret = select(sock + 1, &fds_read, NULL, NULL, &timeout);
        if (ret == -1)
        {
          PDEBUG("select函数出错，被迫终止 !\n");
          NtpDealInputRunState(EnNrFail);
        }
        if (ret == 0)
        {
          PDEBUG("等待服务器响应超时，重发请求 !\n");
          //向服务器发送数据
          send_packet(sock);

          gstvNtp.ErrorCnt++;
          if(gstvNtp.ErrorCnt > 2)
          {
            gstvNtp.ErrorCnt = 0;
            NtpDealRestartHandle();
          }
        }
        else
        {
          if (FD_ISSET(sock, &fds_read))
          {
            if (1 == get_server_time(sock, &newtime))
            {
             
            }
            else
            {
              mod_localtime(newtime);
            }
          }
        }
      }
      break;
    }
    case EnNrFail:
    {
      printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);
      if(gbitNtp.HaveSocket == SET)
      {
        gbitNtp.HaveSocket = CLEAR;
        close(sock);
      }

      NtpDealInputRunState(EnNrFailDelay);
      break;
    }
    case EnNrFailDelay:
    {
      if(TimerGet1sSubb(gstvNtp.Delay, 600*30))
      {
        NtpDealInputRunState(EnNrInit);
      }

      break;
    }
    default:
    {
      NtpDealInputRunState(EnNrInit);
      break;
    }
  }
}

/*!
 * \fn     NtpDealInputRunState
 * \brief  進入操作状态
 *          
 * \param  [in] NtpStateEnum  NtpState   #
 * 
 * \retval static void
 */
static void NtpDealInputRunState(NtpStateEnum  NtpState)
{
  gstvNtp.State = NtpState;

  gstvNtp.Delay = TimerGet1sInc();
  printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);
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
