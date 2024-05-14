/*!
 ***************************************************************************************
 *
 *  Copyright ? 2017-2018 yifneg. All Rights Reserved.
 *
 * \file      EepromDeal.cpp
 * \author    yifeng
 * \version   1.0
 * \date      2023年3月11日
 * \brief     Eeprom处理源程序
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
#include "EepromDeal.h"
#include "iniparser.h"

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
EeNetStru     gstvEeNet;
EeNtpStru     gstvEeNtp;
EeStorageStru gstvEeStorage;
EeAiStru      gstvEeAi;
EeAoStru      gstvEeAo;
EeSysStru     gstvEeSys;



/*=======================================================================================
                             本源程序文件内部使用的局部变量定义                          
=======================================================================================*/




/*
*****************************************************************************************
                            　       程序指令代码清单
*****************************************************************************************
*/


/*!
 * \fn     EepromDeal_Init
 * \brief  初始化Eeprom
 *          
 * \param  [in] void   #
 * 
 * \retval void
 */
void EepromDeal_Init(void)
{  
  dictionary  *ini=NULL;
  
  ini = iniparser_load(cEepromIniPath);
  if( ini ==NULL)
  {
    printf("iniparser_load failure\n");
    /* 出错，复位数据 */
    EeResetEepromData();
  } 

  EeReadSysParamToSram(); 
  EeReadNetParamToSram();
  EeReadNtpParamToSram();
  EeReadStorageParamToSram();
  EeReadAiParamToSram();
  EeReadAoParamToSram();

  EePromSetSysParam();

  
}


/*!
 * \fn     EeReadSysParamToSram
 * \brief  读取系统参数
 *          
 * \param  [in] void   #
 * 
 * \retval void
 */
void EeReadSysParamToSram(void)
{
  dictionary  *ini=NULL;
  
  ini = iniparser_load(cEepromIniPath);
  if( ini ==NULL)
  {
    printf("iniparser_load failure\n");
    /* 出错，复位数据 */
    return ;
  } 
  
  gstvEeSys.Language = iniparser_getint(ini, "SystemParam:Language", 0);
  gstvEeSys.ArmDisarm = iniparser_getint(ini, "SystemParam:ArmDisarm", 0);
  
  iniparser_freedict(ini);

  printf("gstvEeSys.Language: %d\n", gstvEeSys.Language);
  printf("gstvEeSys.ArmDisarm: %d\n", gstvEeSys.ArmDisarm);
}

/*!
 * \fn     EeReadNetParamToSram
 * \brief   读取网络参数
 *          
 * \param  [in] void   #
 * 
 * \retval void
 */
void EeReadNetParamToSram(void)
{
  dictionary  *ini=NULL;
  
  ini = iniparser_load(cEepromIniPath);
  if( ini ==NULL)
  {
    printf("iniparser_load failure\n");
    /* 出错，复位数据 */
    return ;
  } 

  gstvEeNet.DhcpMode = iniparser_getint(ini, "NetParam:DhcpMode", 0);

  iniparser_freedict(ini);

  printf("gstvEeNet.DhcpMode: %d\n", gstvEeNet.DhcpMode);
}


/*!
 * \fn     EeReadNtpParamToSram
 * \brief   读取NTP参数
 *          
 * \param  [in] void   #
 * 
 * \retval void
 */
void EeReadNtpParamToSram(void)
{
  dictionary  *ini=NULL;
  
  ini = iniparser_load(cEepromIniPath);
  if( ini ==NULL)
  {
    printf("iniparser_load failure\n");
    /* 出错，复位数据 */
    return ;
  } 

  gstvEeNtp.NtpMode = iniparser_getint(ini, "NetParam:NtpMode", 0);

  iniparser_freedict(ini);

  printf("gstvEeNtp.NtpMode: %d\n", gstvEeNtp.NtpMode);
}


/*!
 * \fn     EeReadStorageParamToSram
 * \brief   读取参数
 *          
 * \param  [in] void   #
 * 
 * \retval void
 */
void EeReadStorageParamToSram(void)
{
  dictionary  *ini=NULL;
  
  ini = iniparser_load(cEepromIniPath);
  if( ini ==NULL)
  {
    printf("iniparser_load failure\n");
    /* 出错，复位数据 */
    return ;
  } 

  gstvEeStorage.SaveSw = iniparser_getint(ini, "StorageParam:SaveSw", 0);
  gstvEeStorage.RecMode = iniparser_getint(ini, "StorageParam:RecMode", 0);

  iniparser_freedict(ini);

  printf("gstvEeStorage.SaveSw: %d\n", gstvEeStorage.SaveSw);
  printf("gstvEeStorage.RecMode: %d\n", gstvEeStorage.RecMode);
}


