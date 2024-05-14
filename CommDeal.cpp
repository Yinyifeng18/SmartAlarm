/*!
 ***************************************************************************************
 *
 *  Copyright ? 2017-2018 yifneg. All Rights Reserved.
 *
 * \file      CommDeal.cpp
 * \author    yifeng
 * \version   1.0
 * \date      2023年3月11日
 * \brief     串口通信源程序
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
#include "CommDeal.h"

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
static void CommDeal_Init(void);

static void *CommDeal_Receive_Thread(void *arg);
static void *CommDeal_Send_Thread(void *arg);



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
CommDealStru gstvComm;
CommDealBits gbitComm;

typedef struct termios termios_t;

/*=======================================================================================
                             本源程序文件内部使用的局部变量定义                          
=======================================================================================*/




/*
*****************************************************************************************
                            　       程序指令代码清单
*****************************************************************************************
*/



/*!
 * \fn     Start_CommDeal_Monitor_Thread
 * \brief  
 *          
 * \param  [in] void   #
 * 
 * \retval void
 */
void Start_CommDeal_Monitor_Thread(void)
{
  pthread_t recv_tid;
  pthread_t send_tid;
  
  printf("file: %s, line: %d, function: %s !\n", __FILE__, __LINE__, __FUNCTION__);
  
  memset(&gbitComm, 0, sizeof(CommDealBits));
  memset(&gstvComm, 0, sizeof(CommDealStru));

  /* 串口初始化 */
  CommDeal_Init();
  
  if(pthread_create(&recv_tid, NULL, CommDeal_Receive_Thread, NULL) < 0)
  {
    perror("pthread_create: CommDeal_Receive_Thread");
  }
  
  
  if(pthread_create(&send_tid, NULL, CommDeal_Send_Thread, NULL) < 0)
  {
    perror("pthread_create: CommDeal_Send_Thread");
  }
}






/*!
 * \fn     CommDeal_Init
 * \brief  串口初始化
 *          
 * \param  [in] void   #
 * 
 * \retval static void
 */
static void CommDeal_Init(void)
{
  termios_t      *ter_s;
  
  ter_s = (termios_t *)malloc(sizeof(*ter_s));
  
  gstvComm.UsartFd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);//不成为控制终端程序，不受其他程序输出输出影响
  if(gstvComm.UsartFd < 0){
    printf("/dev/ttyUSB0 open faild\r\n");
    gbitComm.InitOk = SET;
    return ;
  }
  printf("open /dev/ttyUSB0 succeed\n");
   
  bzero(ter_s,sizeof(*ter_s));
   
  ter_s->c_cflag |= CLOCAL | CREAD; //激活本地连接与接受使能
   
  ter_s->c_cflag &= ~CSIZE;//失能数据位屏蔽
  ter_s->c_cflag |= CS8;//8位数据位
   
  ter_s->c_cflag &= ~CSTOPB;//1位停止位
   
  ter_s->c_cflag &= ~PARENB;//无校验位
   
  ter_s->c_cc[VTIME] = 0;
  ter_s->c_cc[VMIN] = 0;
   
  /* 1 VMIN> 0 && VTIME> 0
     VMIN为最少读取的字符数，当读取到一个字符后，会启动一个定时器，在定时器超时事前，如果已经读取到了VMIN个字符，则read返回VMIN个字符。如果在接收到VMIN个字符之前，定时器已经超时，则read返回已读取到的字符，注意这个定时器会在每次读取到一个字符后重新启用，即重新开始计时，而且是读取到第一个字节后才启用，也就是说超时的情况下，至少读取到一个字节数据。
     2 VMIN > 0 && VTIME== 0
     在只有读取到VMIN个字符时，read才返回，可能造成read被永久阻塞。
     3 VMIN == 0 && VTIME> 0
     和第一种情况稍有不同，在接收到一个字节时或者定时器超时时，read返回。如果是超时这种情况，read返回值是0。
     4 VMIN == 0 && VTIME== 0
     这种情况下read总是立即就返回，即不会被阻塞。----by 解释粘贴自博客园
  */
  cfsetispeed(ter_s, B115200);//设置输入波特率
  cfsetospeed(ter_s, B115200);//设置输出波特率
  printf("==> cfsetospeed\r\n");
  tcflush(gstvComm.UsartFd, TCIFLUSH);//刷清未处理的输入和/或输出
  printf("==> tcflush\r\n");
  if(tcsetattr(gstvComm.UsartFd, TCSANOW, ter_s) != 0)
  {
    printf("com set error!\r\n");
    gbitComm.InitOk = SET;
    return ;
  }

  gbitComm.InitOk = CLEAR;
  printf("file: %s, line: %d, function: %s !\n", __FILE__, __LINE__, __FUNCTION__);
}




/*!
 * \fn     CommDeal_Receive_Thread
 * \brief  
 *          
 * \param  [in] void *arg   #
 * 
 * \retval static void *
 */
static void *CommDeal_Receive_Thread(void *arg)
{
  printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);
  int i = 0;
  int ret = 0;
  char DataBuf[1024];
  
  pthread_detach(pthread_self());
   
  printf("file: %s, line: %d, function: %s !\n", __FILE__, __LINE__, __FUNCTION__);

  while(!gstvSysRun.quit)
  {
    /* 循环等待接收串口数据 */
    if(gbitComm.InitOk == CLEAR)
    {
      ret = read(gstvComm.UsartFd, DataBuf, 1024);
      if(ret > 0)
      {
        printf("Usart Receive: ");
        for(i = 0; i < ret; i++)
        {
          printf("0x%x ", DataBuf[i]);
        }
        printf("\r\n");
        
      }
    }
    
    usleep(1000);
  }
}


/*!
 * \fn     CommDeal_Send_Thread
 * \brief  
 *          
 * \param  [in] void *arg   #
 * 
 * \retval static void *
 */
static void *CommDeal_Send_Thread(void *arg)
{
  int ret = 0;
  int len = 0;
  int TimeTick = 0;
  unsigned char DataBuf[1024] = {0x00};
  
  printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);

  pthread_detach(pthread_self());

  while(!gstvSysRun.quit)
  {
    /* 每50毫秒处理一次要发送的数据 */
    if((TimeTick >= 3000) && (gbitComm.InitOk == CLEAR))
    {
      TimeTick = 0;
      memcpy(DataBuf, "Comm Test", sizeof("Comm Test"));
      len = sizeof("Comm Test");
      //ret = write(gstvComm.UsartFd, DataBuf, len);
      printf("CommDeal Write Data: Comm Test\n"); 
    }
    else
    {
      TimeTick++;
    }
    
    usleep(1000);
  }
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
