#include "sys_resource.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "apps_vram.h"

#define SUBTILT_DEBUG

typedef struct laninfo_s
{
	short int group;				////������һ��
	int codepage; 			///code page 
	short int  idx;					///���ڶ�������
	unsigned char exts[5];			///��Ҫ��ӵĺ�������
	
}laninfo_t;

typedef struct groupinfo_s
{
	short int groupidx;				///������
	short int idx_min;				///������Сֵ(��������)
	short int idx_max;				///�������ֵ(��������)
}groupinfo_t;

#define sp_info(fmt,arg...) //printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#define sp_err(fmt,arg...) //printf("MERR[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)

groupinfo_t groupinfo_array[]=
{
	{0 ,0,0},
	{1 ,0,0},
	{2 ,0,14},
	{3 ,0,0},
	{4 ,0,2},
	{5 ,0,8},
	{6 ,0,11},
	{7 ,0,65},
	{8 ,0,0},
	{9 ,0,2},
	{10,0,0},
	{11,0,16},
	{12,0,2},
	{13,0,0},
};


laninfo_t laninfo_array[]=
{
	{0,874,0,"-THA"},		//̩��                    					
	{1,932,0,"-JPN"},             //����                    
	{2,936,0,"-HYE"},             //����������              
	{2,936,1,"-KAT"},             //��³������              
	{2,936,2,"-HIN"},             //ӡ����                  
	{2,936,3,"-PAN"},             //��������                
	{2,936,4,"-GUJ"},             //�ż�������              
	{2,936,5,"-TAM"},             //̩�׶���                
	{2,936,6,"-TEL"},             //̩¬����                
	{2,936,7,"-KAN"},             //��������                
	{2,936,8,"-MAR"},             //��������                
	{2,936,9,"-SAN"},             //����                    
	{2,936,10,"-KNK"},            //�׿�����                
	{2,936,11,"-SYR"},            //��������                
	{2,936,12,"-DIV"},            //��ά����                
	{2,936,13,"-CHS"},            //����(�й�)              
	{2,936,14,"-ZHI"},            //����(�¼���)            
	{3,949,0,"-KOR"},             //������                  
	{4,950,0,"-CHT"},             //����(̨��)              
	{4,950,1,"-ZHH"},             //����(����ر�������)    
	{4,950,2,"-ZHM"},             //����(�����ر�������)    
	{5,1250,0,"-CSY"},            //�ݿ���                  
	{5,1250,1,"-HUN"},            //��������                
	{5,1250,2,"-PLK"},            //������                  
	{5,1250,3,"-ROM"},            //����������              
	{5,1250,4,"-HRV"},            //���޵�����              
	{5,1250,5,"-SKY"},            //˹�工����              
	{5,1250,6,"-SQI"},            //������������            
	{5,1250,7,"-SLV"},            //˹����������            
	{5,1250,8,"-SRL"},            //����ά����(������)      
	{6,1251,0,"-BGR"},            //����������              
	{6,1251,1,"-RUS"},            //����                    
	{6,1251,2,"-UKR"},            //�ڿ�����                
	{6,1251,3,"-BEL"},            //����ʱ��                
	{6,1251,4,"-MKI"},            //�������(FYROM)         
	{6,1251,5,"-KKZ"},            //��������                
	{6,1251,6,"-KYR"},            //������˹��(�������)    
	{6,1251,7,"-TTT"},            //������                  
	{6,1251,8,"-MON"},            //�ɹ���(�������)        
	{6,1251,9,"-AZE"},            //�����ݽ���(�������)    
	{6,1251,10,"-UZB"},           //���ȱ����(�������)    
	{6,1251,11,"-SRB"},           //����ά����(�������)    
	{7,1252,0 ,"-CAT"},           //��̩¡��                
	{7,1252,1 ,"-DAN"},           //������                  
	{7,1252,2 ,"-DEU"},           //����(�¹�)              
	{7,1252,3 ,"-ENU"},           //Ӣ��(����)              
	{7,1252,4 ,"-ESP"},           //��������(��ͳ)          
	{7,1252,5 ,"-FIN"},           //������                  
	{7,1252,6 ,"-FRA"},           //����(����)              
	{7,1252,7 ,"-ISL"},           //������                  
	{7,1252,8 ,"-ITA"},           //�������(�����)        
	{7,1252,9 ,"-NLD"},           //������(����)            
	{7,1252,10,"-NOR"},           //Ų����(����÷��)        
	{7,1252,11,"-PTB"},           //��������(����)          
	{7,1252,12,"-SVE"},           //�����                  
	{7,1252,13,"-IND"},           //ӡ����������            
	{7,1252,14,"-EUQ"},           //��ʿ����                
	{7,1252,15,"-AFK"},           //�Ϸ���                  
	{7,1252,16,"-FOS"},           //������                  
	{7,1252,17,"-MSL"},           //������(��������)        
	{7,1252,18,"-SWK"},           //˹��ϣ����              
	{7,1252,19,"-GLC"},           //����������              
	{7,1252,20,"-DES"},           //����(��ʿ)              
	{7,1252,21,"-ENG"},           //Ӣ��(Ӣ��)              
	{7,1252,22,"-ESM"},           //��������(ī����)        
	{7,1252,23,"-FRB"},           //����(����ʱ)            
	{7,1252,24,"-ITS"},           //�������(��ʿ)          
	{7,1252,25,"-NLB"},           //������(����ʱ)          
	{7,1252,26,"-NON"},           //Ų����(��ŵ˹��)        
	{7,1252,27,"-PTG"},           //��������(������)        
	{7,1252,28,"-SVF"},           //�����(����)            
	{7,1252,29,"-MSB"},           //������(������³����)    
	{7,1252,30,"-DEA"},           //����(�µ���)            
	{7,1252,31,"-ENA"},           //Ӣ��(�Ĵ�����)          
	{7,1252,32,"-ESN"},           //��������(����)          
	{7,1252,33,"-FRC"},           //����(���ô�)            
	{7,1252,34,"-DEL"},           //����(¬ɭ��)            
	{7,1252,35,"-ENC"},           //Ӣ��(���ô�)            
	{7,1252,36,"-ESG"},           //��������(Σ������)      
	{7,1252,37,"-FRS"},           //����(��ʿ)              
	{7,1252,38,"-DEC"},           //����(��֧��ʿ��)        
	{7,1252,39,"-ENZ"},           //Ӣ��(������)            
	{7,1252,40,"-ESC"},           //��������(��˹�����)    
	{7,1252,41,"-FRL"},           //����(¬ɭ��)            
	{7,1252,42,"-ENI"},           //Ӣ��(������)            
	{7,1252,43,"-ESA"},           //��������(������)        
	{7,1252,44,"-FRM"},           //����(Ħ�ɸ�)            
	{7,1252,45,"-ENS"},           //Ӣ��(�Ϸ�)              
	{7,1252,46,"-ESD"},           //��������(������ӹ��͹�)
	{7,1252,47,"-ENJ"},           //Ӣ��(�����)            
	{7,1252,48,"-ESV"},           //��������(ί������)      
	{7,1252,49,"-ENB"},           //Ӣ��(���ձȺ�)          
	{7,1252,50,"-ESO"},           //��������(���ױ���)      
	{7,1252,51,"-ENL"},           //Ӣ��(������)            
	{7,1252,52,"-ESR"},           //��������(��³)          
	{7,1252,53,"-ENT"},           //Ӣ��(�������)          
	{7,1252,54,"-ESS"},           //��������(����͢)        
	{7,1252,55,"-ENW"},           //Ӣ��(��Ͳ�Τ)          
	{7,1252,56,"-ESF"},           //��������(��϶��)      
	{7,1252,57,"-ENP"},           //Ӣ��(���ɱ�)            
	{7,1252,58,"-ESL"},           //��������(����)          
	{7,1252,59,"-ESY"},           //��������(������)        
	{7,1252,60,"-ESZ"},           //��������(������)        
	{7,1252,61,"-ESB"},           //��������(����ά��)      
	{7,1252,62,"-ESE"},           //��������(�����߶�)      
	{7,1252,63,"-ESH"},           //��������(�鶼��˹)      
	{7,1252,64,"-ESI"},           //��������(�������)      
	{7,1252,65,"-ESU"},	          //��������(�������(��))  
	{8,1253,0,"-ELL"},	          //ϣ����                  
	{9,1254,0,"-TRK"},	          //��������                
	{9,1254,1,"-AZE"},	          //�����ݽ���(������)      
	{9,1254,2,"-UZB"},            //���ȱ����(������)      
	{10,1255,0,"-HEB"},	          //ϣ������                
	{11,1256,0 ,"-URD"},	        //�ڶ���                  
	{11,1256,1 ,"-FAR"},	        //��˹��                  
	{11,1256,2 ,"-ARI"},          //��������(������)        
	{11,1256,3 ,"-ARE"},	        //��������(����)          
	{11,1256,4 ,"-ARL"},	        //��������(������)        
	{11,1256,5 ,"-ARG"},	        //��������(����������)    
	{11,1256,6 ,"-ARM"},          //��������(Ħ���)        
	{11,1256,7 ,"-ART"},	        //��������(ͻ��˹)        
	{11,1256,8 ,"-ARO"},	        //��������(����)          
	{11,1256,9 ,"-ARY"},	        //��������(Ҳ��)          
	{11,1256,10,"-ARS"},          //��������(������)        
	{11,1256,11,"-ARJ"},	        //��������(Լ��)          
	{11,1256,12,"-ARB"},	        //��������(�����)        
	{11,1256,13,"-ARK"},	        //��������(������)        
	{11,1256,14,"-ARU"},          //��������(������)        
	{11,1256,15,"-ARH"},	        //��������(����)          
	{11,1256,16,"-ARQ"},	        //��������(������)        
	{12,1257,0,"-ETI"},	          //��ɳ������              
	{12,1257,1,"-LVI"},           //����ά����              
	{12,1257,2,"-LTH"},	          //��������                
	{13,1258,0,"-VIT"},	          //Խ����                  
};

