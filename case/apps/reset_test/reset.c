/*  
����һ���ػ����̣�����ϵͳ�������еĽ���   
*/    
#include <unistd.h>    
#include <signal.h>    
#include <sys/types.h>    
#include <sys/stat.h>    
#include <stdio.h>    
#include <stdlib.h>    
#include <sys/resource.h> 

//����һ���ػ�����    
void init()    
{    
    int pid;    
    int i;           
    printf( "---------------%s %s %d \n",__FILE__,__FUNCTION__,__LINE__);
    if(pid = fork())    
        exit(0);                          //�����̣��˳�    
    
    else if(pid < 0)                          //���ٽ���ʧ�ܣ��˳����ر����н���    
        exit(1);    
    /* �ӽ��̼���ִ�� */    
    setsid();                               //�����µĻỰ�飬�ӽ��̳�Ϊ�鳤����������ն˷���    
    
    /* ��ֹ�ӽ��̣��鳤����ȡ�����ն� */    
    if(pid = fork())    
        exit(0);                       //�����̣��˳�     
    else if(pid < 0)     
           exit(1);                        //���ٽ���ʧ�ܣ��˳����ر����н���    
  
/* �ڶ��ӽ��̼���ִ�� , �ڶ��ӽ��̲����ǻ�Ự���鳤*/    
    
    /* �رմ򿪵��ļ�������*/ 
	//int des=getdtablesize();
      //  for (i = 0; i <des; i++)    
           //  close(i);         
    chdir("/tmp"); // �л�����Ŀ¼     
    umask(0);       // �����ļ���������     
    return;    
}    
    
int main()    
{    
  
    signal(SIGCHLD, SIG_IGN); // �����ӽ��̽����źţ���ֹ���ֽ�ʬ����      
    init();           //��ʼ���ػ����̣����Ǵ���һ���ػ�����
 //    printf( "---------------%s %s %d \n",__FILE__,__FUNCTION__,__LINE__);
    while(1)//ÿ��һ������test.log��������״̬ 
    { 
        sleep(3);//˯��һ���� 
        if(access("/tmp/reset.txt",F_OK) == 0)
        {
		printf("it is ready to reset\n");
		system("killall -9 wireUI.app manager.app pthsystem.app");
		sleep(1);
		//system("ps");
		//printf("fuser\n");
		//system("fuser -m /mnt/vram");
 		system("umount  /mnt/vram");
		system("mkfs.vfat	/dev/partitions/vram");
		system("reboot");

        }
    } 

    return 0;    
} 