/*!
 * \fn     EeReadAiParamToSram
 * \brief   读取参数
 *          
 * \param  [in] void   #
 * 
 * \retval void
 */
void EeReadAiParamToSram(void)
{
  dictionary  *ini=NULL;
  
  ini = iniparser_load(cEepromIniPath);
  if( ini ==NULL)
  {
    printf("iniparser_load failure\n");
    /* 出错，复位数据 */
    return ;
  } 

  gstvEeAi.Vol = iniparser_getint(ini, "AiParam:Vol", 0);
  gstvEeAi.Aogain = iniparser_getint(ini, "AiParam:Aogain", 0);

  iniparser_freedict(ini);

  printf("gstvEeAi.Vol: %d\n", gstvEeAi.Vol);
  printf("gstvEeAi.Aogain: %d\n", gstvEeAi.Aogain);
}

/*!
 * \fn     EeReadAoParamToSram
 * \brief   读取参数
 *          
 * \param  [in] void   #
 * 
 * \retval void
 */
void EeReadAoParamToSram(void)
{
  dictionary  *ini=NULL;
  
  ini = iniparser_load(cEepromIniPath);
  if( ini ==NULL)
  {
    printf("iniparser_load failure\n");
    /* 出错，复位数据 */
    return ;
  } 

  gstvEeAo.Vol = iniparser_getint(ini, "AoParam:Vol", 0);
  gstvEeAo.Aogain = iniparser_getint(ini, "AoParam:Aogain", 0);

  iniparser_freedict(ini);

  printf("gstvEeAo.Vol: %d\n", gstvEeAo.Vol);
  printf("gstvEeAo.Aogain: %d\n", gstvEeAo.Aogain);
}


/*!
 * \fn     EeResetEepromData
 * \brief  恢复出厂
 *          
 * \param  [in] void   #
 * 
 * \retval void
 */
void EeResetEepromData(void)
{
  
}


/*=======================================================================================
                         以下函数为参数独立设置及查询                                    
=======================================================================================*/


/*!
 * \fn     EePromSetSysParam
 * \brief  设置系统参数
 *          
 * \param  [in] void   #
 * 
 * \retval void
 */
void EePromSetSysParam(void)
{
  dictionary  *ini = NULL;
  FILE        *fp  = NULL;
  char ArmDisarm[1] = {0}; 
  char Language[1] = {0};
  
  ini = iniparser_load(cEepromIniPath);
  if( ini ==NULL)
  {
    printf("iniparser_load failure\n");
    /* 出错，复位数据 */
    return ;
  } 

  Language[0] = gstvEeSys.Language + '0';
  ArmDisarm[0] = gstvEeSys.ArmDisarm + '0';
  iniparser_set(ini, "SystemParam:Language", "3");
  iniparser_set(ini, "SystemParam:ArmDisarm", "4");
  
  //写入文件
  fp = fopen(cEepromIniPath, "w");
  if( fp == NULL)
  {
    printf("stone:fopen error!\n");
    exit(-1);
  }

  iniparser_dump_ini(ini, fp);
  
  fclose(fp);

  iniparser_freedict(ini);
}

/*!
 * \fn     EepromSetNetParam
 * \brief  设置网络参数
 *          
 * \param  [in] void   # 
 * 
 * \retval void
 */
void EepromSetNetParam(void)
{
  
}

/*!
 * \fn     EepromSetNtpParam
 * \brief  设置参数
 *          
 * \param  [in] void   # 
 * 
 * \retval void
 */
void EepromSetNtpParam(void)
{
  
}

/*!
 * \fn     EepromSetStorageParam
 * \brief  设置参数
 *          
 * \param  [in] void   # 
 * 
 * \retval void
 */
void EepromSetStorageParam(void)
{
  
}


/*!
 * \fn     EepromSetAiParam
 * \brief  设置参数
 *          
 * \param  [in] void   # 
 * 
 * \retval void
 */
void EepromSetAiParam(void)
{
  
}


/*!
 * \fn     EepromSetAoParam
 * \brief  设置参数
 *          
 * \param  [in] void   # 
 * 
 * \retval void
 */
void EepromSetAoParam(void)
{
  
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