#define DIR_ADD		0x01
#define DIR_SUB		0x00

#define GROUP_ID_MAX 13
#define GROUP_ID_MIN  0

short int group_id=2;
short int subidx_id=13;///����Ϊ����

/*
*get the system language and match the lang array id for codepage 
*/

void get_system_lang(){
	int  lang = 0;
	struct sysconf_param sys_cfg_data;
	_get_env_data(&sys_cfg_data);
	lang = sys_cfg_data.local_language_id;
	switch(lang){		//the language order is match the system language which list in fui.xls
		case LAN_ENGLISH:
			group_id = 7;
			subidx_id = 21;
			break;
		case LAN_FRENCH:
			group_id = 7;
			subidx_id = 23;
			break;
		case LAN_GERMAN:
			group_id = 7;
			subidx_id = 2;
			break;
		case LAN_SPANISH:
			group_id = 7;
			subidx_id = 22;
			break;
		case LAN_PORTUGUESE:
			group_id = 7;
			subidx_id = 27;
			break;
		case LAN_CHS:
			group_id = 2;
			subidx_id = 13;
			break;
		case LAN_CHT:
			group_id = 4;
			subidx_id = 0;
			break;
		case LAN_ITALIAN:
			group_id = 7;
			subidx_id = 8;
			break;
		case LAN_NORWEGIAN:
			group_id = 7;
			subidx_id = 10;
			break;
		case LAN_SWEDISH:
			group_id = 7;
			subidx_id = 12;
			break;
		case LAN_DUTCH:
			group_id = 7;
			subidx_id = 9;
			break;
		case LAN_RUSSIAN:
			group_id = 6;
			subidx_id = 1;
			break;
		case LAN_POLISH:
			group_id = 5;
			subidx_id = 2;
			break;
		case LAN_FINNISH:
			group_id = 7;
			subidx_id = 5;
			break;
		case LAN_GREEK:
			group_id = 8;
			subidx_id = 0;
			break;
		case LAN_KOREAN:
			group_id = 3;
			subidx_id = 0;
			break;
		case LAN_HUNGARIAN:
			group_id = 5;
			subidx_id = 1;
			break;
		case LAN_CZECH:
			group_id = 5;
			subidx_id = 0;
			break;
		case LAN_ARABIC:
			group_id = 11;
			subidx_id = 16;
			break;
		case LAN_TURKISH:
			group_id = 9;
			subidx_id = 0;
			break;
		default : 
			break;		
	}
	//printf("%s,%d,lang is %d,\t group_id is %d,\t subidx_id is %d\n",__FUNCTION__,__LINE__,lang,group_id,subidx_id);
}

