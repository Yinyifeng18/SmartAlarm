/*!
 ***************************************************************************************
 *
 *  Copyright ? 2017-2018 yifneg. All Rights Reserved.
 *
 * \file      MqttDeal.cpp
 * \author    yifeng
 * \version   1.0
 * \date      2023��3��11��
 * \brief     MQTT����Դ����
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
#include "MqttDeal.h"
#include "mqtt_aliyun_conf.h"
#include "cJSON.h"
#include "dictionary.h"
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
static void * MqttDeal_Thread(void *arg);
static void Mqtt_Message_Callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message);
static void Mqtt_Connect_Callback(struct mosquitto *mosq, void *obj, int rc);
static void Mqtt_Disconnect_Callback(struct mosquitto *mosq,void *obj,int rc);



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
st_mqtt  mqtt;    
struct   mosquitto *mosq = NULL;

MqttDealStru  gstvMqtt;
MqttDealBits  gbitMqtt;


/*=======================================================================================
                             ��Դ�����ļ��ڲ�ʹ�õľֲ���������                          
=======================================================================================*/




/*
*****************************************************************************************
                            ��       ����ָ������嵥
*****************************************************************************************
*/


/*!
 * \fn     Start_MqttDeal_Monitor_Thread
 * \brief  mqtt�������
 *          
 * \param  [in] void   #
 * 
 * \retval void
 */
void Start_MqttDeal_Monitor_Thread(void)
{
  pthread_t mqtt_tidp;
  
  printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);
  
  if (pthread_create(&mqtt_tidp, NULL, MqttDeal_Thread, NULL) != 0)
  {
    perror("pthread_create:MqttDeal_Thread");
    return;
  }
}


/*!
 * \fn     MqttDealRestartHandle
 * \brief  ��λ
 *          
 * \param  [in] void   #
 * 
 * \retval void
 */
void MqttDealRestartHandle(void)
{
  
}


/*!
 * \fn     MqttDeal_Thread
 * \brief  �̴߳���
 *          
 * \param  [in] void *arg   #
 * 
 * \retval static void *
 */
static void * MqttDeal_Thread(void *arg)
{
  printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);

  int ret;
  
  pthread_detach(pthread_self());

  memset(&mqtt,0,sizeof(mqtt));
  memset(&gstvMqtt, 0, sizeof(MqttDealStru));
  memset(&gbitMqtt, 0, sizeof(MqttDealBits));

  gbitMqtt.LinkOk = SET;

  ret = gain_mqtt_conf(cMqttIniPath, &mqtt);
  
  printf(" hostname: %s\n", mqtt.hostname);
  printf("     port: %d\n", mqtt.port);
  printf(" username: %s\n", mqtt.username);
  printf("   passwd: %s\n", mqtt.passwd);
  printf(" clientid: %s\n", mqtt.clientid);
  printf("sub_topic: %s\n", mqtt.sub_topic);
  printf("pub_topic: %s\n", mqtt.pub_topic);
  printf("      Qos: %d\n", mqtt.pub_topic);

  
  mosquitto_lib_init();
  mosq = mosquitto_new(mqtt.clientid, true, (void *)&mqtt);
  if(!mosq)
  {
    printf("mosquitto_new() failed: %s\n", strerror(errno));
    goto cleanup;
  }
  printf("mosquitto_new succeed\n");
  
  /* Set callback function */
  mosquitto_connect_callback_set(mosq, Mqtt_Connect_Callback) ;
  mosquitto_message_callback_set(mosq, Mqtt_Message_Callback);
  mosquitto_disconnect_callback_set(mosq, Mqtt_Disconnect_Callback) ;
  
  printf("Set callback function succeed\n");
  
  if(mosquitto_username_pw_set(mosq, mqtt.username, mqtt.passwd) != MOSQ_ERR_SUCCESS)
  {
    printf("mosquitto_username_pw_set failed: %s\n",strerror(errno));
    goto cleanup;
  }
  
  if(mosquitto_connect(mosq, mqtt.hostname,mqtt.port,KEEP_ALIVE) != MOSQ_ERR_SUCCESS)
  {
    printf("mosquitto_connect() failed: %s\n", strerror(errno));
    goto cleanup;
  }
  
  // �첽ѭ��
  ret = mosquitto_loop_start(mosq);
  if(ret != MOSQ_ERR_SUCCESS)
  {
    fprintf(stderr, "Error: %s\n", mosquitto_strerror(ret));
    goto cleanup;
  }
  
  printf("connect successful\n");
  while(!gstvSysRun.quit)
  {
    sleep(10);
    printf("%s:%d==> [%s]\n", __FILE__, __LINE__, __FUNCTION__);
    
    //Mqtt_PubData(cTargetDevice, cDeviceName, "SOS", "1");
    
    /* �ж�����״̬ */
   
  }

