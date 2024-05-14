/*!
 ***************************************************************************************
 *
 *  Copyright ? 2017-2018 yifneg. All Rights Reserved.
 *
 * \file      SnapStorageDeal.cpp
 * \author    yifeng
 * \version   1.0
 * \date      2023��3��11��
 * \brief     �洢����Դ����
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
#include "SnapStorageDeal.h"
#include "EepromDeal.h"
#include "FileDeal.h"
#include "DiskDeal.h"
#include "RtcDeal.h"
#include "TimerDeal.h"

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
static void *SnapStorageDeal_Thread(void *arg);

static void StorageDealHandle(void);

static void TimePathConv(void);

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
SnapStorageStru gstvSnapStorage;
SnapStorageBits gbitSnapStorage;


/*=======================================================================================
                             ��Դ�����ļ��ڲ�ʹ�õľֲ���������                          
=======================================================================================*/




/*
*****************************************************************************************
                            ��       ����ָ������嵥
*****************************************************************************************
*/

/*!
 * \fn     Start_SnapStorageDeal_Monitor_Thread
 * \brief  mqtt�������
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
 * \brief  �̴߳���
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
 * \brief  ����
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

  /* һֱʹ�� */
  gstvEeStorage.RecMode = 0x01;
  if(gstvEeStorage.RecMode == 0x01)
  {
    printf("gstvSnapStorage.Delay: %d\n", gstvSnapStorage.Delay);
    
    if((TimerGet1sSubb(gstvSnapStorage.Delay, cSaveTime))
    &&(gbitSnapStorage.RecNow == CLEAR))
    {
      gstvSnapStorage.Delay = TimerGet1sInc();
      
      printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);
      /* ��ȡ���� */
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
      /* ��ȡ���� */
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
    /* ��ʱģʽ */
    
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
 * \brief  �ַ�ת��
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
