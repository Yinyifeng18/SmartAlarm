/*!
 ***************************************************************************************
 *
 *  Copyright ? 2017-2018 yifneg. All Rights Reserved.
 *
 * \file      DiskDeal.cpp
 * \author    yifeng
 * \version   1.0
 * \date      2023��3��11��
 * \brief     ���̴���ͷ�ļ�
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
#include "DiskDeal.h"
#include "FileDeal.h"
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
static void DiskDealMountTfHandle(void);
static int DiskDealParseData(char *buf, int len);
static int DiskDealInitHotPlugSock(void);
static int DiskDealGetDiskInfo(pDISK diskInfo, const char *path);
static int DiskDealCalDiskInfo(char *diskTotal, char *diskAvail, char *diskFree, pDISK diskInfo);
static void DiskkDealReadDiskHandle(void);


static void *MountDeal_Thread(void *arg);
static void *DiskDeal_Thread(void *arg);




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
DiskDealBits gbitDisk;
DiskDealStru gstvDisk;





/*=======================================================================================
                             ��Դ�����ļ��ڲ�ʹ�õľֲ���������                          
=======================================================================================*/




/*
*****************************************************************************************
                            ��       ����ָ������嵥
*****************************************************************************************
*/



/*!
 * \fn     Start_DiskDeal_Monitor_Thread
 * \brief  
 *          
 * \param  [in] void   #
 * 
 * \retval void
 */
void Start_DiskDeal_Monitor_Thread(void)
{
  pthread_t disk_tidp;
  pthread_t mount_tidp;
  
  printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);
  
  if(pthread_create(&mount_tidp, NULL, MountDeal_Thread, NULL) != 0)
  {
    perror("pthread_create: NetDeal_Thread");
    return ;
  }  
  
  printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);
  
  if(pthread_create(&disk_tidp, NULL, DiskDeal_Thread, NULL) != 0)
  {
    perror("pthread_create: NetDeal_Thread");
    return ;
  }  
}


/*!
 * \fn     MountDeal_Thread
 * \brief  
 *          
 * \param  [in] void *arg   #
 * 
 * \retval static void *
 */
static void *MountDeal_Thread(void *arg)
{
  printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);

  pthread_detach(pthread_self());

  memset(&gstvDisk, 0, sizeof(DiskDealStru));
  memset(&gbitDisk, 0, sizeof(DiskDealBits));

  gstvDisk.HotPlugSock = DiskDealInitHotPlugSock();

  while(!gstvSysRun.quit)
  {
    /* ��� */
    DiskDealMountTfHandle();
    usleep(1000);
  }
}

/*!
 * \fn     DiskDeal_Thread
 * \brief  
 *          
 * \param  [in] void *arg   #
 * 
 * \retval static void *
 */
static void *DiskDeal_Thread(void *arg)
{
  printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);
  
  pthread_detach(pthread_self());

  while(!gstvSysRun.quit)
  {
    DiskkDealReadDiskHandle();
    
    usleep(1000);
  }
}





/*!
 * \fn     DiskDealMountTfHandle
 * \brief  Mount TF����
 *          
 * \param  [in] void   #
 * 
 * \retval static void
 */
static void DiskDealMountTfHandle(void)
{
  int ret = -1;
  char buf[cUeventBufferSize * 2] = {0};
  char  cmd[64] = {0};
  printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);
  recv(gstvDisk.HotPlugSock, &buf, sizeof(buf), 0);
  printf("recv data:" );
  printf("%s\n", buf);
  ret = DiskDealParseData(buf, sizeof(buf));
  if(ret == 2)
  {
    printf("mount tf card\r\n");
    FileDealCreateDir(cDefaultDiskPath);
    
    sprintf(cmd, "mount /dev/mmcblk0p1 %s", cDefaultDiskPath);
    printf("cmd: %s\n", cmd);
    //system(cmd);
    
    memset(cmd, 0, 64);
    sprintf(cmd, "ls %s", cDefaultDiskPath);
    printf("cmd: %s\n", cmd);
    //system(cmd);

    gbitDisk.MountOk = SET;
  }
  else
  {
    gbitDisk.MountOk = CLEAR;
  }
}


/*!
 * \fn     DiskDealParseData
 * \brief  ��������
 *          
 * \param  [in] char *buf   #
 * \param  [in] int len     #
 * 
 * \retval static int
 */
static int DiskDealParseData(char *buf, int len)
{
  if (strstr(buf, "add@/devices") != 0
    && strstr(buf, "block/mmcblk0/mmcblk0p1") != 0) {
    return 2;//�������sd��
  }
  else if (strstr(buf, "remove@/devices") != 0
      && strstr(buf, "block/mmcblk0/mmcblk0p1") != 0)
  {
    return 1;//����ε�sd��
  }
  
  return 0;
}

/*!
 * \fn     DiskDealInitHotPlugSock
 * \brief  ��ʼ��hot
 *          
 * \param  [in] void   #
 * 
 * \retval static int
 */
