/*
********************************************************************************
*                       AM7511T---upgrade.drv
*                (c) Copyright 2002-2007, Actions Co,Ld. 
*                        All Right Reserved 
*
* FileName: upgrade.h     Author: Tian Zhimin        Date:2008/06/26
* Description: defines macros for upgrading
* Others:      
* History:         
* <author>       <time>       <version >    <desc>
* Tian Zhimin  2008/06/26       1.0         build this file
********************************************************************************
*/ 



#ifndef  __OTA_H 
#define  __OTA_H 

#include <sysinfo.h>
#include <am_types.h>
#include<linux/types.h>
#include <ota_updata.h>

#define	__DEBUG_UPGRADE__	1
#define	__DEBUG_ERR__	1
#define __DEBUG_RES__	1

#if __DEBUG_UPGRADE__
#define INFO(stuff...)		printk("<INFO>: " stuff)
#else
#define INFO(stuff...)		do{}while(0)
#endif
#if __DEBUG_ERR__
#define ERR(stuff...)		printk("<ERR> : " stuff)
#else
#define ERR(stuff...)		do{}while(0)
#endif

#if __DEBUG_RES__
#define INFO_R(stuff...)		printk("<INFO>: " stuff)
#else
#define INFO_R(stuff...)		do{}while(0)
#endif


#define	TASK_STK_SIZE			4096
#define	SYSUP_TASK_PRIO			TASKPRIO_BG_TASK0
//#define	MY_TASK_ID			TASKID_BG_TASK0

typedef struct 
{
	INT32U	file_pMODE;		//�ļ�ģʽ
	//FS_u8		file_path[64];
	INT8U *		file_path;	//�ļ�·��������
	struct file *	fp;	//file�ṹָ��
	INT32U		file_length;
	loff_t	file_offset;	//�ļ�ƫ��
	loff_t	offset2;


}File_OP_t;




typedef struct 
{
	char 			MultiSecFlag[16];		// "MultiSectorBoot"
	unsigned int		MbrecNum;				// Mbrec�ķ���,���֧��4��
	unsigned int 		MbrecEncryptFlag;		// Mbrec �Ƿ���ܣ�1���ܣ�0δ����
	unsigned int 		MbrecRunAddr;			// Mbrec ��Sram�е����е�ַ
	unsigned int 		MbrecDecryptAddr;		//����Mbrec�Ĵ�ŵ�ַ������MbrecRunAddrͬ
	unsigned int		Mbrec0_Block;			//��0�� Mbrec ����Block
	unsigned int 		Mbrec0_StartSector;		//��0�� Mbrec ��ʼ����
	unsigned int 		Mbrec0_SectorNum;		//��0�� Mbrec ����������
	unsigned int 		Mbrec0_Reserve;   
	unsigned int 		Mbrec1_Block;			//��1�� Mbrec ����Block
	unsigned int 		Mbrec1_StartSector; 	//��1�� Mbrec ��ʼ����
	unsigned int 		Mbrec1_SectorNum; 		//��1�� Mbrec ����������
	unsigned int 		Mbrec1_Reserve;   
	unsigned int 		Mbrec2_Block;  			//��2�� Mbrec ����Block
	unsigned int 		Mbrec2_StartSector; 	//��2�� Mbrec ��ʼ����
	unsigned int 		Mbrec2_SectorNum;  		//��2�� Mbrec ����������
	unsigned int 		Mbrec2_Reserve;  
	unsigned int 		Mbrec3_Block;  			//��3�� Mbrec ����Block
	unsigned int 		Mbrec3_StartSector;  	//��3�� Mbrec ��ʼ����
	unsigned int 		Mbrec3_SectorNum;  		//��3�� Mbrec ����������
	unsigned int 		Mbrec3_Reserve; 
	unsigned int 		PAGE_CNT_PER_PHY_BLK; //tl
	char 			Reserve0[380];	//tl Reserve0[384];  
	unsigned char 	MbrecDecryptKey[16];	//
	unsigned char    	ChipIDFlag;				//�Ƿ�����ChipID:1 ������,0δ����ChipID��Mbrecδ����
	unsigned char    	ChipIDTransLen;			//����ChipID_To_MbrecDecryptKey��ʹ�õ�ChipID����  
	unsigned char    	ChipIDStartByte;		//����ChipID_To_MbrecDecryptKey�õ���ChipID����ʼbyte    
	unsigned char    	Reserve1;                              
	char 			BootCheckFlag[8];		//"ActBrm",0xaa,0x55
	unsigned int 		BootCheckSum;			//ǰ508��byte��32λ�ۼӺ�+0x1234
}__attribute__ ((packed)) MuiltSectorBoot;



#define SECTOR_RW_ONCE			32
#define BREC_RW_ONCE			(SECTOR_RW_ONCE - 1)


#endif /*__UPGRADE_H */

