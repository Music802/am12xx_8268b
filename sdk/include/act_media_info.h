
/* ý��⹫��ͷ�ļ� */

#ifndef _ACT_MEDIA_INFO_H_
#define _ACT_MEDIA_INFO_H_

/* ͼ����Ϣ��ȡAPI����ֵ���� */
typedef enum
{
	M_INFO_OK                 = 0,  //OK
	M_INFO_PRAM_ERR           = -1, //����API�ӿں�����������
	M_INFO_CALL_INTERNAL_ERR  = -2, //�����ڲ�����ʧ��
	M_INFO_NOT_SUPPORT        = -3, //��֧�ֵ�ý������
	M_NO_USR_TAG              = 1   //û���û�tag
	
}MEDIA_INFO_RET;

/* CMD ���� */
typedef enum
{
	M_PHOTO                   = 0,   //photo info
	M_MUSIC                   = 1,   //music info
	M_VIDEO                   = 2,   //video info
	M_PHOTO_USR_TAG           = 3,   //read photo info and usr tag(s)
	M_USR_TAG_R               = 4,   //read usr tag(s) only
	M_USR_TAG_W               = 5    //write usr tag(s)
	
}MEDIA_INFO_CMD;

/* ����media info API������ṹ */
typedef struct tagMInfoInput
{
	void *file_handle;
	long (*p_read)(void *, unsigned char *, unsigned long);
	long (*p_write)(void *, unsigned char *, unsigned long);
    long (*p_tell)(void *);
    long (*p_seek_set)(void *, unsigned long);
    long (*p_seek_cur)(void *, long);
	long (*p_seek_end)(void *, long);           
	void *(*p_malloc)(int);        
	void (*p_free)(void *);
	
}M_INFO_INPUT_S;

/* ԭʼʱ��(����ʱ��) */
typedef struct tagDateTimeOriginal
{
	unsigned short year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
}DATE_TIME_ORIGINAL_S;

/* ͼ����Ϣ�ṹ */
typedef struct tagImgInfo
{
	DATE_TIME_ORIGINAL_S iie_date_time_original;      //����ʱ��
	unsigned short main_img_width;                    //��ͼ���(������ܻ���������ͼ���)
	unsigned short main_img_height;                   //��ͼ�߶�(������ܻ���������ͼ�߶�)
	/* ϣ����ȡͼ�����Щ��Ϣ��������enableע��Ϊ1�������� */
	int date_time_org_enable;
	int img_wid_enable;
	int img_hei_enable;

}IMG_INFO_S;

/* tags added by usrs */
typedef struct{
	char magic[9];         // always "ACT-MICRO"
	char version;
	char rotation;
	char effect;
	unsigned int tag;
	char reserved[2];      // for 16bytes allignment
}PHOTO_DB_T;

typedef struct tagImgAndTagInfo
{
	IMG_INFO_S img_info;
	PHOTO_DB_T usr_tag;
	
}IMG_INFO_USR_TAG_S;

/* ͼ����Ϣ��ȡAPI ÿһ���ļ�����һ�� */
MEDIA_INFO_RET GetMediaInfo(M_INFO_INPUT_S *media_input, MEDIA_INFO_CMD cmd, void *info);

#endif

