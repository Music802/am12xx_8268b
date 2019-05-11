/////Micro-actions   sllin  2009-08-31//////

#include "multialarm.h"
//#include <sys/time.h>
#include "apps_vram.h"
#include <stdio.h>
#include <string.h>
#include "sys_rtc.h"
#include <sys_pmu.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys_gpio.h>
#include <sys/types.h>
#include <unistd.h>
#include <linux/string.h>
#include "ezcast_public.h"

#define STANDBY_BIN_PATH	"/am7x/sdk/standby.bin"
#define PMU_DEV_PATH		"/dev/pmu"
#define SYS_PM_STAT_PATH	"/sys/power/state"

typedef struct
{
	int hour;
	int minute;
	int second;
}al_time_t;

typedef struct
{
	int year;
	int month;
	int day;

	int wday;//����
}al_date_t;

#define AUTO_POWER_ON_EN 1
#define ALARMS_ENABLE
#ifndef ALARMS_ENABLE
char al_add_alarm(alarm_info_t *palarminfo){
	return -1;
}

char al_del_alarm(char ID){
	return 0;
}

char al_set_alarm(char ID,alarm_info_t *palarminfo){
	return 0;	
}

char al_get_alarm_info(char cmd,char Idx,alarm_info_t *palarminfo){
	return -1;
}

char al_load_alarm_info(){
	return 0;
}

char al_store_alarm_info(){
	return 0;
}

char al_close_alarm(char ID){
	return 0;
}

char al_open_alarm(char ID){
	return 0;
}

char al_check(){
	return 0;
}

char al_set_standy(unsigned int *alarmtime){
	return -1;
}

char al_init_alarms(){
	return 0;
}

char al_save_alarm_info(){
	return 0;
}

#else

#define alarm_infor(fmt,arg...)		printf("INFO[%s-%d]:"fmt"\n",__FILE__,__LINE__,##arg)
#define alarm_error(fmt,arg...)	printf("ERROR[%s-%d]:"fmt"\n",__FILE__,__LINE__,##arg)

unsigned char Alarmsnooze[8]={0,5,10,15,20,25,30,60};//��¼���ʱ�䣬��λΪ����
//unsigned char Alarmsnooze[8]={0,1,2,3,4,5,6,7};//��¼���ʱ�䣬��λΪ����
unsigned char shutup_bitmap[ALARM_SHUTUP_BITMAP];	

alarm_info_head_t AlarmHead;
auto_power_info_t PowerInfo;
auto_screenoff_info_t ScreenoffInfo;
char IsFirstBoot=1;
char IsneedSave=0;
extern int board_poweron;
extern int ir_poweron;

static char showAlarmInfo(char ID){
	 alarm_infor("====INFO Alarm ID=%d=====",ID);
	 alarm_infor("days=0x%x,enable=%d,hour=%d,min=%d,ismsgsend=%d",(AlarmHead.info+ID)->days,(AlarmHead.info+ID)->enable,(AlarmHead.info+ID)->hour,\
	 	(AlarmHead.info+ID)->min,(AlarmHead.info+ID)->ismsgsend);
	 alarm_infor("name=%s,userid=%d,snooze=%d",(AlarmHead.info+ID)->name,(AlarmHead.info+ID)->user_id,(AlarmHead.info+ID)->snooze);
	return 0;
}


static int __get_env_data(struct sysconf_param *sys_cfg_data){
	if(apps_vram_read(VRAM_ID_SYSPARAMS,sys_cfg_data,sizeof(struct sysconf_param))!=0){
		alarm_error("Get environment paras err");
		return -1;
	}
	return 0;
}

static int __save_env_data(struct sysconf_param *sys_cfg_data){
	if(apps_vram_write(VRAM_ID_SYSPARAMS,sys_cfg_data,sizeof(struct sysconf_param))!=0){
		alarm_error("App vram write error");
		return -1;
	}
	if(apps_vram_store(VRAM_ID_SYSPARAMS)==-1)
		alarm_error("App vram store error!");
	return 0;
}

static void __get_time(al_time_t *time)
{
	rtc_time_t rtc_time;
	if(tm_get_rtc(NULL,&rtc_time)!=0)
		alarm_error("Get rtc Time Error");
	time->hour = rtc_time.hour;
	time->minute= rtc_time.min;
	time->second = rtc_time.sec;
	//alarm_infor("CurTime=%d,%d,%d",time->hour,time->minute,time->second);
}

static void __get_date(al_date_t *date)
{
	rtc_date_t rtc_date;
	if(tm_get_rtc(&rtc_date,NULL)!=0)
		alarm_error("Get rtc Date Error");
	date->year = rtc_date.year;
	date->month = rtc_date.month;
	date->day = rtc_date.day;
	date->wday = rtc_date.wday;
	if(date->wday==0)//change the sunday to 7
		date->wday = 7;
}