cleanup:
  mosquitto_destroy(mosq);
  mosquitto_lib_cleanup();

  return NULL;
}

/*!
 * \fn     Mqtt_Message_Callback
 * \brief  ��Ϣ�ص�
 *          
 * \param  [in] struct mosquitto *mosq                    #
 * \param  [in] void *userdata                            #
 * \param  [in] const struct mosquitto_message *message   #
 * 
 * \retval static void
 */
static void Mqtt_Message_Callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
  cJSON *payload_json =  cJSON_CreateObject();
  char  value[256];
  cJSON *cjson_str;
  
  if(!userdata)
  {
    printf("Invail parameter in %s\n",__FUNCTION__) ;
    return ;
  }

  if(message->payloadlen)
  {
    printf("message callback: %s\n", message->payload);
    /* �������� */
    memcpy(value, message->payload, message->payloadlen);
    payload_json = cJSON_Parse(value); 
    printf("%s\n",cJSON_Print(payload_json));
    if(payload_json)
    {
      /* ��ȡ���� */
      cjson_str = cJSON_GetObjectItem(payload_json, "TargetDevice");
      if(cjson_str) 
      {
        printf("%s:%s\r\n", cjson_str->string, cjson_str->valuestring);
      }
      
      cjson_str = cJSON_GetObjectItem(payload_json, "DeviceName");
      if(cjson_str) 
      {
        printf("%s:%s\r\n", cjson_str->string, cjson_str->valuestring);
      }

      cjson_str = cJSON_GetObjectItem(payload_json, "data");
      if(cjson_str) 
      {
        printf("%s:%s\r\n", cjson_str->string, cjson_str->valuestring);
      }
      
      cjson_str = cJSON_GetObjectItem(payload_json, "value");
      if(cjson_str) 
      {
        printf("%s:%s\r\n", cjson_str->string, cjson_str->valuestring);
      }

      /* ����������Ϣ����Ϣ�������� */
      
      
    }    
  }
  
  cJSON_Delete(payload_json);
}


/*!
 * \fn     Mqtt_Connect_Callback
 * \brief  ���ӻص�
 *          
 * \param  [in] struct mosquitto *mosq   #
 * \param  [in] void *obj                #
 * \param  [in] int rc                   #
 * 
 * \retval static void
 */
static void Mqtt_Connect_Callback(struct mosquitto *mosq, void *obj, int rc)
{
  int                 mid ;

  if(!obj)
  {
    printf("Invail parameter in %s\n", __FUNCTION__) ;
    return ;
  }
  
  if(!rc)
  {
    printf("connect server sucessful!\n") ;
    gbitMqtt.LinkOk = CLEAR;
    if( mosquitto_subscribe(mosq, &mid, mqtt.sub_topic, 0) != MOSQ_ERR_SUCCESS)
    {
      printf("mosquitto_subscribe() error:%s\n",strerror(errno)) ;
    }   
    
    printf("subscribe topic:%s\n", mqtt.sub_topic);
  }
}

/*!
 * \fn     my_disconnect_callback
 * \brief  ȡ�����ӻص�
 *          
 * \param  [in] struct mosquitto *mosq   #
 * \param  [in] void *obj                #
 * \param  [in] int rc                   #
 * 
 * \retval void
 */
static void Mqtt_Disconnect_Callback(struct mosquitto *mosq,void *obj,int rc)
{
  printf("Call the function: Mqtt_Disconnect_Callback\n");

  gbitMqtt.LinkOk = CLEAR;
}



/*!
 * \fn     Mqtt_PubData
 * \brief  ��������
 *          
 *  {"TargetDevice":"","DeviceName":"", "data":"", "status":0}
 * \retval void
 */
void Mqtt_PubData(char *TargetDevice, char *DeviceName, char *Data, char *status)
{
  cJSON   * root =  cJSON_CreateObject();
  char    *msg;

 
  if(gbitMqtt.LinkOk == SET)
    return ;

  memset(root, 0, sizeof(root));

  cJSON_AddItemToObject(root, "TargetDevice", cJSON_CreateString(TargetDevice));
  cJSON_AddItemToObject(root, "DeviceName",   cJSON_CreateString(DeviceName));
  cJSON_AddItemToObject(root, "data",         cJSON_CreateString(Data));
  cJSON_AddItemToObject(root, "status",       cJSON_CreateString(status));
      
  msg=cJSON_Print(root);
  printf("%s\n",msg);

  if(mosquitto_publish(mosq, NULL, mqtt.pub_topic, strlen(msg), msg, mqtt.Qos, NULL) != MOSQ_ERR_SUCCESS)
  {
    printf("mosquitto_publish failed: %s\n",strerror(errno));
    return;
  }
      
  printf("mqtt_publish successful!\n");
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
