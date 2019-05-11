/**
*******************************************************************************
* @file	FileSelector.h
* @brief �ļ�ѡ����API
*
*   ����app��Ҫ������ָ�����͵��ļ��б�
* 
* 
* @author ����
* @version  �汾
* @date	����
* @warning д�Ϲ��ھ����˵��
* @bug д�Ϲ���bug��˵��
* @deprecated д�Ϲ�ʱ��˵��������˵�����ļ���ʲôʱ��͹�ʱ��û����
* @todo д�Ͻ�Ҫ������
* @see д��Ҫ�ο����ļ����߱���
* @ingroup media_nav_s
********************************************************************************/


/**
* @addtogroup media_nav_s
* @{
*/

#ifndef _FILESELECTOR_H_
#define _FILESELECTOR_H_

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h> 
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "ipc_key.h"


#ifdef __cplusplus                     
extern "C" {
#endif

#ifndef TRUE
#define TRUE	 1
#endif
#ifndef FALSE
#define FALSE	 0
#endif

typedef int bool;
#define FULL_PATH_SIZE	(1024)

#if 0
union semun {
    int val;						/* value for SETVAL */
    struct semid_ds *buf;		/* buffer for IPC_STAT & IPC_SET */
    unsigned short *array;		/* array for GETALL & SETALL */
    struct seminfo *__buf;		/* buffer for IPC_INFO */
    void *__pad;
};
#if 1
typedef enum
{
	NAND_DISK = 0,
	CARD_DISK = 1,
	HARD_DISK = 2,
	USB_DISK1 = 3,
	USB_DISK2 = 4,
	USB_DISK3 = 5,
	USB_DISK4 = 6,
	ROOT_DISK = 7,
	PRIV_DISK=8,
	DISK_NUM  = 9
}Disk_Type;
#endif

struct dirinfo;

/** 
 * @brief �ļ����������ڵ�
*
* ��ϸ˵����
*/
typedef struct dirinfo{
    int filenum;                      	/**< ��ǰ�ļ����µ���ЧͼƬ���� */
    struct dirinfo * children;         	/**< ָ���һ�����ļ��нڵ�*/
    struct dirinfo * brother;          	/**< ָ�����ڵ���һ���ļ��нڵ�*/
    struct dirinfo * parent;             /**< ָ���ļ��нڵ� */
 //   void * parent;
}dir_info_t;

typedef struct{
     struct dirinfo *  * buf;
     char              *  * ext;
     int num;
}dir_info_buffer_t;

#define MAX_DIR_NUM 1000 	// must be not less than  2 

#define FULL_PATH_SIZE	(1024)
#define MAX_EXT_LEN 		(12)
#define MAX_DIR_LEVEL	(10+3)

#define MAX_NAME_LEN	(768)

#define PATH_SEPARATOR	'/'

 struct scan_task_info;

#define mnavi_inf(fmt,arg...) printf("MINF[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)

#define mnavi_err(fmt,arg...) printf("MERR[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)
#define mnavi_debug(fmt,arg...)  //printf("MDEBUG[%s-%d]:"fmt"\n",__func__,__LINE__,##arg)


/**the choices for pFsel->property*/
#define    NEED_FILE_NUM 	(0x01<<0)
#define    NEED_NAME_BUFFER 	(0x01<<1)
#define    NEED_EXIT	(0x01<<2)
#define    EXTERN_BUFFER 	(0x01<<3)

#define MNAVI_SECTOR_SIZE (512)


/** 
 * @brief �ļ�ѡ����
*
* ��ϸ˵����
*/
typedef struct {
//    bool Idle;
    int StartNO;        				 /**< ѡ�е���Ҫ��ѯ�ļ��б����ʼλ��*/
    char level;              				 /**< ��ǰ����Ŀ¼de Ŀ¼���*/			
    int FileTotal;    					 /**< ��ǰĿ¼����Ч�ļ�����(��������Ŀ¼)*/
    int DirTotal;    					 /**< ����Ŀ¼��Ŀ¼������(��������Ŀ¼)*/
    int file_sub_total; 				 /**<  ����Ŀ¼���ļ�����(������Ŀ¼��)*/
    int FileNo;      					 /**< ��ǰ�ļ����*/
    int file_sub_No; 	 			 /**< �ļ��ڹ���Ŀ¼�µ����(������Ŀ¼��)*/
    int start_file_sub;				 /**<��ʼ�������ļ����(������Ŀ¼)*/
    char *Ext;            				 /**< �ļ����˺�׺��*/
    char * cur_ext;   				 /**< ��ǰ��׺��*/
    char CurExt[MAX_EXT_LEN+1];  	 /**< ��ǰ��׺��*/
    char sub;          					 /**< ��Ŀ¼ ����*/
	pid_t scan_process_pid; /**<��������ɨ����̵�PID���������˳���ʱ����Ҫ�ȴ����̽���>**/
   DIR *pDir;    					 /**< Ŀ¼����    */
   //FS_Handle   pDir;      			 /**< Ŀ¼����    */
//    FS_Entry_t  Entry;
   struct dirent *dirent; 				 /**< for save dir info from fs */
    char *path;    					 /**< root path*/
    unsigned char root_level;  		 /**< depth of the root path */
    long int dirpos[MAX_DIR_LEVEL];
    dir_info_t * dir_info;   			 /**< scaned dir info*/
    dir_info_t * cur_dir_info;			 /**< the dir info of the last searched file*/
   struct scan_task_info *scan_task; 	 /**< scan task's info*/
    //unsigned char task_hold;  		 /**< indicate whether  the fsel hold the task or not*/
    unsigned char property;  			 /**< some property of the fsel */
   char cur_file[FULL_PATH_SIZE];		 /**< buffer for saving  founded file path and so on */
   char cur_path[FULL_PATH_SIZE];  	 /**< worh path*/
   unsigned char not_back;     		 /**< useless now*/
   dir_info_buffer_t dir_info_buffer;
   struct{
       DIR *_pDir;
       char * _cur_path;
       int  _file_num;
       dir_info_t * _current;
       unsigned char _next;    
   }sync;
   char fsel_index;
   char ScanMode;					/**<ScanMode=1:create a task for scanning*/
								/**<ScanMode=0:the task will no be created*/
}Fsel_t;

/** 
 * @brief ��̨ɨ������
*
* ��ϸ˵����
*/
struct scan_task_info{
  // Fsel_t *pFsel;
   //OS_STK  * stack;   		/**<task stack */
   unsigned char  runing;      /**<task status */
   unsigned char prio;           /**<task priority */
   unsigned short ID;            /**<task ID */
   unsigned char task_hold;   
   unsigned char task_exit;
   //OS_EVENT * task_com;  /**<OS semphore,for waiting the scan task ready to be deleted  */
};

//struct scan_task_info scan_task;

enum{
	FT_DIR=0,// Ŀ¼��
	FT_FILE,// �ļ���
	FT_MUSIC,
	FT_VIDEO,
	FT_IMAGE,
	FT_VVD,
	FT_TXT
};
#define MAX_FILE_TYPE FT_TXT

enum
{
    GET_TOTAL_FILE_NUM=0,
    GET_FILE_NUM,
    GET_DIR_NUM,
    START_SCAN_DIR,          		  // ��ʼĿ¼ɨ�����񣬽����ļ���Ϣ
    DEL_SCAN_TASK,          			  // ɾ��Ŀ¼ɨ������
    GET_SCANED_FILE_NUM,               //��ǰɨ�赽���ļ�����
    GET_TASK_STATUS,
    REFRESH_FSEL
};

enum{
   EDLE=0,
   RUNING,
   COMPLETED
};


/** 
 * @brief �ⲿbuffer�������ѯ��Ŀ��·������Ϣ
*
* ��ϸ˵����
*/

typedef struct
{
 	int type;                 			/**<item type,file or dir */
	unsigned int length; 			/**<length of the file name*/
	char content[FULL_PATH_SIZE]; 	/**<full path of the item */
	char filename[MAX_NAME_LEN];	/**<filename of the item*/
	char *path;        				/**<not uesed now */
	int index;            				/**<the item's index */
}filename_t;


/*typedef struct {
	int index;
	int valid;
	char  file[FULL_PATH_SIZE];
} LIST_NODE;*/

/**
* @brief  �ļ�ѡ������ʼ������
*
* @param[in]  disk ϣ��������ȫ·��.
* @param[in]  Ext �ļ�ѡ�����Ĺ������͡� �ļ�ѡ����ͨ���趨�ĺ�׺�������ļ�
* @param[in]  property �ļ�ѡ������һЩ����
* -NEED_FILE_NUM ��ʾ���ļ�ѡ������ͬʱ��õ�ǰĿ¼���ļ��к��ļ��ĸ���
* -NEED_NAME_BUFFER ÿ�λ�ȡһ���������ļ����Ƶ���Ϣ������buffer��
* @return	�ļ�ѡ�������,���ļ�ʧ��ʱ���ؿ�ָ��

* @author  ����
* @version�汾
* @date    ����
* @warning 
*         1��ÿ����׺������Ϊ3���� �ո�Ϊ��������
*               ����const char ExtMusic[ ] = "MP3 WMA ASF WMV WAV ACT OGG M4A AA  ";
*         2������׺�����Ȳ���3 ʱ���Կո��㣻
*         3����׺��Ϊ3 ���ո�ʱ���ļ�ѡ��������ʾ�����ļ���
* @code	
*	pFsel  = fsel_open( ROOT_DISK,"JPG BMP JPEG TIFF",10);
*/
Fsel_t * fsel_open( char* disk, char  *pExt, unsigned char property );


/**
* @brief   ���ڹر�һ���Ѿ��򿪵��ļ�ѡ����
*
* @param[in]  pFsel �ļ�ѡ�����ṹ��
* @return   �Ƿ�رճɹ�
* @retval   <0	 �ر��ļ�ѡ����ʧ��
* @retval   0	 �ر��ļ�ѡ�����ɹ�
* @author  ����
* @version�汾
* @date    ����
* @warning 
*/
int fsel_exit(Fsel_t *pFsel);


/**
* @brief   ���ڻ�ȡ��Ч�ļ�/Ŀ¼������
*
* @param[in]  pFsel �ļ�ѡ�����ṹ�壬����ļ�ѡ������Ҫ�õ�����ز���
* @param[in]  cmd ����
* -GET_TOTAL_FILE_NUM 	��ȡ����Ŀ¼�£�������Ŀ¼��������Ч�ļ���Ŀ������ֵ<0����ʾ��ȡʧ��
* -GET_FILE_NUM 			��ȡ����Ŀ¼�£���������Ŀ¼����Ч�ļ�����������ֵ<0����ʾ��ȡʧ�ܡ�
* -GET_DIR_NUM 			��ȡ����Ŀ¼��Ŀ¼������ ����ֵ<0����ʾ��ȡʧ�ܡ�
* -START_SCAN_DIR 		��ʼĿ¼ɨ�����񣬽����ļ���Ϣ������ֵ-2��ʾ��������ʧ�ܣ�����ֵ��ʾ�������ɹ���ֱ�ӷ����ļ�����   
* -DEL_SCAN_TASK 		ɾ��Ŀ¼ɨ������,����ֵ<0,��ʾɾ��ʧ��
* -GET_SCANED_FILE_NUM 	��ǰɨ�赽���ļ�����������ֵ<0����ʾɨ��ʧ��
* -GET_TASK_STATUS 		��ȡĿ¼ɨ�������״̬������-1��ʾ��ǰ���񲻴��ڣ�����-2��ʾ�����Ѿ�ֹͣ
* -REFRESH_FSEL			ˢ���ļ�ѡ���������Ƿ���0
* @return   ��cmd���
* @author  ����
* @version�汾
* @date    ����
* @warning 
*/
int fsel_getFileNum (Fsel_t *pFsel, unsigned char cmd);


/**
* @brief   ���ڻ�ȡ����Ŀ¼���ļ��������б�(��������Ŀ¼)
*
* @param[in]  file_name �ļ����ƵĽṹ������ָ�롣
* @param[in] fsel �ļ�ѡ�����ṹ��
* @param[in] startNO ��ȡ�б�Ŀ�ʼλ��,��1��ʼ����
* @param[in] num  ������ϣ����ȡ���ļ�������
* @return  ʵ�ʻ�ȡ�����ļ���������ʵ�ʴ��ڵı�������С��num��ֵ����˷���ֵΪʵ�ʵı��������
* @retval   0 ��ʾ��ѯ��Ŀ¼��StartNO����û����Ч�ļ�����Ҫ�������ÿ�ʼ��š�
* @retval   <0 ��ʾ��ȡʧ�ܡ�
* @author  ����
* @version�汾
* @date    ����
* @warning 
*        1.��һ��ʹ��ǰ�����ù���Ŀ¼
*        2.Ӧ����Ҫ���ݻ�ȡ���ļ���Ŀ��������file_name�ռ��С��
*        ���磺��ȡ5 ���ļ�������file_name�Ŀռ䶨��ӦΪ��filename_t file_name [5]��
*/
int fsel_getFileList (filename_t *file_name, Fsel_t *pFsel, int startNO,int num);


/**
* @brief   ���ڻ�ȡ����Ŀ¼��(������Ŀ¼)�ļ�������
*
* @param[in]  file_name �ļ����ƵĽṹ������ָ�롣
* @param[in] fsel �ļ�ѡ�����ṹ��
* @param[in] startNO ��ȡ�б�Ŀ�ʼλ��,��1��ʼ����
* @param[in] num  ������ϣ����ȡ���ļ�������
* @return  ʵ�ʻ�ȡ�����ļ���������ʵ�ʴ��ڵı�������С��num��ֵ����˷���ֵΪʵ�ʵı��������
* @retval   0 ��ʾ��ѯ��Ŀ¼��StartNO����û����Ч�ļ���
* @retval   <0 ��ʾ��ȡʧ�ܡ�
* @author  ����
* @version�汾
* @date    ����
* @warning 
*        1.��һ��ʹ��ǰ�����ù���Ŀ¼
*        2.Ӧ����Ҫ���ݻ�ȡ���ļ���Ŀ��������file_name�ռ��С��
*        ���磺��ȡ5 ���ļ�������file_name�Ŀռ䶨��ӦΪ��filename_t file_name [5]��
*/
int fsel_getFileListAll (filename_t *file_name, Fsel_t *pFsel, int startNO,int num);


/**
* @brief   ���ڻ�ȡ����Ŀ¼����Ŀ¼������,��������Ŀ¼�µ���Ŀ¼
*
* @param[in]  file_name �ļ����ƵĽṹ������ָ�롣
* @param[in] fsel �ļ�ѡ�����ṹ��
* @param[in] startNO ��ȡ�б�Ŀ�ʼλ��,��1��ʼ����
* @param[in] num  ������ϣ����ȡ���ļ�������
* @return  ʵ�ʻ�ȡ����Ŀ¼����������ʵ�ʴ��ڵı�������С��num��ֵ����˷���ֵΪʵ�ʵı��������
* @retval   0 ��ʾ��ѯ��Ŀ¼��StartNO����û����Ч�ļ���
* @retval   <0 ��ʾ��ȡʧ�ܡ�
* @author  ����
* @version�汾
* @date    ����
* @warning 
*        1.��һ��ʹ��ǰ�����ù���Ŀ¼
*        2.Ӧ����Ҫ���ݻ�ȡ���ļ���Ŀ��������file_name�ռ��С��
*        ���磺��ȡ5 ���ļ�������file_name�Ŀռ䶨��ӦΪ��filename_t file_name [5]��
*/
int fsel_getDirList (filename_t *file_name, Fsel_t *pFsel, int startNO,int num);


/**
* @brief   ���ڻ�ȡ����Ŀ¼����Ŀ¼���ļ�������
*
* @param[in]  file_name �ļ����ƵĽṹ������ָ�롣
* @param[in] fsel �ļ�ѡ�����ṹ��
* @param[in] startNO ��ȡ�б�Ŀ�ʼλ��,��1��ʼ����
* @param[in] num  ������ϣ����ȡ���ļ�������
* @return  ʵ�ʻ�ȡ���ĸ�������ʵ�ʴ��ڵı�������С��num��ֵ����˷���ֵΪʵ�ʵı��������
* @retval   0 ��ʾ��ѯ��Ŀ¼��û����Ч�ļ���
* @retval   <0 ��ʾ��ȡʧ�ܡ�
* @author  ����
* @version�汾
* @date    ����
* @warning 
*        1.��Ŀ¼�������ļ���
*        2.Ӧ����Ҫ���ݻ�ȡ���ļ���Ŀ��������file_name�ռ��С��
*        ���磺��ȡ����5���ļ�������file_name�Ŀռ䶨��ӦΪ��filename _t file_name [5]��
*/
int fsel_getList (filename_t *file_name, Fsel_t *pFsel, int startNO,int num );


/**
* @brief  ��ȡ�ļ���·����Ϣ��
*
* @param[in]  path ���·����Ϣbuffer������ǰ����÷����㹻�ռ�
* @param[in] fsel �ļ�ѡ�����ṹ��
* @param[in] start_NO ָ�����ļ����
* @return  �Ƿ��ȡ�ɹ�
* @retval   1 		��ʾ��ȡ�ɹ�
* @retval   <=0�� ��ʾ��ȡʧ��
* @author  ����
* @version�汾
* @date    ����
* @warning 
*/
int fsel_getFullpath (char * path, Fsel_t *pFsel, int start_NO);


/**
* @brief  �����ļ�ѡ�����Ĺ���Ŀ¼�������ǵ�ǰ����Ŀ¼�µ�·��
*
* @param[in] fsel �ļ�ѡ�����ṹ��
* @param[in] pDirName  ����·���������ĺϷ���Ŀ¼��(����·��)

* @return  �Ƿ����óɹ�
* @retval   >=0 ���ù���Ŀ¼�ɹ�
* @retval  <0���ù���Ŀ¼ʧ��
* @author  ����
* @version�汾
* @date    ����
* @warning 
* 	�����ĺϷ���Ŀ¼��Ϊ��[device][DirPathList]DirectoryName���ֱ�Ϊ�豸����Ŀ¼·����Ŀ¼��
* @code 	fsel_serWorkDir(pFsel,"/mnt/udisk/test");
*/
int fsel_setWorkDir (Fsel_t *pFsel, char *pDirName);


/**
* @brief  �����ļ�ѡ�����Ĺ�������
*
* @param[in] fsel �ļ�ѡ�����ṹ��
* @param[in] Ext  �ļ�ѡ�����Ĺ������͡��ļ�ѡ����ͨ���趨�ĺ�׺�������ļ�

* @return  �Ƿ����óɹ�
* @retval   TRUE ���óɹ�
* @retval  FALSE ����ʧ��
* @author  ����
* @version�汾
* @date    ����
* @warning 
*         1��ÿ����׺������Ϊ3���Կո�Ϊ��������
*               ����const char ExtMusic[ ] = "MP3 WMA ASF WMV WAV ACT OGG M4A AA  ";
*         2������׺�����Ȳ���3 ʱ���Կո��㣻
*         3����׺��Ϊ3 ���ո�ʱ���ļ�ѡ��������ʾ�����ļ��� 
*/
bool fsel_setExt (Fsel_t *pFsel, char* Ext);


/**
* @brief  ���˵���һ��Ŀ¼������������Ϊ��ǰ����Ŀ¼�������ļ�ѡ������
			(��Ŀ¼Ϊfsel_setRootDir()���õ�Ŀ¼)
*
* @param[in] fsel �ļ�ѡ�����ṹ��
* @param[in] index  ��һ��Ŀ¼�����

* @return  �Ƿ�ɹ�
* @retval   >=0 �ɹ�
* @retval  <0 ʧ��
* @author  ����
* @version�汾
* @date    ����
* @warning 
*/
int fsel_setparent(Fsel_t *pFsel,int *index);


/**
* @brief   �ӵ�ǰĿ¼���뵽��һ��Ŀ¼����������Ϊ����Ŀ¼���Ҹ����ļ�ѡ����
*
* @param[in] fsel �ļ�ѡ�����ṹ��
* @param[in] index  ��Ҫ�����Ŀ¼�����
* @param[in] pDirName ��Ҫ�����Ŀ¼��
* @return  �Ƿ�ɹ�
* @retval   >=0 �ɹ�
* @retval  <0 ʧ��
* @author  ����
* @version�汾
* @date    ����
* @warning 
*/
int fsel_CD (Fsel_t *pFsel,char *pDirName,int index);


/**
* @brief  ���� �������ļ���Ϣ��ɾ��ָ����ŵ��ļ�
*
* @param[in] fsel �ļ�ѡ�����ṹ��
* @param[in] index  ��Ҫɾ�����ļ����

* @return  �Ƿ�ɹ�
* @retval   0 ɾ���ɹ�
* @retval  <0 ɾ��ʧ��
* @author  ����
* @version�汾
* @date    ����
* @warning 
*/
int  fsel_del_file(Fsel_t *pFsel,int index);


/**
* @brief  �����ļ�ѡ�����ĸ�Ŀ¼
*
* @param[in] fsel �ļ�ѡ�����ṹ��
* @param[in] pDirName  ��Ŀ¼����·�������ĺϷ���Ŀ¼��

* @return  �Ƿ�ɹ�
* @retval   TRUE 	�ɹ�
* @retval  FALSE	ʧ��
* @author  ����
* @version�汾
* @date    ����
* @warning 
*˵���������ĺϷ���Ŀ¼��Ϊ��[device][DirPathList]DirectoryName���ֱ�Ϊ�豸����Ŀ¼·����Ŀ¼��
*/
bool fsel_setRootDir (Fsel_t *pFsel,char *pDirName);


/**
* @brief  ��ǰĿ¼��ָ������ļ����°�������Ч�ļ�����
*
* @param[in] fsel �ļ�ѡ�����ṹ��
* @param[in] index  �ļ������

* @return  �ļ�����

* @author  ����
* @version�汾
* @date    ����
* @warning 
*
*/
int  fsel_dir_total_fileNum(Fsel_t *pFsel,int index);


/**
* @brief  �򿪵�ǰָ��ȫ·�����ļ�
*  ��̨�ļ���Ϣ��û�н���������ʱ�����ô˺���ͬ���򿪵��ļ�
* @param[in] fsel �ļ�ѡ�����ṹ��
* @param[in] pDirName  �ļ�ȫ·��
* - NULL ͬ���ļ�
* @param[in] index  ����򿪵��ļ����
* @return  �Ƿ�ɹ�
* @retval   1 �ɹ�
* @retval   0 ����ͬ��
* @retval  -1 ʧ��
* @author  ����
* @version�汾
* @date    ����
* @warning 
*/
int  fsel_OpenFile (Fsel_t *pFsel,char *pDirName,int *index);

#if 0
/**
* @brief  �����ļ���Ϣ��
*
* @param[in] fsel �ļ�ѡ�����ṹ��
* @param[in] addr  buffer��ַ

* @return  �Ƿ�ɹ�

* @retval   NULL ʧ��

* @author  ����
* @version�汾
* @date    ����
* @warning 
*
*/
int fsel_search_dir(Fsel_t *pFsel,char * path,dir_info_t **addr_array,char ** ext_array,int num);


/**
* @brief  ���û��߲�ѯ�����ļ���Ϣ��buffer��ַ��
*
* @param[in] fsel �ļ�ѡ�����ṹ��
* @param[in] addr  buffer��ַ

* @return  �Ƿ�ɹ�
* @retval   NULL ʧ��

* @author  ����
* @version�汾
* @date    ����
* @warning 
*	��ʱû�ã��������ܻ�ȡ��
*/
dir_info_t *  fsel_ListBuffer (Fsel_t *pFsel, dir_info_t ** addr,char ** ext,unsigned int num,unsigned char op);

#endif
/** 
* @brief   ����һ���ַ������Ƿ������ָ������չ��
*
* @param[in] exts ��չ���ַ���
* @param[in] ext  Ҫ���ҵ��ַ���

* @return  �Ƿ�ɹ�

* @retval   1 	�ɹ�
		  0	ʧ�� 
* @author  ����
* @version�汾
* @date    ����
* @warning 
*	mnavi �ڲ�����
*/
int fsel_find_ext(char * exts,char *ext );


/**
* @brief   ���ļ����л�ȡ�ļ��ĺ�׺��
*
* @param[in] name �ļ���
* @param[in] ext  ������չ����buf

* @return  �Ƿ�ɹ�

* @retval   1 �ɹ�

* @author  ����
* @version�汾
* @date    ����
* @warning
*	mnavi �ڲ�����
*/
unsigned char fsel_get_ext(char *name,char*ext);


/**
* @brief   ��Ŀ¼��������Ϊ��ǰ����·��
*
* @param[in] name ���Ŀ¼��
* @param[in] pdir 	Ŀ¼���

* @return  �Ƿ�ɹ�
* @DIR*   �ǿ��� �ɹ�

* @author  ����
* @version�汾
* @date    ����
* @warning 
*	mnavi �ڲ�����
*/
DIR *xcddir(DIR *pdir,const char *const name);


/**mnavi �ڲ�����*/
int  fsel_getFileListSub (filename_t *file_name, Fsel_t *pFsel,	int  startNO ,	int  num);


/**
* @brief  ���˵���һ��Ŀ¼
*
* @param[in] fsel �ļ�ѡ�����ṹ��

* @return  �Ƿ�ɹ�
* @retval   0 ���سɹ�
* @retval  -1 ����ʧ��
* @author  ����
* @version�汾
* @date    ����
* @warning 	
*	��fsel_setparent����;
*	mnavi �ڲ�����
*/
int fsel_back_parent(Fsel_t *pFsel);


/**
* @brief   �ӵ�ǰĿ¼���뵽��һ��Ŀ¼
*
* @param[in] fsel �ļ�ѡ�����ṹ��
* @param[in] name ��Ҫ�����Ŀ¼��
* @return  �Ƿ�ɹ�
* @retval   0 	�ɹ�
* @retval  -1	ʧ��
* @author  ����
* @version�汾
* @date    ����
* @warning 
*	��fsel_CD��ͬ���ú���ֻ�Ǽ򵥵ؽ��뵽��һ��Ŀ¼;	
*	mnavi �ڲ�����
*/
int fsel_docd(Fsel_t *pFsel,char* name);

#endif
/**
 * @}
 */
#endif

