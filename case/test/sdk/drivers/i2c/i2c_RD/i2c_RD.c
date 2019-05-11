/**
* i2c���� ���ر�����:
* read()/write()ֻ���һ����Ϣ�Ĵ��ͣ�
* һ������ɶ�����Ϣ�Ĵ�����Ҫioctl(fd,I2C_RDWR..)ʵ�֣�
* �������ĵ�ʵ�ֵ���:ͨ��read()/write()ʵ�ֵ���һ����Ϣ�Ĵ���
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
#define I2C_SLAVE_FORCE      	   0x0706
 
/**
* argv[1]: i2c���豸�ţ���0��ʼ����:/dev/i2c-0��
* argv[2]:device_address,Ϊi2c���豸��ַ
* argv[3]:register_address,Ϊ�豸�ļĴ�����ַ
* argv[4]:��Ҫ���͵���Ϣ������
* eg:   I2C_RD.app /dev/i2c-0 71 0 1 ��ʾ��ȡ�豸i2c-0��ƫ��0����ֵ,��ȻҪ֪�����豸�ĵ�ַΪ0x71
**/
int main(int argc, char **argv)
{
	unsigned int fd;
   	unsigned short device_address,register_address;
   	unsigned short size;
  	unsigned short idx;
   	#define BUFF_SIZE    32
   	char buf[BUFF_SIZE];
	char cswap;
	int ret;
	
	union
	{
		unsigned short addr;
		char bytes[2];
	} tmp;
	if (argc < 4)
	{
		printf("Use:\n%s /dev/i2c-x mem_addr size\n", argv[0]);
		return 0;
	}
   	sscanf(argv[2], "%x", &device_address);
   	sscanf(argv[3], "%x", &register_address);
	sscanf(argv[4], "%d", &size);

	if (size > BUFF_SIZE)
	 	size = BUFF_SIZE;
	fd = open(argv[1], O_RDWR);

   	if (!fd)
 	 {
   	 	 printf("Error on opening the device file\n");
     		return 0;
  	 }
 
	  ioctl(fd, I2C_SLAVE_FORCE, device_address); /* ���ô��豸��ַ */
	  ioctl(fd, I2C_TIMEOUT, 1); /* ���ó�ʱ */
	  ioctl(fd, I2C_RETRIES, 1); /* �������Դ��� */

	  for (idx = 0; idx < size; ++idx, ++register_address)
	   {
	    	tmp.addr = register_address;
	    	//cswap = tmp.bytes[0];
	    	//tmp.bytes[0] = tmp.bytes[1];
	    	//tmp.bytes[1] = cswap;
	     	write(fd, &tmp.addr, 1); //send the register addr
		ret = read(fd, &buf[idx], 1);//read datas from the register
		if (ret < 0)
		 {
		     printf("Error during I2C_RDWR ioctl with error code: %d\n", ret);
		     return 0;
		 } 
		printf("buf[%d]=0x%x\n",idx, buf[idx]);

	   }
   	close(fd);
   	return 0;
 }


