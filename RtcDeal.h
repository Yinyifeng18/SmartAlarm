/*!
 ***************************************************************************************
 *
 *  Copyright ? 2017-2018 yifneg. All Rights Reserved.
 *
 * \file      RtcDeal.h
 * \author    yifeng
 * \version   1.0
 * \date      2023年3月11日
 * \brief     Rtc处理头文件
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


/*=======================================================================================
下面两条指令是本头文件条件编译的预处理指令，用于防止本头文件被其他文件重复引用。
=======================================================================================*/
#ifndef __RtcDeal_H__
#define __RtcDeal_H__

/*========================================================================================
                                 本源程序包括的头文件                                     
建议：包含本项目的文件使用 #include "文件名.扩展名"，                                   
      包含系统库的文件使用 #include <文件名.扩展名> 。                                    
========================================================================================*/
#include "Main.h"

/*=======================================================================================
下面这三条指令是用于区分编译系统为 C++ 或 C 的预处理指令。                               
"__cplusplus" 为 C++ 预处理器名字，这三条指令表明接在其下面的那些指令为 C 程序特征.      
=======================================================================================*/
#ifdef __cplusplus
extern "C" {
#endif


/*=======================================================================================
                               系统运行的各类字符化常数定义                             
=======================================================================================*/
/*---------------------------------------------------------------------------------------
为了做到较好的软件可移植性，推荐用户在编程时使用下面定义的数据类型大小
---------------------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------------------
的字符化常数定义：
----------------------------------------------------------------------------------------*/
#define cRtcDateLen     11
#define cRtcTimeLen     9

/*=======================================================================================
                      各类字符化常数和硬件设计相关的输入／输出引脚定义
=======================================================================================*/
/*--------------------------------------------------------------------------------------
的输入／输出引脚定义：
--------------------------------------------------------------------------------------*/


/*======================================================================================
                             全局变量数据结构类型定义
======================================================================================*/
/*--------------------------------------------------------------------------------------
的全局数据结构类型定义 ：  
--------------------------------------------------------------------------------------*/
typedef struct  
{  
  char Date[cRtcDateLen];              // 日期    "2023-03-11"
  char Time[cRtcTimeLen];              // 时间    "16:54:30"
}RtcDealStru;  


/*=======================================================================================
                                         全局常量声明                                    
=======================================================================================*/
/*---------------------------------------------------------------------------------------
的全局常量声明：
---------------------------------------------------------------------------------------*/


/*=======================================================================================
                                         全局变量声明                                    
=======================================================================================*/
/*---------------------------------------------------------------------------------------
的全局变量声明：
---------------------------------------------------------------------------------------*/
extern RtcDealStru  gstvRtc;

/*=======================================================================================
                                 源程序文件中的函数原型声明
=======================================================================================*/




/*!
 * \fn     Start_RtcDeal_Monitor_Thread
 * \brief  定时器处理
 *          
 * \param  [in] void   #
 * 
 * \retval void
 */
void Start_RtcDeal_Monitor_Thread(void);



/*!
 * \fn     Rtc_GetSysTime
 * \brief  获取系统时间
 *          
 * \param  [in] void   #
 * 
 * \retval void
 */
void RtcGetSysTime(void);



/*!
 * \fn     RtcSetSysTime
 * \brief  设置系统时间
 *          
 * \param  [in] char *Date   # 2023-03-11
 * \param  [in] char *Time   # 17:00:00
 * 
 * \retval void
 */
void RtcSetSysTime(char *Date, char *Time);




/*=======================================================================================
下面这三条指令是用于与上面三条区分编译系统为 C++ 或 C 的预处理指令相对应。用于指定
 extern "C" 链接指示符作用域，此处的"}"与上面的"{"相对应，为链接指示符作用结束符。
=======================================================================================*/
#ifdef __cplusplus
}
#endif

#endif
/*
*****************************************************************************************
                                 本Ｃ语言源程序文件到此结束
*****************************************************************************************
*/


