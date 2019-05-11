/**
* i2c���� ���ر�����:
* read()/write()ֻ���һ����Ϣ�Ĵ��ͣ�
* һ������ɶ�����Ϣ�Ĵ�����Ҫioctl(fd,I2C_RDWR..)ʵ�֣�
* �������ĵ�ʵ�ֵ���:ͨ��ioctlʵ�ֵ��ζ�����Ϣ�Ĵ���
*
* ע��:��Ҫ��make menuconfig->device drivers->[*]i2c support->[*]i2c device interfae
**/
#include <stdio.h>
#include <linux/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
 
#define I2C_RETRIES      0x0701
#define I2C_TIMEOUT     0x0702
#define I2C_RDWR          0x0707
#define MSG_LEN 1

struct i2c_msg
{
   __u16 addr; 		/* �ӵ�ַ */
   __u16 flags;
   #define I2C_M_RD        0x01
   __u16 len;			/* msg length	*/
   __u8 *buf; 			/* ��Ϣ����ָ�� */
 };

 struct i2c_rdwr_ioctl_data
 {
  struct i2c_msg *msgs; /* i2c_msg[]ָ�� */
  int nmsgs; 			/* i2c_msg���� */
 };
 
/**
* argv[1]: i2c���豸�ţ���0��ʼ������
* argv[2]:device_address,Ϊi2c���豸��ַ
* argv[3]:register_address,Ϊ�豸�ļĴ�����ַ
* argv[4]:��Ҫ���͵���Ϣ������
* eg:   I2C_IOCTL.app /dev/i2c-0 71 0 1 ��ʾ��ȡ�豸i2c-0��ƫ��0����ֵ,��ȻҪ֪�����豸�ĵ�ַΪ0x71
**/
 int main(int argc, char **argv)
{
  struct i2c_rdwr_ioctl_data work_queue;
  unsigned int idx;
  unsigned int fd,num;
  unsigned short device_address,register_address;
   int ret;
  #define BUFF_SIZE    32
  char tmp[BUFF_SIZE] = {0};
  
   if (argc < 4)
   {
    	printf("Usage:\n%s /dev/i2c-x start_addr\n", argv[0]);
     	return 0;
   }
 
   fd = open(argv[1], O_RDWR);
 
   if (!fd)
   {
    	printf("Error on opening the device file\n");
     	return 0;
   }
   sscanf(argv[2], "%x", &device_address);
   sscanf(argv[3], "%x", &register_address);
   sscanf(argv[4], "%d", &num); /* ��Ϣ���� */
   
   work_queue.nmsgs = num+1;
   work_queue.msgs = (struct i2c_msg*)malloc(work_queue.nmsgs *sizeof(struct i2c_msg));
   if (!work_queue.msgs)
   {
    	printf("Memory alloc error\n");
    	close(fd);
     	return 0;
   }

   //send the read register address first!   
  (work_queue.msgs[0]).flags  = 0;
  (work_queue.msgs[0]).len = 1;
  (work_queue.msgs[0]).addr = device_address;
  (work_queue.msgs[0]).buf = &register_address;

//then make the read buf  point to tmp[]
  for (idx = 1; idx < work_queue.nmsgs; ++idx)
   {
   	(work_queue.msgs[idx]).flags |= I2C_M_RD;
    	(work_queue.msgs[idx]).len = 1;
    	(work_queue.msgs[idx]).addr = device_address;
     	(work_queue.msgs[idx]).buf = &tmp[idx];
  }
 
   ioctl(fd, I2C_TIMEOUT, 2); /* ���ó�ʱ */
   ioctl(fd, I2C_RETRIES, 1); /* �������Դ��� */

   ret = ioctl(fd, I2C_RDWR, (unsigned long) &work_queue);
  for (idx = 1; idx < work_queue.nmsgs; ++idx)
   {
   	printf("read buf[%d] = 0x%x\n",idx,tmp[idx]);
  }
   if (ret < 0)
   {
     printf("Error during I2C_RDWR ioctl with error code: %d\n", ret);
   }
 
   close(fd);
   free( work_queue.msgs );
  return 0 ;
 }