//��⵱ǰ��һ��û�б��õ�ID
//�ɹ�����û�б��õ�ID��
//���󷵻�-1
static char __get_firstID(alarm_cmd_getid_e cmd,char startidx){
	char whichbyte,whichbit;
	char mark=0;
	char i=0;
	char j;
	char tmp;
	whichbyte = startidx>>3;
	whichbit = startidx&0x07;
	for(i=whichbyte;i<ALARM_VAILD_BITMAP;i++){
		tmp = *(AlarmHead.alarmbitmap+i);
		for(j=whichbit;j<8;j++){
			mark = 1<<j;
			if(cmd ==ALARM_VALID){
				if((tmp&mark)!=0){
					return (i*8+j);
				}
			}
			else if(cmd ==ALARM_INVALID){
				if((tmp&mark)==0){
					return (i*8+j);
				}
			}	
		}
	}
	alarm_error("Can't find matched id");
	return -1;
}

//�������ID����Ϊ��Ч
//����ֵ:
//-1:�������ֵ������ʧ��
//0:���óɹ�
static char __set_IDVaild(char ID){
	char whichbyte,whichbit;
	if(ID>=ALARM_MAX_NUM){
		return -1;
	}
	whichbyte = ID>>3;
	whichbit = ID&0x07;
	*(AlarmHead.alarmbitmap+whichbyte) |= 1<<whichbit;
	return 0;
}

//��⴫���ID�Ƿ�Ϊ��ЧID
// 0����ЧID
//-1������ЧID
//-2�����ID ��
static char __check_IDVaild(char ID){
	unsigned char whichbyte,whichbit;
	if(ID>=ALARM_MAX_NUM){
		alarm_error("ID is out of range MAX=%d, your ID=%d",ALARM_MAX_NUM-1,ID);
		return -2;
	}
	whichbyte = ID>>3;
	whichbit = ID&0x07;
	if((AlarmHead.alarmbitmap[whichbyte] & (1<<whichbit))==0)
		return -1;
	else
		return 0;
}

//�жϸ���ID��Ӧ�������Ƿ񱻹ر���
//����ֵ:
// 1:���ӿ���
//0:���ӹر�
static char __check_alarm_shutup(char ID){
	unsigned char whichbyte,whichbit;
	whichbyte = ID>>3;
	whichbit = ID&0x07;
	if((AlarmHead.shutup[whichbyte] & (1<<whichbit))!=0)
		return 0;
	else
		return 1;
}

//����ӦID�����ӹر�
static char __shutup_alarm(char ID){
	unsigned char whichbyte,whichbit;
	whichbyte = ID>>3;	
	whichbit = ID&0x07;
	AlarmHead.shutup[whichbyte] |= 1<<whichbit;
	return 0;
}

static void __store_shutupbitmap()
{
	memcpy(shutup_bitmap,AlarmHead.shutup,ALARM_SHUTUP_BITMAP);
}

static void __load_shutupbitmap()
{
	memcpy(AlarmHead.shutup,shutup_bitmap,ALARM_SHUTUP_BITMAP);
}
//��������ȫ����ʱ�ӣ����Ǵ�ָ��ID�ĵ���ʱ��
//��һ�쿪ʼ��Ҫ��ȫ�����Ӵ�
static char __open_alarm(char cmd,char ID){
	unsigned char whichbyte,whichbit;
	whichbyte = ID>>3;	
	whichbit = ID&0x07;
	if(cmd == ALARM_OPEN_ALL){
		memset(AlarmHead.shutup,0,sizeof(AlarmHead.shutup));
	}
	else
		AlarmHead.shutup[whichbyte] &= ~(1<<whichbit);
	return 0;
}

//���ݲ�ͬ���������ͬ����Ϣ��
//�ϲ���յ���Ϣ��ID�ű������ĸ����ӷ�����
static char __send_alarm_msg(char cmd,char ID){
	if(ALARM_MSG_ALARMON==cmd){
		if(AlarmHead.msg_send.msg_send_func!=NULL){
			char rtn;
			int parasend=(int)ID;
			alarm_infor("Alarm time: ID=%d send message cmd=%d",ID,cmd);
			rtn = AlarmHead.msg_send.msg_send_func(AlarmHead.msg_send.pararcv,(void*)parasend);
			if(rtn==0)
				(AlarmHead.info+ID)->ismsgsend = 1;
			else
				alarm_infor("Send Msg Error!");
		}
	}
	return 0;
}

//��ȡ��ǰ���������ڼ�
//0~6����1~������
static char __get_curdate(){
	al_date_t date;
	char wday;
	char i,num,startidx=0,id;
	__get_date(&date);
	if(date.day != AlarmHead.curday || date.month !=AlarmHead.curmonth || date.year!=AlarmHead.curyear){
		__open_alarm(ALARM_OPEN_ALL,0);
		AlarmHead.curday = (char)date.day;
		AlarmHead.curmonth = (char)date.month;
		AlarmHead.curyear =  (short int)date.year;
		num = al_get_alarm_info(ALARM_GET_TOLNUM,0,NULL);
		for(i=0;i<num;i++){//�µ�һ�쿪ʼ����Ҫ�������е����岢�ȽϽ�ismsgsend���Ϊ0
			id = __get_firstID(ALARM_VALID,startidx);
			if(id!=-1){
				(AlarmHead.info+id)->ismsgsend = 0;
				(AlarmHead.info+id)->istimematch=0;
			}
			startidx = id+1;
		}
		//Alarm_storeInfo();//�������ж��ж�дVram
		IsneedSave = 1;//
	}
	return date.wday-1;//7�������죬1~6������һ��������
}

