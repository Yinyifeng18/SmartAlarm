/*!
 ***************************************************************************************
 *
 *  Copyright ? 2017-2018 yifneg. All Rights Reserved.
 *
 * \file      DiskDeal.cpp
 * \author    yifeng
 * \version   1.0
 * \date      2023年3月11日
 * \brief     磁盘处理头文件
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
#include "DiskDeal.h"
#include "FileDeal.h"
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
static void DiskDealMountTfHandle(void);
static int DiskDealParseData(char *buf, int len);
static int DiskDealInitHotPlugSock(void);
static int DiskDealGetDiskInfo(pDISK diskInfo, const char *path);
static int DiskDealCalDiskInfo(char *diskTotal, char *diskAvail, char *diskFree, pDISK diskInfo);
static void DiskkDealReadDiskHandle(void);


static void *MountDeal_Thread(void *arg);
static void *DiskDeal_Thread(void *arg);




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
DiskDealBits gbitDisk;
DiskDealStru gstvDisk;





/*=======================================================================================
                             本源程序文件内部使用的局部变量定义                          
=======================================================================================*/




/*
*****************************************************************************************
                            　       程序指令代码清单
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
    /* 检测 */
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
 * \brief  Mount TF处理
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
 * \brief  解析数据
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
    return 2;//代表插入sd卡
  }
  else if (strstr(buf, "remove@/devices") != 0
      && strstr(buf, "block/mmcblk0/mmcblk0p1") != 0)
  {
    return 1;//代表拔掉sd卡
  }
  
  return 0;
}

/*!
 * \fn     DiskDealInitHotPlugSock
 * \brief  初始化hot
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
 * \brief  获取信息
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
 * \brief  计算Disk信息
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

  //字符串转换
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
 * \brief  读取信息
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

  /* 读信息 */
  if(TimerGet1sSubb(gstvDisk.Timer3s, 3))
  {
    gstvDisk.Timer3s = TimerGet1sInc();

    memset(&diskInfo, 0, sizeof(DISK));
    DiskDealGetDiskInfo(&diskInfo, cDefaultDiskPath);

    DiskDealCalDiskInfo((char *)&gstvDisk.Total[0], (char *)&gstvDisk.Avail[0], (char *)&gstvDisk.Free[0],&diskInfo);

    printf("\ntotal:%s avail:%s free:%s\n",gstvDisk.Total, gstvDisk.Avail, gstvDisk.Free);

    if(atoi(gstvDisk.Free) < cMinFreeSpace)  
    {
      /* 获取数据 */
      FileName = FileDealGetDirFileName(cDefaultDiskPath);

      /* 删除 */
      DiskDealDeleteDir((char *)&FileName.Name[0].Data[0]);
    }
  }
}



/*!
 * \fn     DiskDealDeleteAll
 * \brief  删除所有
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
 * \brief  删除某一目录
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