/*get the codepage*/
int subtitle_get_codepage(short int groupid,short int subidx,unsigned char * exts)
{
	int codepage=0;
	int i=0;
	int array_len = sizeof(laninfo_array)/sizeof(laninfo_t);
	laninfo_t *laninfo=NULL;
	//printf("%s,%d,group_id is %d,\t subidx_id is %d\n",__FUNCTION__,__LINE__,group_id,subidx_id);
	for(i=0;i<array_len;i++){
		laninfo = laninfo_array+i;
		if(laninfo->group == groupid && laninfo->idx==subidx){
			memcpy(exts,laninfo->exts,5);
			codepage =  laninfo->codepage;
		}
	}
	
	return codepage;
}

short int subtitle_set_groupidx(int dir)
{	
	int code_page=0;
	unsigned char exts[5];

	#ifndef SUBTILT_DEBUG
	return -1;
	#endif
	
	if(dir==DIR_ADD){
		group_id ++;
		if(group_id>GROUP_ID_MAX)
			group_id=GROUP_ID_MIN;
	}
	else{
		group_id --;
		if(group_id<GROUP_ID_MIN)
			group_id=GROUP_ID_MAX;
	}
	code_page=subtitle_get_codepage(group_id,0,exts);

	sp_info("Code Page==%d",code_page);
	return group_id;
}