//�жϸ���ʱ���Ƿ�������ʱ�����õ�
//����ֵ:
// 1:��ʱ�����õ�
//0:����
static char __check_time(al_time_t time, char ID){
	char wday;
	char rtn;
	int tmp;
	wday = __get_curdate();
	rtn = __check_alarm_shutup(ID);
	//alarm_infor("Wday=%d, rtnID=%d",wday,rtn);
	//alarm_infor("IDmin=%d,timemin=%d,IDhour=%d,timehour=%d",(AlarmHead.info+ID)->min,time.minute, (AlarmHead.info+ID)->hour,time.hour);
	if(rtn == 1){//���ӿ���
		//alarm_infor("wday=%d,days=%d",wday,(AlarmHead.info+ID)->days);
		if((1<<wday)&(AlarmHead.info+ID)->days){//�ж����ڼ��Ƿ�����������
			if((AlarmHead.info+ID)->min==time.minute && (AlarmHead.info+ID)->hour == time.hour){
				if((AlarmHead.info+ID)->ismsgsend)
					return 0;
				else{
					(AlarmHead.info+ID)->istimematch= 1;
					return 1;
				}
			}
			else{
				if(time.hour>= (AlarmHead.info+ID)->hour){
					tmp = (time.hour-(AlarmHead.info+ID)->hour)*60+time.minute-(AlarmHead.info+ID)->min;
					if(tmp>0){
						if(Alarmsnooze[(AlarmHead.info+ID)->snooze]!=0){
							if(tmp%Alarmsnooze[(AlarmHead.info+ID)->snooze]==0){
								if((AlarmHead.info+ID)->ismsgsend ||(AlarmHead.info+ID)->istimematch!=1)
									return 0;
								else
									return 1;
							}
						}
						else
							return 0;
					}
				}	
			}
			(AlarmHead.info+ID)->ismsgsend = 0;//������������ʱ����ͬ�����ڣ���Ҫ��ismsgsend����Ϊ0
		}
	}
	return 0;
}

//Ϊϵͳ�������ʱ��
static char __init_alarms(){
	alarm_info_t alarminfo;
	alarm_infor("Init three group alarms");
	memset(&alarminfo,0,sizeof(alarm_info_t));
	al_load_alarm_info();
	al_add_alarm(&alarminfo);
	al_add_alarm(&alarminfo);
	al_add_alarm(&alarminfo);
	al_store_alarm_info();
	return 0;
}


//��ʱ�������ұȴ����ʱ������ʱ������
//wday:Ϊ����1~�������һ��0~6
//starttime: ʱ�䵥λΪ����
//����ҪѰ�ұ�12:20�����ʱ�����ã���*starttime=12*60+20
//����ʱ����Ȼ����starttime��
static void __get_earliset_time(char wday,int *starttime,int curtime){
	int tmpminalarm;
	char i,startidx=0,id,num;
	num = al_get_alarm_info(ALARM_GET_TOLNUM,0,NULL);
	for(i=0;i<num;i++){//�������е����岢�Ƚϵ����Ƿ��б�tmpminstart��С����Чʱ������
		id = __get_firstID(ALARM_VALID,startidx);
		if(id!=-1){
			alarm_infor("Get Valid=%d",id);
			if((AlarmHead.info+id)->enable){
				if(__check_alarm_shutup(id)==1){
					tmpminalarm =(AlarmHead.info+id)->hour*60+(AlarmHead.info+id)->min;
					alarm_infor("tmpminalarm hour=====%d,min=%d",tmpminalarm/60,tmpminalarm%60);
					if(tmpminalarm<*starttime){
						if((1<<wday)&(AlarmHead.info+id)->days){
							if(tmpminalarm>=curtime){//����ȵ�ǰʱ��Ҫ���趨ֵ�Żᱻ��ֵ
								*starttime=tmpminalarm;
								alarm_infor("tmpminstart hour=%d,min=%d",tmpminalarm/60,tmpminalarm%60);
							}
						}
					}
				}
			}
		}
		startidx = id+1;
	}
}



/////////////////////////////////////////////////////////////
//���һ�����ӣ�����������Ϣ
//����ֵ:
//�ɹ�:�������ӵ�ID��
//����:-1
//ע�⣬������֮��ǵõ���Alarm_storeInfo��������Ϣ���浽Vram
//�ɵ��ö�θú���֮���ٵ���һ��Alarm_storeInfo
char al_add_alarm(alarm_info_t *palarminfo){
	unsigned char invaidid;
	if(AlarmHead.num<ALARM_MAX_NUM){
		invaidid = __get_firstID(ALARM_INVALID,0);
		if(invaidid==-1)
			goto error_exit;
		memcpy(AlarmHead.info+invaidid,palarminfo,sizeof(alarm_info_t));
		__set_IDVaild(invaidid);
		AlarmHead.num++;
		return invaidid;			
	}
error_exit:
	alarm_error("add Alarm Error!, can't find invaid ID");
	return -1;
}

