/**
*******************************************************************************
* @file
* @brief  ebook header file.
*
<PRE>
               Copyright (c) 1999-2009 Artek Corporation
      All or Portions of This Software are Copyrighted by Artek
                       Company Proprietary
</PRE>
* Define struct and micro for ebook engine.\n
* @author hikwlu
* @version  v1.00
* @date	2010/07/22
* @see EBook.c
* @ingroup Ebook_A 
********************************************************************************
*/

/**
* @addtogroup Ebook_A
* @{
*/

#ifndef _EBOOK_H
#define _EBOOK_H

#ifdef MODULE_CONFIG_EBOOK
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "sys_cfg.h"
#include "system_res.h"
/*
#define INT32U	unsigned int
#define INT32S	int
#define INT8U	unsigned char
#define INT8S	char
#define INT16S	short
#define INT16U	unsigned short
*/
#define TRUE		1
#define FALSE	0

#if  __DEBUG_EBOOKS__ == 0
//#define printf(...)     
#endif

#define		SWF_EBOOK_H
#define		BG_EBOOK_STACK_SIZE		(2*1024)
#define		EB_PAGESAVE_LENGTH		(40*1024)

#define		EBOOK_BG_ERROR			(-1)
//for EBOOK Engine CMD define
#define		GET_CUR_LINE_NUM			0x40
#define		GET_CUR_PAGE_NUM			0x41
#define		GET_TOTAL_PAGE_NUM		0x42
#define		GET_SPECIAL_LINE_DATA	0x43
#define		COUNT_TOTAL_PAGE			0x44

#define		GOTO_SPECIAL_LINE			0x45
#define		GOTO_SPECIAL_PAGE			0x46
#define		GOTO_SPECIAL_POINT		0x47
#define		CONTINUE_SHOW_ONE_LINE	0x48
#define		GET_FILE_TYPE				0x49
#define		GOTO_CUR_POINT			0x50
#define		BG_EBOOK_TASK_STOP		0x51
#define		GET_TOTAL_LINE_NUM		0x52

#define		RTC2HZ						0xFF

#define		MBCS_SPECIAL_VALUE		0xfffe

#define		SAVE_OFFSET_PAGE_SMALL	10	//С��Ļʱ��SAVE_OFFSET_PAGE_SMALL   ҳ����ƫ��
#define 		SAVE_OFFSET_PAGE_LARGE	4	//����Ļʱ��SAVE_OFFSET_PAGE_LARGE   ҳ����ƫ��

//���ִ���Ļ��С��Ļ
#define		EB_WIDTH_VALUE_MIN		480
#define		EB_HEIGTH_VALUE_MIN		234

#define		ONE_SECTOR_LENGTH		512
#define		REVERSE512					0xfffffe00

#define		CHAR_LENGTH_SIZE			256		//�����ַ���ȵ�����Ĵ�С

#define		PAGE_OFFSET_SAVE_SIZE	2048		//����ҳ������Ĵ�С

#define		LINE_MAX_BYTES			512		//  buffer size for each line
#define		MAX_FIND_PRELINE_POINT	256	//��ǰ��ҳʱ��������ƫ����

#define		SUPPORT_MAX_MARK			0
#define		MARK_NAME_SIZE			200
//�������ܶϴ�0-9,A-Z,a-z
#define		EBOOK_SPECIAL_CHAR(c)    ((c>= 0x30&&c<= 0x39)||(c>= 0x41&&c<= 0x5a)||(c>= 0x61&&c<= 0x7a) )

#define		EBOOK_CJK_CHAR(c)    ((c>= 0x4e00&&c<= 0x9fa5)||(c>= 0xf900&&c<= 0xfa2d))

#define		EBOOK_CJK_CHAR_EXT(c)    ((c>= 0x4e00&&c<= 0x9fa5)||(c>= 0xf900&&c<= 0xfa2d)\
									||(c>= 0x3000&&c<= 0x318f)||(c>= 0x1100&&c<= 0x11ff)||(c>= 0xac00&&c<= 0xdfaff))


#define		FILE_SIZE_1MB				0x100000
#define		FILE_SIZE_1KB				0x400

