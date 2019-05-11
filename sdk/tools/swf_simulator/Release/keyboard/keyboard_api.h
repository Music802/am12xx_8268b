typedef struct keyboard_rect{
	int width; //the width of keyborad
	int height; //the height of keyboard
}keyboard_rect;
//����keyboard��Ҫ����Դ
struct keyboard_rect *keyboard_open(void);
//�ͷ�keyboardʹ�õ���Դ 
int keyboard_close(void);
//ʹ��keyboard��������
int keyboard_enable(void);
//�ر�keyboard��������
int keyboard_disable(void);
enum KEYBOARD_FORMAT{
	K_RGB565=1,
	K_YUV422=2,
	K_RGB888=3
};
//����keyboard����
/*
(x,y):keyboard�������
*/
int keyboard_config_position(int x,int y);
/*
frame:���buffer
stride:���ڿ��
height:���ڸ߶�
*/
int keyboard_config_frame(char *frame,int stride,int height);
/*
format:ͼ���ʽ
alpha:0������ʾ��255:ȫɫ��ʾ
osd:	1:use osd 0:don't usb osd
scale:	0~100, 50 means displaying 50%,100 means displaying 100% 
*/
int keyboard_config_display_mode(enum KEYBOARD_FORMAT format,int alpha,int osd,int scale);
void keyboard_updata(void);
//int keyboard_config(char *frame,int x,int y,int stride,int width,int height,enum KEYBOARD_FORMAT format,int alpha);
enum KEYBOARD_EVENT{
	K_LBUTTONDOWN=1,
	K_MOVE=2,
	K_LBUTTONUP=3
};
//������궯����������Ч����ʱ������Ӧ����asciiֵ�������keyboard״̬
int keyboard_event_to_keyvalue(enum KEYBOARD_EVENT event,int x,int y);
int keyboard_is_drag(int x,int y);
int keyboard_config_position(int x,int y);

int keyboard_config_display_mode(enum KEYBOARD_FORMAT format,int alpha,int osd,int scale);

int keyboard_config_lang(int english);

void keyboard_Enable_Chinese(int flag);
int is_valid_key_for_freepy(int key);