//ɾ������ID��ʱ��
//����ֵ:
//-1:ʧ��
//0:�ɹ�
char al_del_alarm(char ID){
	unsigned char whichbyte,whichbit;
	unsigned char tmp;
	if(ID>=ALARM_MAX_NUM){
		return -1;
	}
	whichbyte = ID>>3;
	whichbit = ID&0x07;
	if(whichbyte>=ALARM_VAILD_BITMAP){
		alarm_error("Carzy ID is out of range==%d",ID);
		return -1;
	}
	*(AlarmHead.alarmbitmap+whichbyte) &= ~(1<<whichbit);
	AlarmHead.num--;
	return 0;
}

//���Ѿ����ڵ�ID���ӽ�������
//����ֵ:
//-1:ID����Ч
//0:���óɹ�
//ע�⣬������֮��ǵõ���Alarm_storeInfo��������Ϣ���浽Vram
//�ɵ��ö�θú���֮���ٵ���һ��Alarm_storeInfo
char al_set_alarm(char ID,alarm_info_t *palarminfo){
	if(__check_IDVaild(ID)!=0){
		alarm_infor("the ID is invaid");
		return -1;
	}
	else{
		__open_alarm(ALARM_OPEN_ONE,ID);//��Ҫ�����ӹر�λ��
		palarminfo->ismsgsend = 0;//��Ҫ����Ϣ�Ѿ�����λ����Ϊ0
		palarminfo->istimematch = 0;
		memcpy(AlarmHead.info+ID,palarminfo,sizeof(alarm_info_t));
		//showAlarmInfo(ID);
	}
	return 0;
		
}

//���ݴ���������Index��ȡ��Ӧ��ʱ����Ϣ
//Ϊ��ֹ��Ϣ��ʱ�����ȵ���Alarm_loadInfo�����øú���
//�ȵ��øú���������ALARM_GET_TOLNUM�����ȡ�м�������
//����ALARM_GET_INFO�����ȡ������Ϣ������Idx Ϊ0~AlarmHead.num-1
//����ֵ���Ϊ-1����ʧ�ܣ�����ֵ��Ϊ�������ӵ�ID
char al_get_alarm_info(char cmd,char Idx,alarm_info_t *palarminfo){
	char staridx=0;
	char ID=0;
	char i;
	if(cmd==ALARM_GET_TOLNUM){
		return AlarmHead.num;
	}
	else{
		if(cmd==ALARM_GET_INFO_VIRID){
			for(i=0;i<Idx;i++){
				ID = __get_firstID(ALARM_VALID,staridx);
				staridx = ID+1;
			}
		}
		else if(cmd==ALARM_GET_INFO_PHYID)
			ID = Idx;
		else
			goto error_exit;
		if(__check_IDVaild(ID)!=0){
			alarm_infor("the ID is invalid");
			return -1;
		}
		else{
			memcpy(palarminfo,AlarmHead.info+ID,sizeof(alarm_info_t));
		}
		return ID;
	}
error_exit:
	alarm_error("get Alarm info cmd = %d is error",cmd);
	return -1;
}

//��Vram�е����ݴ�ŵ�ȫ�ֱ�����
//�ڶ�ȡ�κ��й�������Ϣ֮ǰ�������е��ù��ú���
char al_load_alarm_info(){
	struct sysconf_param env_data;
	__get_env_data(&env_data);
	memcpy(&AlarmHead,&env_data.alarm_info_head,sizeof(alarm_info_head_t));
	return 0;
}

//��ȫ�ֱ����е����ݱ��浽Vram��
char al_store_alarm_info(){
	struct sysconf_param env_data;
	__get_env_data(&env_data);
	memcpy(&env_data.alarm_info_head,&AlarmHead,sizeof(alarm_info_head_t));
	__save_env_data(&env_data);
	return 0;
}

//�ر�ָ��ID������
char al_close_alarm(char ID){
	__shutup_alarm(ID);
	return 0;
}

char al_open_alarm(char ID){
	__open_alarm(ALARM_OPEN_ONE,ID);
	(AlarmHead.info+ID)->ismsgsend = 0;//��Ҫ����Ϣ�Ѿ�����λ����Ϊ0
	(AlarmHead.info+ID)->istimematch = 0;
	return 0;
}