//#define SHOWBUF_LENGTH   (MAX_LINE_EN*LINE_MAX_BYTES)

#define		MAX_BOOKMARK				32
#define		NEW_LINE_CHAR				0x0a
#define		NEW_LINE_CHAR_BIG		0x0a00
#define		SPACE_CHAR					0x20
#define		TAB_CHAR					0x09
#define		MAX_CTL_CHAR				0x20
#define		END_STRING_CHAR			0x00
#define		ASCII_CODE					0x80

#define		LINE_HIGHT					30

#define		MSG_AUTOROLL				0x40
#define		MSG_MYEND					0x41
#define		MSG_CARDOUT				0x42

#define		EXIT_EBOOK					4
#define		CARD_PULL					5

#define		EBOOK_OSD_BUTTON_MAX	7
#define		EBOOK_SEARCH_ONE			1
#define		LIST_DELAY_SHOW			2


typedef enum
{
	EBOOK_MANUAL=0,
	EBOOK_AUTO
}EBOOK_MODE;  

typedef enum
{
	MBCS = 0,
	UNI16_BIG,//unicode big endian
	UNI16_LIT,//unicode little endian
	UTF8
}txt_type_t;

enum
{
    IN_LIST = 0,
    IN_READING
};


enum ebook_read_sector{
	READ_SECTOR_FAILE= 0,
	READ_SECTOR_OK,
	TO_THE_END,
	TO_THE_HEAD
};

typedef struct {//30 byte
	INT32U mark_page;//cur_line
	INT32U total_page;
	INT8S   txtFileName[MARK_NAME_SIZE];
}ebook_mark_info;

typedef struct {
	ebook_mark_info mark;//[SUPPORT_MAX_MARK];//26*5=130 byte
	INT32U txtcolor;//cur_line
	INT32U txtsize;
}EBOOK_APP_VRAM;

typedef struct
{
	FILE* fp;
	txt_type_t  type;			//txt �ļ�����
	INT8U *showbuf;			//show buffer ��Ŀǰ��һ�еĴ�С
	INT8U *bookbuffer;			//�����һ��sector ���ݴ���ڴ�buffer
	INT32U *PageOffsetSave;	//ҳ��ƫ����Ϣ�����ڿ��ٶ�λ
	INT32U TxtW;				//�ı���
	INT32S  MaxLine;			//һҳ�����ɵ�����
	INT8U LineSaveStep;		//��LineSaveStep ҳ����ƫ����

	INT32S bookbuffer_len;		//Ҫ��һ��sector �����ݣ�ʵ���϶����Ĵ�С
	INT32S sector;    			//����Ϊһ��sector�ı�����
	INT32S sector_offset;  		//sector�ڵ�ƫ��
	INT32S cur_point; 			//��ǰһ���Ŀ�ʼ�ַ��������ļ��е�ƫ����
	INT32S TotlePage;
	INT32S CurPage;
	INT32S totalLine;
	INT32S curLine;
}eb_show_var_t;


typedef struct
{
	INT32S line;
	INT8U *buf;
	INT8U unicode;
}special_line_t;



typedef struct
{   
    int focusWin;
    char (*dealkey)(unsigned short key);
}win_control_t;

typedef struct
{ 
	win_control_t	winctrl;
	eb_show_var_t showvar;
	ebook_mark_info mark;

}book_var_t;

#define EB_reading_LineSpace	10
#define EB_reading_Tex_F		16

enum pisel_err{			//see alien-error.h for details
    PicselDocumentError_AgentMatchFailed = (1<<16),
    PicselDocumentError_AgentMatchFailedNoData,
    PicselDocumentError_DocumentTranslationFailed,
    PicselDocumentError_InternalError,
    PicselDocumentError_DocumentPasswordProtected,
    PicselDocumentError_UnsupportedCharset,
    PicselError_OutOfMemory,
    PicselError_SettingsPathUndefined,
    PicselError_UnsupportedJSPopup,
    PicselError_UploadFileTooLarge,
    PicselError_LastErrorUnused,
    PicselError_Unknow,
};


INT32S   ebook_bg_ioctl(INT32U  cmd, void *param);

#endif	/** MODULE_CONFIG_EBOOK */

#endif
