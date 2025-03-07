/*!
 ***************************************************************************************
 *
 *  Copyright ? 2017-2018 yifneg. All Rights Reserved.
 *
 * \file      NetDeal.h
 * \author    yifeng
 * \version   1.0
 * \date      2023年3月11日
 * \brief     网络处理头文件
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
#ifndef __NetDeal_H__
#define __NetDeal_H__


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
typedef enum 
{
  EnWsInit          = 0,        // 初始化
  EnWsLinkDet,                  // 检测网络链接
  EnWsIpParaSet,                // IP参数设置
  EnWsDHCP,                     // DHCP 
  EnWsPing,                     // ping 

  EnWsFail,                     // 失败
  EnWsFailDelay,                // 延时
  
  EnWsSucc,                     // 成功
}WiredStateEnum;


typedef struct 
{
  u16   LinkTiming1s;
  
  u8    LinkDetHave;
  u8    LinkDetLoss;

  u16   StateDelay;
  WiredStateEnum WiredState;   
  WiredStateEnum WiredStateBak;

  u8    ErrorTry;
}NetDealStru;

typedef struct 
{
  u8    InitBoot:       1;
  u8    LinkState:      1;
  u8    WiredOk:        1;
  u8    DHCPBoot:       1;
}NetDealBits;

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
extern NetDealStru gstvNet;
extern NetDealBits gbitNet;


/*=======================================================================================
                                 源程序文件中的函数原型声明
=======================================================================================*/





/*!
 * \fn     Start_NetDeal_Monitor_Thread
 * \brief  网络处理操作
 *          
 * \param  [in] void   #
 * 
 * \retval void
 */
void Start_NetDeal_Monitor_Thread(void);


/*!
 * \fn     WiredStateInputHandle
 * \brief   进入有线网络操作状态
 *          
 * \param  [in] WiredStateEnum WiredState   #
 * 
 * \retval void
 */
void WiredStateInputHandle(WiredStateEnum WiredState, unsigned char DelayMode);






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


