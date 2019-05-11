#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys_msg.h>
#include <sys_gpio.h>
#include <sys_rtc.h>
#include <sys_timer.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys_vram.h>

typedef struct 
{
	char  Sys_Flag[16];
	unsigned int Sys_offset;
	unsigned int Sys_len;
	unsigned int Sys_para1;
	unsigned int Sys_para2;
}__attribute__ ((packed))Sys_File;

typedef struct
{
    unsigned int Scodecap;     //ϵͳ����ռ��С������Ϊ��λ
    unsigned int Scodebakcap;  //����ռ䱸������С������Ϊ��λ
    unsigned int Appinfocap;   //Appinfo�ռ��С������Ϊ��λ
    unsigned int Liccap;       //license�ռ��С������Ϊ��λ��flash������ʼ��ʱ����flash��С��license�ռ��Ӧ��ϵ�޸�
    unsigned int Micap;        //Mtp info�ռ��С������Ϊ��λ��flash������ʼ��ʱ����flash��С��mtpinfo�ռ��Ӧ��ϵ�޸�
    unsigned int Dspcap;       //DSP CODE�ռ��С������Ϊ��λ
    unsigned int Autocap;     //�û�����ռ��С������Ϊ��λ��fwmaker���߸����û������޸�
    unsigned int Udiskcap;     //Udisk�ռ��С������Ϊ��λ
    unsigned int Logicalcap;   //����flash���߼��ռ��С������Ϊ��λ
    unsigned int Reserved[3];	
}__attribute__ ((packed)) CapInfo_t;             //48�ֽ�

typedef struct
{
    char LFIFlag[4];        //={0x55,0xaa,0xf2,0x0f};
    char Version[15];       //�汾��xx.xx.xx.xxxx
    char Reserve0;
    char Date[4];           //xxxx.xx.xx
    unsigned short VID;               //xxxx
    unsigned short PID;               //xxxx
    long DIRItemCheckSum;   //���е�240��Ŀ¼���������//DWORD�ۼ�
	//32�ֽ�
	
    char OEMName[16];
    char Designer[16];
    char Productor[32];
    char DeviceName[32];
    CapInfo_t CapInfo;      //48�ֽ�
    char SysComVar[64];     //64�ֽ�
    char Reserve1[16];      //����
	//7*32
	
    char UsbVendor[8];
    char UsbProductID[16];
    char UsbVersion[8];
    char UsbSetupInfo[48];
    char Mtpmfrinfo[33];
    char Mtpprodinfo[33];
    char Mtpprodver[17];
    char Mtpprodsn[17];
    char MtpVID[2];
    char MtpPID[2];
    char Reserve2[15];      //����
    char Volume[5][11];
    unsigned short Headchecksum;      //ǰ510byteУ���, WORD�ۼ�
	//    LDIR_t diritem[240];    //�߼�Ŀ¼��7680byte
} __attribute__ ((packed)) LFIHead_t;                 //8k



void static hex_printf2(  const INT8U* pData, INT16U inLen);
int static destory_kernel(int flag1,int flag2);
int static destory_root(int flag1);

 int Destory_FW(unsigned int sum_param,char * param_buf);