//��ʼ��ʱ����
char al_init_alarms(alarm_msg_send_t *msg_send){
	if(IsFirstBoot==1){
		al_time_t time;
		IsFirstBoot=0;
		__get_time(&time);
		al_load_alarm_info();
		al_load_poweroff_info();
		////test
		if(AlarmHead.num==0)
			__init_alarms();
		#if 0 //����ǲ�����
		if(AlarmHead.num!=0){
			alarm_info_t alarminfo;
			alarminfo.days = 127;
			alarminfo.enable = 1;
			alarminfo.hour = time.hour;
			alarminfo.min = time.minute+1;
			alarminfo.snooze = 0;
			al_set_alarm(0,&alarminfo);
			alarminfo.min = time.minute+2;
			alarminfo.snooze = 1;
			al_set_alarm(1,&alarminfo);
			alarm_infor("Set Alarm OK  bitmap=0x%x,curhour =%d,curmin=%d ",AlarmHead.alarmbitmap[0],time.hour,time.minute);
		}
		#endif
		if(msg_send!=NULL)
			memcpy(&AlarmHead.msg_send,msg_send,sizeof(alarm_msg_send_t));
		else
			memset(&AlarmHead.msg_send,0,sizeof(alarm_msg_send_t));
		/////////////////
	}
	return 0;
}

char al_save_alarm_info(){
	if(IsneedSave == 1){
		IsneedSave = 0;
		al_store_alarm_info();
	}
	return 0;
}


char al_check(){
	al_time_t time;
	char num,id;
	char i,startidx;
	__get_time(&time);

	if(time.second>0 && time.second<30){
		startidx = 0;
		num = al_get_alarm_info(ALARM_GET_TOLNUM,0,NULL);
		//alarm_infor("Check Alarm time Num=%d",num);
		for(i=0;i<num;i++){//�������е����岢�Ƚ�
			id = __get_firstID(ALARM_VALID,startidx);
			if(id!=-1){
				//alarm_infor("Get Valid=%d",id);
				if((AlarmHead.info+id)->enable){
					if(__check_time(time,id)==1){
						if((AlarmHead.info+id)->snooze==0){
							__shutup_alarm(id);
						}
						__send_alarm_msg(ALARM_MSG_ALARMON,id);
					}
				}
			}
			startidx = id+1;
		}
	}
	al_save_alarm_info();
	return 0;
}

//��Vram�е����ݴ�ŵ�ȫ�ֱ�����auto power info
char al_load_poweroff_info(){
	struct sysconf_param env_data;
	__get_env_data(&env_data);
	PowerInfo.auto_power_frequence = env_data.auto_power_freq;
	PowerInfo.auto_power_off_enable = env_data.auto_power_off_enable;
	PowerInfo.auto_power_on_enable = env_data.auto_power_on_enable;
	memcpy(&PowerInfo.power_off_time,&env_data.power_off_time,sizeof(rtc_time_t));
	memcpy(&PowerInfo.power_on_time,&env_data.power_on_time,sizeof(rtc_time_t));
	return 0;
}

//��ȫ�ֱ����е����ݱ��浽Vram��
char al_store_poweroff_info(){
	struct sysconf_param env_data;
	__get_env_data(&env_data);
	env_data.auto_power_off_enable = PowerInfo.auto_power_off_enable;
	env_data.auto_power_on_enable = PowerInfo.auto_power_on_enable;
	__save_env_data(&env_data);
	return 0;
}

