/*!
 ***************************************************************************************
 *
 *  Copyright ? 2017-2018 yifneg. All Rights Reserved.
 *
 * \file      SnapStorageDeal.cpp
 * \author    yifeng
 * \version   1.0
 * \date      2023年3月11日
 * \brief     存储处理源程序
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
#include "SnapStorageDeal.h"
#include "EepromDeal.h"
#include "FileDeal.h"
#include "DiskDeal.h"
#include "RtcDeal.h"
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
static void *SnapStorageDeal_Thread(void *arg);

static void StorageDealHandle(void);

static void TimePathConv(void);

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
SnapStorageStru gstvSnapStorage;
SnapStorageBits gbitSnapStorage;


/*=======================================================================================
                             本源程序文件内部使用的局部变量定义                          
=======================================================================================*/




/*
*****************************************************************************************
                            　       程序指令代码清单
*****************************************************************************************
*/

/*!
 * \fn     Start_SnapStorageDeal_Monitor_Thread
 * \brief  mqtt处理操作
 *          
 * \param  [in] void   #
 * 
 * \retval void
 */
void Start_SnapStorageDeal_Monitor_Thread(void)
{
  pthread_t SnapStorage_tidp;
  
  printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);
  
  if (pthread_create(&SnapStorage_tidp, NULL, SnapStorageDeal_Thread, NULL) != 0)
  {
    perror("pthread_create: SnapStorageDeal_Thread");
    return;
  }
}


/*!
 * \fn     SnapStorageDeal_Thread
 * \brief  线程处理
 *          
 * \param  [in] void *arg   #
 * 
 * \retval static void *
 */
static void *SnapStorageDeal_Thread(void *arg)
{
  printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);

  int ret;
  
  pthread_detach(pthread_self());

  while(!gstvSysRun.quit)
  {
    StorageDealHandle();
    usleep(1000);
  }

  pthread_exit(0);
}



/*!
 * \fn     StorageDealHandle
 * \brief  处理
 *          
 * \param  [in] void   #
 * 
 * \retval static void
 */
static void StorageDealHandle(void)
{
  char  cmd[cStreamPathLen];
  
  gstvEeStorage.SaveSw = 0x01;
  if(gstvEeStorage.SaveSw == 0x00)
    return ;

  /* 一直使能 */
  gstvEeStorage.RecMode = 0x01;
  if(gstvEeStorage.RecMode == 0x01)
  {
    printf("gstvSnapStorage.Delay: %d\n", gstvSnapStorage.Delay);
    
    if((TimerGet1sSubb(gstvSnapStorage.Delay, cSaveTime))
    &&(gbitSnapStorage.RecNow == CLEAR))
    {
      gstvSnapStorage.Delay = TimerGet1sInc();
      
      printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);
      /* 获取数据 */
      sprintf(cmd, "%s%s", cDefaultDiskPath, (char *)&gstvRtc.Date[0]);
      if(FileDealCreateDir(cmd) == 0)
      {
        memset((char *)&gstvSnapStorage.StreamPath[0], 0, cStreamPathLen); 
        memcpy((char *)&gstvSnapStorage.Date[0], (char *)&gstvRtc.Date[0], cStreamDateLen);
        memcpy((char *)&gstvSnapStorage.Time[0], (char *)&gstvRtc.Time[0], cStreamTimeLen);
        TimePathConv();
        sprintf((char *)&gstvSnapStorage.StreamPath[0], "%s%s/%s_%s.h264", cDefaultDiskPath, (char *)&gstvSnapStorage.Date[0], (char *)&gstvSnapStorage.Date[0], (char *)&gstvSnapStorage.Time[0]);
        printf("StreamPath: %s\n", (char *)&gstvSnapStorage.StreamPath[0]);
        gstvSnapStorage.StreamFd = open((char *)&gstvSnapStorage.StreamPath[0], O_RDWR | O_CREAT | O_TRUNC, 0777);
        if (gstvSnapStorage.StreamFd < 0) 
        {
          printf("failed: %s\n", (char *)&gstvSnapStorage.StreamPath[0]);
          close(gstvSnapStorage.StreamFd);
          gstvSnapStorage.SaveTime = 0;
          gbitSnapStorage.StreamFdOk = CLEAR;
        }
        else
        {
          printf("StreamPath open OK\n");
          
          gstvSnapStorage.SaveTime = 0;
          gbitSnapStorage.RecNow = SET;
          gbitSnapStorage.StreamFdOk = SET;
        }
      }
    }
  }
  else if(gstvEeStorage.RecMode == 0x00)
  {
    if((gbitSnapStorage.MoveDet == SET)
    &&(gbitSnapStorage.RecNow == CLEAR))
    {
      gbitSnapStorage.MoveDet = CLEAR;
      /* 获取数据 */
      sprintf(cmd, "%s%s", cDefaultDiskPath, (char *)&gstvRtc.Date[0]);
      if(FileDealCreateDir(cmd) == 0)
      {
        memset((char *)&gstvSnapStorage.StreamPath[0], 0, cStreamPathLen); 
        memcpy((char *)&gstvSnapStorage.Date[0], (char *)&gstvRtc.Date[0], cStreamDateLen);
        memcpy((char *)&gstvSnapStorage.Time[0], (char *)&gstvRtc.Time[0], cStreamTimeLen);
        TimePathConv();
        sprintf((char *)&gstvSnapStorage.StreamPath[0], "%s%s/%s_%s.h264", cDefaultDiskPath, (char *)&gstvSnapStorage.Date[0], (char *)&gstvSnapStorage.Date[0], (char *)&gstvSnapStorage.Time[0]);
        printf("StreamPath: %s\n", (char *)&gstvSnapStorage.StreamPath[0]);
        gstvSnapStorage.StreamFd = open((char *)&gstvSnapStorage.StreamPath[0], O_RDWR | O_CREAT | O_TRUNC, 0777);
        if (gstvSnapStorage.StreamFd < 0) 
        {
          printf("failed: %s\n", (char *)&gstvSnapStorage.StreamPath[0]);
          close(gstvSnapStorage.StreamFd);
          gstvSnapStorage.SaveTime = 0;
          gbitSnapStorage.StreamFdOk = CLEAR;
        }
        else
        {
          printf("StreamPath open OK\n");
          
          gstvSnapStorage.SaveTime = 0;
          gbitSnapStorage.RecNow = SET;
          gbitSnapStorage.StreamFdOk = SET;
        }
      }
    }
    
  }
  else if(gstvEeStorage.RecMode == 0x02)
  {
    /* 定时模式 */
    
  }
  else
  {
  }

  if(TimerGet1sSubb(gstvSnapStorage.SaveTime, cSaveTime))
  {
    gstvSnapStorage.SaveTime = TimerGet1sInc();
    
    if(gbitSnapStorage.StreamFdOk == SET)
    {
      gbitSnapStorage.StreamFdOk = CLEAR;
      gbitSnapStorage.RecNow     = CLEAR;
      close(gstvSnapStorage.StreamFd);
      printf("close streamFd\n");
    }
  }
}



/*!
 * \fn     TimePathConv
 * \brief  字符转换
 *          
 * \param  [in] void   #
 * 
 * \retval static void
 */
static void TimePathConv(void)
{
  u8  i = 0;

  for(i = 0; i < cStreamTimeLen; i++)
  {
    if(gstvSnapStorage.Time[i] == ':')
      gstvSnapStorage.Time[i] = '-';
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
