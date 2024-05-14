/*!
 ***************************************************************************************
 *
 *  Copyright ? 2017-2018 yifneg. All Rights Reserved.
 *
 * \file      EepromDeal.cpp
 * \author    yifeng
 * \version   1.0
 * \date      2023��3��11��
 * \brief     Eeprom����Դ����
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
#include "EepromDeal.h"
#include "iniparser.h"

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


/*=======================================================================================
                             ��Դ�����ļ��ڲ�ʹ�õĺ���ԭ������                          
=======================================================================================*/


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
EeNetStru     gstvEeNet;
EeNtpStru     gstvEeNtp;
EeStorageStru gstvEeStorage;
EeAiStru      gstvEeAi;
EeAoStru      gstvEeAo;
EeSysStru     gstvEeSys;



/*=======================================================================================
                             ��Դ�����ļ��ڲ�ʹ�õľֲ���������                          
=======================================================================================*/




/*
*****************************************************************************************
                            ��       ����ָ������嵥
*****************************************************************************************
*/


/*!
 * \fn     EepromDeal_Init
 * \brief  ��ʼ��Eeprom
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
    /* ������λ���� */
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
 * \brief  ��ȡϵͳ����
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
    /* ������λ���� */
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
 * \brief   ��ȡ�������
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
    /* ������λ���� */
    return ;
  } 

  gstvEeNet.DhcpMode = iniparser_getint(ini, "NetParam:DhcpMode", 0);

  iniparser_freedict(ini);

  printf("gstvEeNet.DhcpMode: %d\n", gstvEeNet.DhcpMode);
}


/*!
 * \fn     EeReadNtpParamToSram
 * \brief   ��ȡNTP����
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
    /* ������λ���� */
    return ;
  } 

  gstvEeNtp.NtpMode = iniparser_getint(ini, "NetParam:NtpMode", 0);

  iniparser_freedict(ini);

  printf("gstvEeNtp.NtpMode: %d\n", gstvEeNtp.NtpMode);
}


/*!
 * \fn     EeReadStorageParamToSram
 * \brief   ��ȡ����
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
    /* ������λ���� */
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
 * \brief   ��ȡ����
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
    /* ������λ���� */
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
 * \brief   ��ȡ����
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
    /* ������λ���� */
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
 * \brief  �ָ�����
 *          
 * \param  [in] void   #
 * 
 * \retval void
 */
void EeResetEepromData(void)
{
  
}


/*=======================================================================================
                         ���º���Ϊ�����������ü���ѯ                                    
=======================================================================================*/


/*!
 * \fn     EePromSetSysParam
 * \brief  ����ϵͳ����
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
    /* ������λ���� */
    return ;
  } 

  Language[0] = gstvEeSys.Language + '0';
  ArmDisarm[0] = gstvEeSys.ArmDisarm + '0';
  iniparser_set(ini, "SystemParam:Language", "3");
  iniparser_set(ini, "SystemParam:ArmDisarm", "4");
  
  //д���ļ�
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
 * \brief  �����������
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
 * \brief  ���ò���
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
 * \brief  ���ò���
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
 * \brief  ���ò���
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
 * \brief  ���ò���
 *          
 * \param  [in] void   # 
 * 
 * \retval void
 */
void EepromSetAoParam(void)
{
  
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