int auto_power_off(int wakeup_mode)
{
	int fd,ret;
	char test_buf[128];
	char *func_buf;
	struct stat fbuf;
	struct am_pm_info pm_info;
	struct tm *poweron_time;

	if(check_connect_pc()==1){
		alarm_infor("now is connecting pc, cannot power off");
		return 0;
	}
	/*****************************************
	config standby info
	******************************************/
	fd = open(PMU_DEV_PATH,O_RDWR);
	if(fd==-1){
		alarm_error("can not open pmu device\n");
		return -19;
	}
	if(wakeup_mode==KEY_NDC_MODE)
		pm_info.arg.wakeup_mode = KEY_NDC_MODE;
	else
#if EZMUSIC_ENABLE
		pm_info.arg.wakeup_mode = EXT_DC_MODE | REBOOT_DC_MODE;
#else
		pm_info.arg.wakeup_mode = RTC_DC_MODE | EXT_DC_MODE | KEYE_DC_MODE | IRE_DC_MODE | REBOOT_DC_MODE | LAN_DC_MODE;
#endif
		
	if((pm_info.arg.wakeup_mode&RTC_DC_MODE) == RTC_DC_MODE)
	{
		alarm_infor("now is in RTC_DC_MODE");
		poweron_time = al_set_standbytime();
		if(poweron_time!=NULL)
		{
			alarm_infor("powerondate is %04d-%02d-%02d,%02d:%02d:%02d",poweron_time->tm_year,poweron_time->tm_mon,poweron_time->tm_mday,poweron_time->tm_hour,poweron_time->tm_min,poweron_time->tm_sec);
			pm_info.arg.rtc_param[0] = AM_PMU_MK_DATE(poweron_time->tm_year,poweron_time->tm_mon,poweron_time->tm_mday);	//alarm date
			pm_info.arg.rtc_param[1] = AM_PMU_MK_TIME(poweron_time->tm_hour,poweron_time->tm_min,poweron_time->tm_sec);	// alarm time
		}
	}
	if((pm_info.arg.wakeup_mode&EXT_DC_MODE) == EXT_DC_MODE)
	{
		alarm_infor("now is in EXT_DC_MODE,board power on is %d",board_poweron);
		pm_info.arg.ext_param[0] = board_poweron;
	}
	if((pm_info.arg.wakeup_mode&KEYE_DC_MODE) == KEYE_DC_MODE)
	{
		alarm_infor("now is in KEYE_DC_MODE");
		pm_info.arg.key_param[0] = 3;	//count times
		pm_info.arg.key_param[1] = 6;	//key number
	}
	if((pm_info.arg.wakeup_mode&IRE_DC_MODE) == IRE_DC_MODE)
	{
		alarm_infor("now is in IRE_DC_MODE,ir power on is 0x%x",ir_poweron);
		pm_info.arg.ire_param[0] = 1;   //valid code number
		/*irkey code*/
		pm_info.arg.ire_param[1] = ir_poweron;//0xc0;//0x12; 
		pm_info.arg.ire_param[2] = 0;
		pm_info.arg.ire_param[3] = 0;
		pm_info.arg.ire_param[4] = 0;
	}
	if((pm_info.arg.wakeup_mode&KEY_NDC_MODE) == KEY_NDC_MODE)
	{
		alarm_infor("now is in KEY_NDC_MODE");
		pm_info.arg.sram_param[0] = 60;	//gpio num
	}
	if((pm_info.arg.wakeup_mode&LAN_DC_MODE) == LAN_DC_MODE)
	{
		alarm_infor("now is in LAN_DC_MODE");
		pm_info.arg.lan_param[0] = reg_readl(0xb020006c);	//gpio num
		pm_info.arg.lan_param[1] = reg_readl(0xb0200070);
		pm_info.arg.lan_param[2] = reg_readl(0xb0200074);

	}	
	ioctl(fd,AM_PMU_SET,&pm_info);
	close(fd);
	/*****************************************
	enter standby
	*******************************************/
	fd = open(SYS_PM_STAT_PATH, O_RDWR);
	if(fd==-1){
		alarm_error("cant open power state\n");
		return -19;
	}

	/*read status*/
	ret = read(fd,test_buf,128);
	if(ret>0){
		test_buf[ret] = '\0';
		alarm_infor("power state:%s\n",test_buf);
		memcpy(test_buf,"mem",3);
		write(fd,test_buf,3);
	}else{
		alarm_error("read failed :%d\n",ret);
	}

	close(fd);
	return 0;
}

char power_off_check()
{
	rtc_time_t rtc_time;
	rtc_date_t rtc_date;
	int wday;
	struct tm *poweron_time;
	//alarm_infor("@@@Now is in power off check!\n");
	if(PowerInfo.auto_power_off_enable == 1)
	{
		if(tm_get_rtc(&rtc_date,&rtc_time)!=0)
		{
			alarm_error("Get rtc Time Error");
			return -1;
		}
		//alarm_infor("rtc date:%04d:%02d:%02d--weekday is %d\n",rtc_date.year,rtc_date.month,rtc_date.day,rtc_date.wday);
		if(rtc_date.wday == 0)
			wday = 7;
		else
			wday = rtc_date.wday;
		//alarm_infor("Now wday is %d\n",wday);
		//alarm_infor("Now time is %02d:%02d:%02d, power off time is %02d:%02d:00\n",rtc_time.hour,rtc_time.min,rtc_time.sec,PowerInfo.power_off_time.hour,PowerInfo.power_off_time.min);
		if(rtc_time.hour==PowerInfo.power_off_time.hour && rtc_time.min==PowerInfo.power_off_time.min && rtc_time.sec<10)
		{
			//alarm_infor("auto power freq is %d\n",PowerInfo.auto_power_frequence);
			if((PowerInfo.auto_power_frequence & 0x80)!=0)	//auto-power only once
			{
				PowerInfo.auto_power_off_enable = 0;
				al_store_poweroff_info();
				auto_power_off(RTC_DC_MODE);
			}
			else if((PowerInfo.auto_power_frequence & (0x01<<(wday-1)))!=0)
			{
				//alarm_infor("0x%x\n",(0x01<<(wday-1)));
				auto_power_off(RTC_DC_MODE);
			}
		}
	}
	return 0;
}

//����ת��Ϊ����1970-01-01,00:00:00
static time_t datetosec(struct tm * tm_t)
{
	return mktime(tm_t);
}

//����ת��Ϊ����
static struct tm * sectodate(time_t time)
{
	return gmtime(&time);
}

//����StandBy֮ǰ��Ҫ���øú����������ʱ��
struct tm * al_set_standbytime(){
	int curmin,tmppoweron,plusmin,wday,plusday,tmpminstart;
	int i;
	rtc_date_t rtc_date;
	rtc_time_t rtc_time;
	time_t nowsec;
	struct tm nowdate;
	struct tm *powerondate;
	int poweron_plus_time=0;
	