static int DiskDealInitHotPlugSock(void)
{
  const int buffersize = 1024;
  int ret;

  struct sockaddr_nl snl;
  bzero(&snl, sizeof(struct sockaddr_nl));
  snl.nl_family = AF_NETLINK;
  snl.nl_pid = getpid();
  snl.nl_groups = 1;

  int s = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
  if (s == -1) 
  {
    perror("socket");
    return -1;
  }
  setsockopt(s, SOL_SOCKET, SO_RCVBUF, &buffersize, sizeof(buffersize));

  ret = bind(s, (struct sockaddr *)&snl, sizeof(struct sockaddr_nl));
  if (ret < 0) 
  {
    perror("bind");
    close(s);
    return -1;
  }

  return s;
}


/*!
 * \fn     DiskDealGetDiskInfo
 * \brief  ��ȡ��Ϣ
 *          
 * \param  [in] pDISK diskInfo     #
 * \param  [in] const char *path   #
 * 
 * \retval int
 */
static int DiskDealGetDiskInfo(pDISK diskInfo,const char *path)
{
  int flag=0;

  if(NULL!=path)
  {
    strcpy(cDefaultDiskPath,path);
  }

  if(-1==(flag=statfs(cDefaultDiskPath,diskInfo)))
  {
    perror("getDiskInfo statfs fail");
    return 0;
  }

  return 1;
}

/*!
 * \fn     DiskDealCalDiskInfo
 * \brief  ����Disk��Ϣ
 *          
 * \param  [in] char *diskTotal   #
 * \param  [in] char *diskAvail   #
 * \param  [in] char *diskFree    #
 * \param  [in] pDISK diskInfo    #
 * 
 * \retval int
 */
static int DiskDealCalDiskInfo(char *diskTotal,char *diskAvail,char *diskFree,pDISK diskInfo)
{
  unsigned long long total=0,avail=0,free=0,blockSize=0;
  int flag=0;

  if(!diskTotal&&diskAvail&&diskFree&&diskInfo)
  {
    printf("\ncalDiskInfo param null!\n");
    return 0;
  }
  blockSize=diskInfo->f_bsize;
  total=diskInfo->f_blocks*blockSize;
  avail=diskInfo->f_bavail*blockSize;
  free=diskInfo->f_bfree*blockSize;

  //�ַ���ת��
  flag=sprintf(diskTotal,"%llu",total>>20);
  flag=sprintf(diskAvail,"%llu",avail>>20);
  flag=sprintf(diskFree,"%llu",free>>20);

  if(-1==flag)
  {
    return 0;
  }
  
  return 1;
}


/*!
 * \fn     DiskkDealReadDiskHandle
 * \brief  ��ȡ��Ϣ
 *          
 * \param  [in] void   #
 * 
 * \retval static void
 */
static void DiskkDealReadDiskHandle(void)
{
  DISK diskInfo;
  u32  free = 0;
  FileDealStru FileName;

  //if(gbitDisk.MountOk == CLEAR)
  //  return ;

  /* ����Ϣ */
  if(TimerGet1sSubb(gstvDisk.Timer3s, 3))
  {
    gstvDisk.Timer3s = TimerGet1sInc();

    memset(&diskInfo, 0, sizeof(DISK));
    DiskDealGetDiskInfo(&diskInfo, cDefaultDiskPath);

    DiskDealCalDiskInfo((char *)&gstvDisk.Total[0], (char *)&gstvDisk.Avail[0], (char *)&gstvDisk.Free[0],&diskInfo);

    printf("\ntotal:%s avail:%s free:%s\n",gstvDisk.Total, gstvDisk.Avail, gstvDisk.Free);

    if(atoi(gstvDisk.Free) < cMinFreeSpace)  
    {
      /* ��ȡ���� */
      FileName = FileDealGetDirFileName(cDefaultDiskPath);

      /* ɾ�� */
      DiskDealDeleteDir((char *)&FileName.Name[0].Data[0]);
    }
  }
}



/*!
 * \fn     DiskDealDeleteAll
 * \brief  ɾ������
 *          
 * \param  [in] void   #
 * 
 * \retval void
 */
void DiskDealDeleteAll(char *path)
{
  char  cmd[32];
  
  sprintf(cmd, "rm -rf %s*", cDefaultDiskPath);
  printf("cmd: %s\n", cmd);
  //system(cmd);
}


/*!
 * \fn     DiskDealDeleteDir
 * \brief  ɾ��ĳһĿ¼
 *          
 * \param  [in] char *Dir   #
 * 
 * \retval void
 */
void DiskDealDeleteDir(char *Dir)
{
  char  cmd[32];
  
  sprintf(cmd, "rm -rf %s%s", cDefaultDiskPath, Dir);
  printf("cmd: %s\n", cmd);
  //system(cmd);
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
