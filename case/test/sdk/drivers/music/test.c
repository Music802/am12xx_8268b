#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
typedef  enum{
		NORMAL_PLAY,						//��������					
		TAG_PLAY,						//TAG����
		SEEK_PLAY						//�ϵ�����/������seek����
} play_mode_t;

typedef struct{
	play_mode_t mode;					//��ǰ�Ĳ���ģʽ
	unsigned int param;					//��ͬ����ģʽ�µ��������
}play_param_t;	
main(){

	void *handle;
	play_param_t play_param;
printf("1\n");
	handle=audioDecOpen(0);
printf("2\n");
	audioDecCmd(handle, 0x16,"a.mp3");//setfile
printf("3\n");
	play_param.mode=NORMAL_PLAY;
	play_param.param=0;
	audioDecCmd(handle, 0x1,(unsigned int)&play_param);
	
printf("4\n");
while(1);



}