	plusmin = 0;
	tm_get_rtc(&rtc_date,&rtc_time);
	if(rtc_date.wday == 0)
		wday = 7;
	else
		wday = rtc_date.wday;
	curmin = rtc_time.hour*60+rtc_time.min;
	if(PowerInfo.auto_power_on_enable==AUTO_POWER_ON_EN){
		tmppoweron = PowerInfo.power_on_time.hour*60+PowerInfo.power_on_time.min;
		//alarm_infor("tmppoweronis %d,curmin is %d,wday is %d",tmppoweron,curmin,wday);
		plusmin = tmppoweron - curmin;
		//alarm_infor("plusmin is %d",plusmin);
		if(tmppoweron < curmin){
			plusday = 1;
		}
		else{
			plusday = 0;
		}
		if((PowerInfo.auto_power_frequence& 0x80)!= 0){	//auto-power only once
			//alarm_infor("now is auto power on once!");
			PowerInfo.auto_power_on_enable = 0;
			al_store_poweroff_info();
		}
		else if((PowerInfo.auto_power_frequence& 0x7f) == 0x7f){
			//alarm_infor("now is auto power on everyday!");
		}
		else if((PowerInfo.auto_power_frequence& 0x1f) == 0x1f){
			//alarm_infor("now is auto power on monday to friday!");
			if(tmppoweron<curmin){
				if(wday==5){
					plusday += 2;
				}
				else if(wday==6){
					plusday += 1;
				}
			}
		}
		else if((PowerInfo.auto_power_frequence& 0x60) == 0x60){
			//alarm_infor("now is auto power on weekday!");
			if(tmppoweron<curmin){
				if(wday==7){
					plusday += 5;
				}
				else if(wday==1){
					plusday += 4;
				}
				else if(wday==2){
					plusday += 3;
				}
				else if(wday==3){
					plusday += 2;
				}
				else if(wday==4){
					plusday += 1;
				}
			}
		}
		plusmin += 24*60*plusday;
		//alarm_infor("plusmin is %d,plusday is %d",plusmin,plusday);
		poweron_plus_time = plusmin;
		tmpminstart = 24*60+1;
		if(plusday==0)
			tmpminstart = tmppoweron;
	}
	else{
		tmpminstart = 24*60+1;
		plusday=7;
	}
	wday = __get_curdate();
	alarm_infor("tmpminstart is %d",tmpminstart);
	__get_earliset_time(wday,&tmpminstart,curmin);
	alarm_infor("tmpminstart is %d",tmpminstart);
	__store_shutupbitmap();
	__open_alarm(ALARM_OPEN_ALL,0);//��Ҫ��ʼ׼���жϵڶ�������
	for(i=0;i<plusday-1;i++){
		if(tmpminstart!=24*60+1)
			break;
		else{//���첢û���ҵ���Ч�Ŀ���ʱ�����ã���ҪѰ�ҵڶ���
			wday++;
			if(wday==7)//����Ϊ�����죬��Ҫ��λ������һ
				wday=0;
			//alarm_infor("wday is %d,i is %d",wday,i);
			__get_earliset_time(wday,&tmpminstart,0);
		}
	}
	if(tmpminstart==24*60+1){
		if(plusday!=0){
			tmpminstart = tmppoweron;
			wday++;
			i++;
			if(wday==7)//����Ϊ�����죬��Ҫ��λ������һ
				wday=0;
			__get_earliset_time(wday,&tmpminstart,0);
			if(tmpminstart!=tmppoweron){
				alarm_infor("find alarm!!!!!");
			}
			else{
				alarm_error("no alarm is before!!!!!");
				if(poweron_plus_time!=0){//����֪���Զ����ػ��Ѿ������ã������˳�
				}
				else
					return NULL;
			}
		}
	}
	else{
		alarm_infor("find alarm!!!!!");
	}
	__load_shutupbitmap();
	alarm_infor("tmpminstart is %d,curmin is %d,i is %d",tmpminstart,curmin,i);
	if(tmpminstart == tmppoweron)//˵��û���ҵ�alarm������ʱ��
		plusmin = poweron_plus_time;
	else
		plusmin = tmpminstart - curmin + 24*60*i;
	//alarm_infor("plusmin is %d",plusmin);
	tm_get_rtc(&rtc_date,NULL);
	nowdate.tm_sec=rtc_time.sec;
	nowdate.tm_min=rtc_time.min;
	nowdate.tm_hour=rtc_time.hour;
	nowdate.tm_mday=rtc_date.day;
	nowdate.tm_mon=rtc_date.month-1;
	nowdate.tm_year=rtc_date.year-1900;
	alarm_infor("####nowdate is %02d-%02d-%02d,%02d:%02d:%02d",nowdate.tm_year,nowdate.tm_mon,nowdate.tm_mday,nowdate.tm_hour,nowdate.tm_min,nowdate.tm_sec);
	nowsec = datetosec(&nowdate);
	alarm_infor("@@@@@nowsec is %ld",nowsec);
	nowsec += (plusmin*60-rtc_time.sec);
	powerondate = sectodate(nowsec);
	powerondate->tm_year += 1900;
	powerondate->tm_mon += 1;
	alarm_infor("####powerondate is %04d-%02d-%02d,%02d:%02d:%02d",powerondate->tm_year,powerondate->tm_mon,powerondate->tm_mday,powerondate->tm_hour,powerondate->tm_min,powerondate->tm_sec);
	return powerondate;
}

