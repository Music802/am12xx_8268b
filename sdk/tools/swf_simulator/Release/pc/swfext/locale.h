#ifndef SWF_LOCALE_20090902_H
#define SWF_LOCALE_20090902_H

#include "swf_types.h"
#include "act_plugin.h"

#define		MAX_ID_NAME_LEN				60
#define		MAX_TEXT_LEN				256
#define		MAX_APP_NAME_LEN			16
#define		MAX_LANG_ID_LEN				6

#define DWORD 	unsigned long

#pragma pack(1)
// �ļ�ͷ�ṹ (16)Byte
typedef struct tagFUI_RES_HEADER
{
	char Magic[2];
	char Version[2];
	DWORD dwFileLen;
	unsigned short SheetCnt;    // �ַ�������  (2byte)	
	unsigned short LangCnt;
	char Reserved[4];
	char *pLangId;    // ��¼���е�����id �� �ַ���Ϣ������
}FUI_RES_HEADER, *PFUI_RES_HEADER;


// һ��������ṹ ((16+2+4)* SheetCnt) Byte
typedef struct tagFUI_KEYS1_TAB
{
	char AppName[MAX_APP_NAME_LEN];    //AP������������ �����������ؼ���
	unsigned short StrCnt;  // �ַ���������ʾ��AP���ж��ٸ��ַ�������	
	DWORD StartAddr;        // ָ�������������ʼ��ַ��������ļ�ͷ��ʼ��
}FUI_KEYS1_TAB, *PFUI_KEYS1_TAB;

// ����������ṹ
typedef struct tagFUI_RES_TABLE
{
	char   StrID[MAX_ID_NAME_LEN];    
	DWORD  dwStartAddr;	   // ������ʵ��ַ
	unsigned short *pOffset;    // �����Ե����ƫ��ֵ
}FUI_RES_TABLE, *PFUI_RES_TABLE;

typedef struct tagFUI_RES_HASH_TAB
{
	int    hash;
	DWORD  OffAddr;	
}FUI_RES_HASH_TAB, *PFUI_RES_HASH_TAB;

typedef struct tagFUI_RES_STR{
	stream_input_t  *pb;
	FUI_RES_HEADER	header;
	PFUI_KEYS1_TAB  pKeys1;   // ��¼һ���������������ݡ�
	int		LangIdx;   //��¼��ǰ���Ե�����ֵ������ʾ�ڼ�������.
	int     AppIdx;    //��¼��ǰAPP������ֵ������ʾ�ڼ���APP.
	char *  pData;
}FUI_RES_STR, *PFUI_RES_STR;

#pragma pack()

void * open_res(char *filename);
void close_res();

#endif