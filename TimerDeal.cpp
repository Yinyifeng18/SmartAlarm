/*!
 ***************************************************************************************
 *
 *  Copyright ? 2017-2018 yifneg. All Rights Reserved.
 *
 * \file      TimerDeal.cpp
 * \author    yifeng
 * \version   1.0
 * \date      2023年3月11日
 * \brief     定时器处理头文件
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
#include "TimerDeal.h"
#include "RtcDeal.h"

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
static void *TimerDeal_Thread(void *arg);

static void microseconds_sleep(unsigned long uSec);


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
TimerIncStru gstvTimerInc;
TimerCntStru gstvTimerCnt;


/*=======================================================================================
                             本源程序文件内部使用的局部变量定义                          
=======================================================================================*/




/*
*****************************************************************************************
                            　       程序指令代码清单
*****************************************************************************************
*/

/*!
 * \fn     Start_TimerDeal_Monitor_Thread
 * \brief  定时器处理
 *          
 * \param  [in] void   #
 * 
 * \retval void
 */
void Start_TimerDeal_Monitor_Thread(void)
{
  pthread_t timer_tidp;

  printf("file: %s, line: %d, function: %s !\n", __FILE__, __LINE__, __FUNCTION__);
  
  if(pthread_create(&timer_tidp, NULL, TimerDeal_Thread, NULL) != 0)
  {
    perror("pthread_create: TimerDeal_Thread");
    return;
  }
  
}



/*!
 * \fn     TimerDeal_Thread
 * \brief  定时器线程
 *          
 * \param  [in] void *arg   #
 * 
 * \retval static void *
 */
static void *TimerDeal_Thread(void *arg)
{
  printf("file: %s, line: %d, function: %s !\n", __FILE__, __LINE__, __FUNCTION__);
  
  pthread_detach(pthread_self());

  
  /* 初始化参数 */
  memset(&gstvTimerInc, 0, sizeof(TimerIncStru));
  memset(&gstvTimerCnt, 0, sizeof(TimerCntStru));
  
  while(!gstvSysRun.quit)
  {
    TimerUpdataIsr();

    microseconds_sleep(750);
  }

  pthread_exit(0);
}

/*!
 * \fn     microseconds_sleep
 * \brief  微妙级别定时器
 *          
 * \param  [in] unsigned long mSec   #
 * 
 * \retval static void
 */
static void microseconds_sleep(unsigned long uSec)
{
  struct timeval tv;
  tv.tv_sec = uSec/1000000;
  tv.tv_usec = uSec%1000000;

  int err;

  do {
    err = select(0, NULL, NULL, NULL, &tv);
  }while(err < 0 && errno == EINTR);
}


/*!
 * \fn     TimerGet1msInc
 * \brief  读取1ms计时器数值。
 *          
 * \param  [in] void   #
 * 
 * \retval u16
 */
u16 TimerGet1msInc(void)
{
  return(gstvTimerInc.Inc1msCnt);
}

/*!
 * \fn     TimerGet1sInc
 * \brief  读取1s计时器数值。
 *          
 * \param  [in] void   #
 * 
 * \retval u16
 */
u16 TimerGet1sInc(void)
{
  return(gstvTimerInc.Inc1sCnt);
}

/*!
 * \fn     TimerGet1minInc
 * \brief  读取1min计时器数值。
 *          
 * \param  [in] void   #
 * 
 * \retval u16
 */
u16 TimerGet1minInc(void)
{
  return(gstvTimerInc.Inc1minCnt);
}

/*!
 * \fn     TimerGet1msSubb
 * \brief  计算与1ms计时器差值。
 *          
 * \param  [in] u16 MsCnt     #
 * \param  [in] u16 SubbVal   #
 * 
 * \retval bool
 */
bool TimerGet1msSubb(u16 MsCnt, u16 SubbVal)
{
  u16 Count;
  u16 Value;
  u16 Subb;
 
  Count  = MsCnt;
  Value  = SubbVal;
  Subb   = gstvTimerInc.Inc1msCnt - Count;
  if(Subb >= Value)
  {
    return(TRUE);
  }
  else
  {
    return(FALSE);
  }
}

/*!
 * \fn     TimerGet1sSubb
 * \brief  计算与1s计时器差值。
 *          
 * \param  [in] u16 SecCnt    #
 * \param  [in] u16 SubbVal   #
 * 
 * \retval bool
 */
bool TimerGet1sSubb(u16 SecCnt, u16 SubbVal)
{
  u16 Count;
  u16 Value;
  u16 Subb;
  
  Count  = SecCnt;
  Value  = SubbVal;
  Subb   = gstvTimerInc.Inc1sCnt - Count;
  if(Subb >= Value)
  {
    return(TRUE);
  }
  else
  {
    return(FALSE);
  }
}


/*!
 * \fn     TimerGet1minSubb
 * \brief  计算与1min计时器差值。
 *          
 * \param  [in] u16 MinCnt    #
 * \param  [in] u16 SubbVal   #
 * 
 * \retval bool
 */
bool TimerGet1minSubb(u16 MinCnt, u16 SubbVal)
{
  u16 Count;
  u16 Value;
  u16 Subb;
 
  Count  = MinCnt;
  Value  = SubbVal;
  Subb   = gstvTimerInc.Inc1minCnt - Count;
  
  if(Subb >= Value)
  {
    return(TRUE);
  }
  else
  {
    return(FALSE);
  }
}

/*!
 * \fn     TimerUpdataIsr
 * \brief  中断程序
 *          
 * \param  [in] void   #
 * 
 * \retval void
 */
void TimerUpdataIsr(void)
{
  gstvTimerInc.Inc1msCnt++;
  gstvTimerCnt.Timing1sCnt++;
  if(gstvTimerCnt.Timing1sCnt >= 1000)
  {
    gstvTimerCnt.Timing1sCnt = 0;
    gstvTimerInc.Inc1sCnt++;
    gstvTimerCnt.Timing1minCnt++;

    printf("Timer second: %d\n", gstvTimerInc.Inc1sCnt);
    if(gstvTimerCnt.Timing1minCnt >= 60)
    {
      gstvTimerCnt.Timing1minCnt = 0;
      gstvTimerInc.Inc1minCnt++;
    }
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