short int subtitle_set_subidx(int dir)
{
	int code_page=0;
	unsigned char exts[5];

	#ifndef SUBTILT_DEBUG
	return -1;
	#endif
	
	if(dir==DIR_ADD){
		subidx_id ++;
		if(subidx_id>groupinfo_array[group_id].idx_max)
			subidx_id=groupinfo_array[group_id].idx_min;
		if(subidx_id<groupinfo_array[group_id].idx_min)
			subidx_id=groupinfo_array[group_id].idx_max;
	}
	else{
		subidx_id --;
		if(subidx_id<groupinfo_array[group_id].idx_min)
			subidx_id=groupinfo_array[group_id].idx_max;
		if(subidx_id>groupinfo_array[group_id].idx_max)
			subidx_id=groupinfo_array[group_id].idx_min;
	}
	
	code_page=subtitle_get_codepage(group_id,subidx_id,exts);
	sp_info("SUB IDX==%d, codepage==%d,exts=%s",subidx_id,code_page,exts);
	return subidx_id;
}





int subtitle_extend_filename(char *src_name,char**dest_name)
{
#ifndef SUBTILT_DEBUG
	return -1;
#endif

	int code_page=0;
	int name_len;
	int i=0;
	unsigned char exts[5];
	if(!src_name || !dest_name)
		return -1;
	code_page=subtitle_get_codepage(group_id,subidx_id,exts);

	if(code_page==0)
		return -1;
	
	name_len= strlen(src_name);
	if(name_len){
		*dest_name = malloc(name_len+5);
		if(*dest_name == NULL)
			return -1;
		memset(*dest_name,0,name_len+5);

		for(i=name_len-1;i>=0;i--){///�ȸ��ƺ�׺��
			*(*dest_name+i+4) = src_name[i];
			if(src_name[i]=='.')
				break;
		}
		
		memcpy(*dest_name,src_name,i);///���ļ������ƹ�ȥ
		memcpy(*dest_name+i,exts,4);///�����������
	}
	else
		return -1;

	sp_info("EXTEND SUBTITLE FILE=%s",*dest_name);
	return 0;
}

void subtitle_free_filename(char**dest_name)
{
#ifndef SUBTILT_DEBUG
	return ;
#endif

	if(!dest_name && *dest_name!=NULL)
		free(*dest_name);
}

int subtitle_get_charset()
{
	int src_charset=LAN_GB2312;
	int code_page=0;
	unsigned char exts[5];
	code_page=subtitle_get_codepage(group_id,subidx_id,exts);
	switch(code_page){
		case 847:
			src_charset = LAN_CP874;
			break;
		case 932:
			src_charset = LAN_CP932;
			break;
		case 936:
			src_charset = LAN_GB2312;
			break;
		case 949:
			src_charset = LAN_CP949;
			break;
		case 950:
			src_charset = LAN_CP950;
			break;
		case 1250:
			src_charset = LAN_CP1250;
			break;
		case 1251:
			src_charset = LAN_CP1251;
			break;
		case 1252:
			src_charset = LAN_CP1252;
			break;
		case 1253:
			src_charset = LAN_CP1253;
			break;
		case 1254:
			src_charset = LAN_CP1254;
			break;
		case 1255:
			src_charset = LAN_CP1255;
			break;
		case 1256:
			src_charset = LAN_CP1256;
			break;
		case 1257:
			src_charset = LAN_CP1257;
			break;
		case 1258:
			src_charset = LAN_CP1258;
			break;
		default:
			sp_err("code page error!");
	}

	return src_charset;
}


