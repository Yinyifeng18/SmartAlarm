INCLUDE=-I./ -I./include
LIBS= -lpthread -lm -lrt -ldl -lmosquitto
all:SmartAlarm
SmartAlarm:
	 gcc -o SmartAlarm -O2 -w Main.cpp  AiDeal.cpp AoDeal.cpp cJSON.c CommDeal.cpp dictionary.c DiskDeal.cpp EepromDeal.cpp FileDeal.cpp GpioDeal.cpp iniparser.c KeyDeal.cpp mqtt_aliyun_conf.c MqttDeal.cpp NetDeal.cpp NtpDeal.cpp P2pDeal.cpp QueueDeal.cpp RtcDeal.cpp SnapStorageDeal.cpp TimerDeal.cpp VideoDeal.cpp WebDeal.cpp PingDeal.cpp $(INCLUDE) $(LIBS)
clean:
	rm -rfv SmartAlarm