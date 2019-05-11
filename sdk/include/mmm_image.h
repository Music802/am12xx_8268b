
/* ͼƬ�м������ͷ�ļ� */

#ifndef __MMM_IMAGE_H__
#define __MMM_IMAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* ������� */
#define IMG_ROT_NONE                0
#define IMG_ROT_RIGHT_90			1
#define IMG_ROT_LEFT_90			    2
#define IMG_ROT_HOR_FLIP			3
#define IMG_ROT_VER_FLIP			4
#define IMG_ROT_180					5
#define IMG_ZOOM_IN					6
#define IMG_ZOOM_OUT			    7
#define IMG_MOVE_UP					8
#define IMG_MOVE_DOWN				9
#define IMG_MOVE_LEFT				10
#define IMG_MOVE_RIGHT				11
#define IMG_ZOOM_RESET           	12
#define IMG_SET_SCALE_RATE          13
#define IMG_DECODE              	14
#define IMG_SET_ROTATION         	15 //ֱ��ָ��������ԭͼ����ת����, ����������PP_ROT_LEFT_90�ȼ��������ϴλ�����...
#define IMG_GET_PHOTO_INFO          16
#define IMG_DEC_STOP                17

/* ͼƬ��ʽ */
#define IMG_FMT_YCBCR_4_2_0_SEMIPLANAR              0		//IO
#define IMG_FMT_YCBCR_4_2_2_INTERLEAVED             1		//IO

/* ������ʾģʽ */
typedef enum tagImgDecMode
{
	DEC_MODE_ORG          = 0,  	    //ԭʼ�ߴ�, ���ܻ��ڽ������ڲ�����ԭͼ�������ת��
	DEC_MODE_LBOX		  = 1,   	    //letter_box, ���ܻ��ڽ������ڲ�����ԭͼ�������ת�� 
	DEC_MODE_EXP		  = 2,      	//pan_and_scan, ���ܻ��ڽ������ڲ�����ԭͼ�������ת��  
	DEC_MODE_FULL		  = 3,      	//ȫ��ģʽ, ���ܻ��ڽ������ڲ�����ԭͼ�������ת�� 
	DEC_MODE_ACTUAL_SIZE  = 4,          //ԭʼ�ߴ�,ǿ��ִ��, �����ڽ������ڲ�����ԭͼ�������ת��
	DEC_MODE_LETTER_BOX   = 5,    		//letter_box, ǿ��ִ��, �����ڽ������ڲ�����ԭͼ�������ת��
	DEC_MODE_FULL_SCREEN  = 6,   		//ȫ��ģʽ, ǿ��ִ��, �����ڽ������ڲ�����ԭͼ�������ת��
	DEC_MODE_PAN_AND_SCAN = 7           //pan_and_scan, ǿ��ִ��, �����ڽ������ڲ�����ԭͼ�������ת��
	
}IMG_DEC_MODE;

typedef struct tagLinearBuf
{
	unsigned char *buf;
	unsigned long bus_addr;
	unsigned long size;
	
}IMG_LINEAR_BUFFER;

/* �ϲ㴫�����������ݽṹ */
typedef struct tagImgDecodeParam
{
    void *handle;
	char file_name[1024];
	int (*img_fread)(void*, void*, int);
	int (*img_fseek_set)(void*, int);
	int (*img_fseek_cur)(void*, int);
	int (*img_fseek_end)(void*, int);
	int (*img_ftell)(void*);
	unsigned long pic_ID;          //ͼ���ID��
	IMG_LINEAR_BUFFER out_buf_y;   //���buffer: y����
	IMG_LINEAR_BUFFER out_buf_uv;  //���buffer: uv����
	IMG_LINEAR_BUFFER tmp_buffer;  //�����ʹ�õ��м���ʱbuffer
	unsigned long cmd;             //��������
	int is_high_prio_cmd;          //�Ƿ��Ǹ����ȼ�����,1=yes,0=no
	unsigned int formate;          //����ͼ���ʽ
	unsigned long output_width;
	unsigned long output_height;   //����������
	unsigned long lcd_width;       //��Ļ���
	unsigned long lcd_height;
	IMG_DEC_MODE pic_dec_mode;     //��ʾģʽ
	signed long brightness;        //��ʾЧ��
	signed long saturation;
	signed long contrast;
	unsigned long num;             //���ؿ�߱ȼ���ʹ��
	unsigned long den;
	int pip_enable;                //�Ƿ���"���л�"ģʽ
	long pip_start_x;              //���л���ʼx����(�����Ļ���Ͻ�)
	long pip_start_y;              //���л���ʼy����(�����Ļ���Ͻ�)
	unsigned long pip_frame_width; //���л�frame buffer width
	unsigned long pip_frame_height;//���л�frame buffer height
	int usr_rotated;               //�û�ָ����ת�Ƕ�,��Գ�ʼ״̬�������������һ��
	int thumb_rot;                 //�û�ָ������ͼ��ת����(�������ͼ��ͬ)
	int auto_rotation_by_exif_en;  //�Ƿ�������exif��Ϣ�Զ���ת
	unsigned long usr_zoom_rate;   //�û�ָ���Ŵ���,�����ԭͼ
	int thumb_browse_mode;         //�Ƿ�������ͼ���ģʽ
	unsigned long hw_rst_enable;   //whether to reset hantro
	unsigned long mem_use_trace;   //whether to trace mem use in photo midware
	unsigned long reg_trace;       //whether to trace hantro registers
	int can_be_stopped;            //��һ��������ʱ����������Ƿ���Ա����
	
}IMG_DECODE_PARAM_S;

typedef struct tagImgDate
{
	unsigned int year;
	unsigned int month;
	unsigned int day;
	
}IMG_DATE;

/* �������������ϲ����Ϣ */
typedef struct tagImgDecodeInfo
{
	int result;                    //������ǰ�����õ��ķ���ֵ
	int img_ready;                 //����ɹ����
	unsigned long pic_ID;          //ͼ���ID��
	long img_start_x;              //���������ʵ��ͼ�񲿷������Ļ���Ͻǵ�x����
    long img_start_y;              //���������ʵ��ͼ�񲿷������Ļ���Ͻǵ�y����
    unsigned long img_actual_wid;  //���������ʵ��ͼ�񲿷ֵĿ��
    unsigned long img_actual_hei;  //���������ʵ��ͼ�񲿷ֵĸ߶�
    unsigned long cur_scale_rate;  //����100�ǵ�ǰ���ű���,ע���������ԭͼ��!
    IMG_DATE date;                 //����ʱ����Ϣ
    unsigned int src_width;        //ԭʼͼƬ���
	unsigned int src_height;       //ԭʼͼƬ�߶�
	int ap_stop;
    
}IMG_DECODE_INFO_S;

/* ��Ϣ���� */
#define DECODE_PARAM 0
#define DECODE_INFO  1

/* �ر�ͼƬ������API */
int image_dec_close(void *img_browser);

/* ��ͼƬ������API */
void *image_dec_open(void *param);

/* ����Ϣ���з�����ϢAPI */
int image_dec_send_msg(void *img_browser, void *msg, int size, int type);

/* ����Ϣ���еõ���ϢAPI,��pend */
int image_dec_get_msg(void *img_browser, void *msg, int size, int type);

/* stop */
int image_dec_stop(void *img_browser);

#ifdef __cplusplus
}
#endif

#endif //__MMM_IMAGE_H__