//��Vram�е����ݴ�ŵ�ȫ�ֱ�����auto screenoff info
char al_load_screenoff_info(){
	struct sysconf_param env_data;
	__get_env_data(&env_data);
	ScreenoffInfo.auto_screen_off_enable = env_data.auto_screen_off_enable;
	ScreenoffInfo.auto_screen_off_time = env_data.screen_off_time;
	//alarm_infor("now is to call al_set_screenoff_time");
	al_set_screenoff_time();
	return 0;
}

//��ȫ�ֱ����е����ݱ��浽Vram��
char al_store_screenoff_info(){
	struct sysconf_param env_data;
	__get_env_data(&env_data);
	env_data.auto_screen_off_enable = ScreenoffInfo.auto_screen_off_enable;
	__save_env_data(&env_data);
	return 0;
}

char al_set_screenoff_time(){
	rtc_date_t rtc_date;
	rtc_time_t rtc_time;
	time_t nowsec;
	struct tm nowdate;
	struct tm *powerondate;
	alarm_infor("now is in al_set_screenoff_time, flag is %d",ScreenoffInfo.auto_screen_off_enable);
	if(ScreenoffInfo.auto_screen_off_enable == 1){
		tm_get_rtc(&rtc_date,&rtc_time);
		alarm_infor("rtc time is %04d-%02d-%02d,%02d:%02d:%02d",rtc_date.year,rtc_date.month,rtc_date.day,rtc_time.hour,rtc_time.min,rtc_time.sec);
		nowdate.tm_sec=rtc_time.sec;
		nowdate.tm_min=rtc_time.min;
		nowdate.tm_hour=rtc_time.hour;
		nowdate.tm_mday=rtc_date.day;
		nowdate.tm_mon=rtc_date.month-1;
		nowdate.tm_year=rtc_date.year-1900;
		alarm_infor("nowdate is %02d-%02d-%02d,%02d:%02d:%02d",nowdate.tm_year,nowdate.tm_mon,nowdate.tm_mday,nowdate.tm_hour,nowdate.tm_min,nowdate.tm_sec);
		nowsec = datetosec(&nowdate);
		alarm_infor("nowsec is %ld",nowsec);
		nowsec += (ScreenoffInfo.auto_screen_off_time-1)*60;
		powerondate = sectodate(nowsec);
		ScreenoffInfo.screen_off_date.year = powerondate->tm_year + 1900;
		ScreenoffInfo.screen_off_date.month = powerondate->tm_mon + 1;
		ScreenoffInfo.screen_off_date.day = powerondate->tm_mday;
		ScreenoffInfo.screen_off_time.hour = powerondate->tm_hour;
		ScreenoffInfo.screen_off_time.min = powerondate->tm_min;
		ScreenoffInfo.screen_off_time.sec = powerondate->tm_sec;
		alarm_infor("time to screen off is %04d-%02d-%02d,%02d:%02d:%02d",ScreenoffInfo.screen_off_date.year,ScreenoffInfo.screen_off_date.month,ScreenoffInfo.screen_off_date.day,ScreenoffInfo.screen_off_time.hour,ScreenoffInfo.screen_off_time.min,ScreenoffInfo.screen_off_time.sec);
	}
	return 0;
}

static int auto_screen_off()
{
	int bklight = 1;
	set_back_light(bklight);
	return 0;
}

char screen_off_check()
{
	rtc_date_t rtc_date;
	rtc_time_t rtc_time;
	//alarm_infor("now is screen_off_check");
	if(ScreenoffInfo.auto_screen_off_enable == 1)
	{
		tm_get_rtc(&rtc_date,&rtc_time);
		//alarm_infor("now time is %04d-%02d-%02d,%02d:%02d:%02d",rtc_date.year,rtc_date.month,rtc_date.day,rtc_time.hour,rtc_time.min,rtc_time.sec);
		if(rtc_date.year==ScreenoffInfo.screen_off_date.year && rtc_date.month==ScreenoffInfo.screen_off_date.month && rtc_date.day==ScreenoffInfo.screen_off_date.day && rtc_time.hour==ScreenoffInfo.screen_off_time.hour && rtc_time.min==ScreenoffInfo.screen_off_time.min)
		{
			//alarm_infor("auto screen off freq is %d",ScreenoffInfo.auto_screen_off_enable);
			ScreenoffInfo.auto_screen_off_enable = 0;
			al_store_screenoff_info();
			auto_screen_off();
		}
	}
	return 0;
}

#endif
