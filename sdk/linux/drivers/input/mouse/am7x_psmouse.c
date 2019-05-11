#include <linux/init.h>
#include<linux/module.h>
#include <linux/kernel.h>   
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/timer.h>
#include <sys_msg.h>
#include "actions_regs.h"
#include "actions_io.h"
#include "sys_cfg.h"


struct input_dev *am_mouse_dev;
struct timer_list mouse_timer;
struct timer_list checkerr_timer;
struct timer_list mouse_timeout;
static INT32U gGPIO_Multi5;
static INT32U gGPIO_Multi6;
static int send_message_state;
static char DownLeftFlag;
static char DownRightFlag;
static char DownMiddleFlag;
static int mousex_data;
static int mousey_data;
static int mousez_data;
#define MOUSE_CLK_IO   36 //GPIO 36
#define MOUSE_DATA_IO 40 //GPIO 40	
#define defaultstate		0x00
#define LeftBtnDown		0x01
#define LeftBtnUp			0x02
#define RightBtnDown		0x03
#define RightBtnUp		0x04
#define MiddleDown		0x05
#define MiddleUp   		0x06
#define move_z			0x08
#define move_x_y		0x09
#define Receive_cmd_data      0x10
#define Receive_normal_data	0x20
#define Send_cmd			0x30
#define Other_state			0x40
#define ERR_CHECK_PERIOD	2	//�����ݴ���ʱʱ��
#define receive_maxtime     	3	//8ms����
#define init_mouse_times        15
#define end_init_mouse      	 init_mouse_times+1
#define resetmouse_maxtime  10 //ʵ�ʸ�λΪresetmouse_maxtime
#define con_cmd_times 		20//������⵽����receive_cmd_state״̬N��
#define con_send_times 		30//������⵽����send_cmd_state״̬N��
#define con_other_times 		30//������⵽����other_state״̬N��
#define con_clinterrupt_times 	30//������⵽����close interrutp״̬N��
#define con_interrupt_switch   5//colse interrupt
#define BUFNUMMAX			4
#define other_interrupt         0xff
#define interrupt_touchpad  0xf0
#define interrupt_mouse  	   0xf1
#define MOUSE_MOVE_FREQUENCE		1
static int MouseReceiveBuffer[BUFNUMMAX];//���mouse������
static int xpos_buf[MOUSE_MOVE_FREQUENCE];
static int ypos_buf[MOUSE_MOVE_FREQUENCE];

const int BitTab[]={0x0001,0x0002,0x0004,0x0008,0x0010,0x0020,0x0040,0x0080,
                     0x0100,0x0200,0x0400,0x0800,0x1000,0x2000,0x4000,0x8000};

static struct interrupt{
	unsigned char ext_irq_source;
}interrupt_state;

static struct ps2mouse {
	int send_data;
	int receive_data;
	int init_mouse_index;		
	int each_restart_times;
	unsigned char state;
	unsigned char open_timer0_flag;
	unsigned char open_timer1_flag;
	unsigned char first_init_mouse;
	unsigned char receive_end;//����һ�ֽ���ϱ�ʾλ
	unsigned char restart_mouse_flag;
	unsigned char send_bit_index;
	unsigned char receive_bit_index;
	unsigned char receive_byte_index;
	unsigned char receive_byte_index2;
	unsigned char receive_data_timeout;
	unsigned char interrupt_state_flag;	//0--interrupt close   1--interrupt open
	unsigned char interrupt_switch;		// 1--�����������0--����������
}ps2mouse_state;
/**************************
*@��������
*@
**************************/
#define act_writel(val,reg)  (*(volatile unsigned int *)(reg) = (val))                
#define act_readl(port)  (*(volatile unsigned int *)(port))   
void am_set_gpio(loff_t num,INT8U value);
INT8U am_get_gpio(loff_t num);
static void mouse_clearPending(void);
static int  enableIRQ(void );
static int  disableIRQ(void );
static void mouserest(void);
static irqreturn_t mouse_interrupt(void);
static int init_mouse_timer(void);
static int init_mouse_io(void);
static int TestMouseData(int data);
static int SendDatatoDevice( int data);
static int receive_cmd_data(void);
static int receive_normal_data(void);
static int MouseTimerCheck(int *PacketReg);
static void sendmessage(void);
static int  check_whick_interrupt(void);

/*************************
*@���ж����ٺ�
*@check which interrupt 
**************************/
static int  check_whick_interrupt(void)
{	
	interrupt_state.ext_irq_source=other_interrupt;
	if(act_readl(INTC_EXTCTL01) & 0x00010000)
	{
		interrupt_state.ext_irq_source=interrupt_mouse;//External Interrupt 1
		return 1;
	}
	if(act_readl(INTC_EXTCTL01) & 0x01)
	{
		interrupt_state.ext_irq_source=interrupt_touchpad;
		return 0;//External Interrupt 0
	}
	return -1;//err interrupt 
}
/**************************
*@���ж����ٺ�
*@�����־λ
**************************/
static void mouse_clearPending(void)
{
	if(act_readl(INTC_EXTCTL01) & 0x00010000)
	{
		act_writel(act_readl(INTC_EXTCTL01)|(1<<16), INTC_EXTCTL01);
	}
}
/**************************
*@ʹ���ⲿ�ж�1
*@�½��ش���
**************************/
static int  enableIRQ(void )
{
#if CONFIG_AM_CHIP_ID == 1213
	// to be done
#else
	//Set Interrupt source
	gGPIO_Multi5=act_readl(GPIO_MFCTL5);	
	act_writel((gGPIO_Multi5&(~(7<<28)))|(5<<28),GPIO_MFCTL5); //set IO36 for MouseCLK (EXT_INT1)
	//�½��ش���
	//12-3
	//act_writel(0x07000000, INTC_EXTCTL01);
	act_writel(((act_readl(INTC_EXTCTL01)&(~(7<<24)))|(7<<24)), INTC_EXTCTL01);//�½��ش���
	//�͵�ƽ����
	//act_writel(0x03000000, INTC_EXTCTL01);
	//�����ش���
	//act_writel(0x05000000, INTC_EXTCTL01);	
	mouse_clearPending();
	ps2mouse_state.interrupt_state_flag=1;
#endif
	return 0;
}
/**************************
*@�ر��ж�
*@
**************************/
static int  disableIRQ(void )
{
#if CONFIG_AM_CHIP_ID == 1213
	// to be done
#else
	act_writel(act_readl(INTC_EXTCTL01)&(~(1<<24)), INTC_EXTCTL01);	//���ж�
	gGPIO_Multi5=act_readl(GPIO_MFCTL5);	
	act_writel((gGPIO_Multi5&(~(7<<28)))|(4<<28),GPIO_MFCTL5); //set IO36 for MouseCLK (IO)
	ps2mouse_state.interrupt_state_flag=0;
#endif
	return 0;
}
/**************************
*@��ʼ�����IO��
*@MOUSE_CLK_IO---GPIO 36(�ⲿ�ж�1)��ΪPS/2��CLK
*@MOUSE_DATA_IO---GPIO 40 ��ΪPS/2��DATA
**************************/
static int init_mouse_io(void)
{
	int mm;
	int dev_id = 1;

#if CONFIG_AM_CHIP_ID == 1213
	// to be done
#else
	gGPIO_Multi5=act_readl(GPIO_MFCTL5);	
	gGPIO_Multi6=act_readl(GPIO_MFCTL6);	
	act_writel((gGPIO_Multi5&(~(7<<28)))|(4<<28),GPIO_MFCTL5); //set IO36 for MouseCLK (IO)
	act_writel((gGPIO_Multi6&(~(3<<0))),GPIO_MFCTL6); //set IO40 for MoouseDAT (IO)

	am_set_gpio(MOUSE_CLK_IO,1);
	am_set_gpio(MOUSE_DATA_IO,1);
	
	//mm = request_irq(IRQ_EXT, (irq_handler_t)mouse_interrupt, 0, "mouse", NULL);
	mm = request_irq(IRQ_EXT, (irq_handler_t)mouse_interrupt, IRQF_SHARED, "mouse", &dev_id);
	if(mm< 0){
		printk(">>>>>request ext_int1 error<<<<<<\n");
		return mm;
	}else
		printk(">>>>>request ext_int1 ok<<<<<<\n");
#endif

	return 0;
}
/**************************
*@�������ݵ����
*@
**************************/
static int  SendDatatoDevice( int data)
{
	int temp=0;
	int t1,i,tt;
	disableIRQ();
	temp=data&0xff;
	t1=1;
	for(i=0;i<8;i++)//У��λ
	{
		tt = temp&BitTab[i];
		if(tt)
		{	t1^=1;	}
	}	
	t1<<=8;
	temp=data+t1;//10λ
	ps2mouse_state.state=Send_cmd;
	ps2mouse_state.send_bit_index=0;
	ps2mouse_state.send_data=temp;
	am_set_gpio(MOUSE_CLK_IO,0);
	udelay(120);
	am_set_gpio(MOUSE_CLK_IO,1);
	enableIRQ();//��CLK �ж�
	udelay(10);
	am_set_gpio(MOUSE_DATA_IO,0);
	return 0;
}
/**************************
*@�����ϳɵ��ĸ��ֽ�
*@
**************************/
static int MouseTimerCheck(int *PacketReg)
{	
	int temph = PacketReg[0];
	if((!(temph&0x08))|| ((temph&0x40)) || ((temph&0x80)))//Allways1,YOverFlow ,XOverFlow
	{													
		return 1;
	}
	return 0;	
}
/**************************
*@���ϲ㷢����Ϣ
*@
**************************/
static void sendmessage(void)
{
	int tdata;
	static int movefreq = 0;
	if(MouseReceiveBuffer[0]&0x10)//x��1=��0=��
	{
		tdata = MouseReceiveBuffer[1]&0xff;
		tdata += 0xffffff00;
	}
	else
	{
		tdata = MouseReceiveBuffer[1]&0xff;
	}
	mousex_data=tdata;

	if(MouseReceiveBuffer[0]&0x20)//y��1=��0=��
	{
		tdata = MouseReceiveBuffer[2]&0xff;
		tdata += 0xffffff00;			
	}
	else
	{
		tdata = MouseReceiveBuffer[2]&0xff;		
	}			
	mousey_data=tdata;
	
	if(MouseReceiveBuffer[3]&0x08)//z��1=��0=��
	{
		tdata = MouseReceiveBuffer[3]&0x07;
		tdata += 0xfffffff8;			
	}
	else
	{
		tdata = MouseReceiveBuffer[3]&0x07;	
	}
	mousez_data=tdata;	


	send_message_state=defaultstate;
	
	if(MouseReceiveBuffer[0]&0x01)// press left
	{
		send_message_state=LeftBtnDown;
		DownLeftFlag=1;
	}
	else
	{
		if(DownLeftFlag==1)
		{
			send_message_state=LeftBtnUp;
			DownLeftFlag=0;
		}
	}
	
	 if(MouseReceiveBuffer[0]&0x02)//press right
	 {
		send_message_state=RightBtnDown;
		DownRightFlag=1;
	 }
	else
	{
		if(DownRightFlag==1)
		{
			send_message_state=RightBtnUp;
			DownRightFlag=0;
		}
	}

	if(MouseReceiveBuffer[0]&0x04)//press middle
	{
		send_message_state=MiddleDown;
		DownMiddleFlag=1;
	}
	else
	{
		if(DownMiddleFlag==1)
		{
			send_message_state=MiddleUp;
			DownMiddleFlag=0;
		}
	}

	 if((MouseReceiveBuffer[3]&0x0f)!=0)
		send_message_state=move_z;
	 
	if(send_message_state==defaultstate)
		send_message_state=move_x_y;


	switch(send_message_state)
	{
		case LeftBtnDown:
			input_report_rel(am_mouse_dev,REL_X, mousex_data);
			input_report_rel(am_mouse_dev,REL_Y,mousey_data);
			input_report_key(am_mouse_dev,BTN_LEFT, 0x01);//0x01����(��Ϊ0�����)
			input_sync(am_mouse_dev);
			break;
		case LeftBtnUp:	
			input_report_rel(am_mouse_dev,REL_X, mousex_data);
			input_report_rel(am_mouse_dev,REL_Y,mousey_data);
			input_report_key(am_mouse_dev,BTN_LEFT, 0x00);//0x00̧��
			input_sync(am_mouse_dev);
			break;	
		case RightBtnDown:
			input_report_rel(am_mouse_dev,REL_X, mousex_data);
			input_report_rel(am_mouse_dev,REL_Y,mousey_data);
			input_report_key(am_mouse_dev,BTN_RIGHT, 0x02);
			input_sync(am_mouse_dev);	
			break;
		case RightBtnUp:
			input_report_rel(am_mouse_dev,REL_X, mousex_data);
			input_report_rel(am_mouse_dev,REL_Y,mousey_data);
			input_report_key(am_mouse_dev,BTN_RIGHT, 0x00);
			input_sync(am_mouse_dev);	
			break;
		case MiddleDown:
			input_report_rel(am_mouse_dev,REL_X, mousex_data);
			input_report_rel(am_mouse_dev,REL_Y,mousey_data);
			input_report_key(am_mouse_dev,BTN_MIDDLE, 0x04);
			input_sync(am_mouse_dev);		
			break;
		case MiddleUp:
			input_report_rel(am_mouse_dev,REL_X, mousex_data);
			input_report_rel(am_mouse_dev,REL_Y,mousey_data);
			input_report_key(am_mouse_dev,BTN_MIDDLE, 0x00);
			input_sync(am_mouse_dev);		
			break;
		case move_z:
			printk("raw move data=%d\n",mousez_data);
			input_report_rel(am_mouse_dev,REL_WHEEL,mousez_data);
			input_sync(am_mouse_dev);
			break;
		case move_x_y:
			xpos_buf[movefreq] = mousex_data;
			ypos_buf[movefreq] = mousey_data;
			movefreq++;
			if(movefreq==MOUSE_MOVE_FREQUENCE)
			{
				int cur_x=0,cur_y=0;
				int i;
				for(i=0;i<MOUSE_MOVE_FREQUENCE;i++)
				{
					cur_x += xpos_buf[i];
					cur_y += ypos_buf[i];
				}
				cur_x = cur_x/MOUSE_MOVE_FREQUENCE;
				cur_y = cur_y/MOUSE_MOVE_FREQUENCE;
				movefreq = 0;
				input_report_rel(am_mouse_dev,REL_X, cur_x);
				input_report_rel(am_mouse_dev,REL_Y,cur_y);
				input_sync(am_mouse_dev);	
			}
			break;	
	}
}
/**************************
*@�����������
*@У��λ
**************************/
static int TestMouseData(int data)//�������
{
	int temp ,t1,i,tt;
	temp = data ;
	if(data < 0x400 || data >= 0x800) return 1;
	if((data%2)==1)	return 1;
	t1=1;
	for(i=0;i < 11;i++)	
	{
		tt = temp&BitTab[i];
		if(tt)
		{	t1^=1;	}
	}
	if(!t1) return 1;
	return 0;
}
/**************************
*@���շ������mouse ������������
*@
**************************/
static int receive_cmd_data(void)
{
	int Bit;
	Bit = am_get_gpio(MOUSE_DATA_IO);;//��IO��
	if(Bit)
	{
		ps2mouse_state.receive_data |= BitTab[ps2mouse_state.receive_bit_index];
		ps2mouse_state.receive_bit_index += 1;
	}
	else
	{
		ps2mouse_state.receive_data &= ~BitTab[ps2mouse_state.receive_bit_index];
		ps2mouse_state.receive_bit_index += 1;
	}
	
	if( ps2mouse_state.receive_bit_index >= 11 )
	{
		int err =0;
		ps2mouse_state.receive_bit_index = 0 ;
		ps2mouse_state.receive_end=1;
		err=TestMouseData(ps2mouse_state.receive_data);
		if(err==0)
		{
			ps2mouse_state.receive_data=(ps2mouse_state.receive_data>>1)&0xff;
		//      printk("d=%x\n",ps2mouse_state.receive_data);
		}
		else
		{
			//printk("dr\n");
		}	
	}
	return 0;
}
/**************************
*@����mouse ��������������������
*@
**************************/
static int receive_normal_data(void)
{
	int Bit;
	int err =0;
	if((ps2mouse_state.receive_bit_index==0)&(ps2mouse_state.receive_byte_index2==0)&(ps2mouse_state.open_timer0_flag==0))//�ϴζ�ʱ���ѵ�
	{
		ps2mouse_state.receive_data_timeout=0;
		ps2mouse_state.open_timer0_flag=1;
		mod_timer(&mouse_timeout,jiffies+receive_maxtime);	//���ĸ��ֽ�����ʱ��
	}
	Bit = am_get_gpio(MOUSE_DATA_IO);//��IO��
	if(Bit)
	{
		ps2mouse_state.receive_data |= BitTab[ps2mouse_state.receive_bit_index];
		ps2mouse_state.receive_bit_index += 1;
	}
	else
	{
		ps2mouse_state.receive_data &= ~BitTab[ps2mouse_state.receive_bit_index];
		ps2mouse_state.receive_bit_index += 1;
	}
	
	if( ps2mouse_state.receive_bit_index >= 11 )
	{	
		ps2mouse_state.receive_bit_index = 0 ;
		ps2mouse_state.receive_byte_index2+=1;//��¼ʵ�ʽ��յ��ֽ���
		err=TestMouseData(ps2mouse_state.receive_data);
		if(err==0)
		{	
			ps2mouse_state.receive_data=(ps2mouse_state.receive_data>>1)&0xff;
 		//	printk("d=%x\n",ps2mouse_state.receive_data);
			if( ps2mouse_state.receive_byte_index<BUFNUMMAX )
			{
				MouseReceiveBuffer[ps2mouse_state.receive_byte_index] = ps2mouse_state.receive_data ;	//������
			}
			ps2mouse_state.receive_byte_index+=1;//��¼��ȷ���ֽ�	
			//�Զ���λ
			if((MouseReceiveBuffer[ps2mouse_state.receive_byte_index-1]==0xaa)&(MouseReceiveBuffer[ps2mouse_state.receive_byte_index]==0x00))
			{
				ps2mouse_state.restart_mouse_flag=1;//��ʱ����λmouse
				ps2mouse_state.receive_data_timeout=1;//�����ͣ���ʱ������������
				ps2mouse_state.each_restart_times=0;
				ps2mouse_state.receive_byte_index=0;
				ps2mouse_state.receive_byte_index2=0;
				memset(MouseReceiveBuffer,0x00,sizeof(int)*BUFNUMMAX);
				ps2mouse_state.state=Other_state;
				printk(">>>restart mouse<<<\n");
			}
		}
		else
		{	
			if(ps2mouse_state.open_timer1_flag==0)
			{
				ps2mouse_state.interrupt_switch=0;	//close receive data
				ps2mouse_state.open_timer1_flag=1;
				mod_timer(&checkerr_timer,jiffies + ERR_CHECK_PERIOD);
			}
			//printk("bit\n");
		}	
	}
	return 0;
}
/**************************
*@mouse clk �ж����
*@��������
*@���շ��������ظ�������
*@����mouse ��������ʱ����������
*@���ϲ㷢����Ϣ
**************************/
static irqreturn_t mouse_interrupt(void)
{
 	check_whick_interrupt();
	if(interrupt_state.ext_irq_source!=interrupt_mouse)
		return IRQ_NONE;
	if(ps2mouse_state.interrupt_switch==0)
		goto errdata;
	switch(ps2mouse_state.state)
	{
		case Send_cmd://send cmd 
		{
			switch(ps2mouse_state.send_bit_index)
			{
				case 0://bit 0
				case 1://bit 1
				case 2://bit 2
				case 3://bit 3
				case 4://bit 4
				case 5://bit 5
				case 6://bit 6
				case 7://bit 7
				case 8://У��λ
					if(ps2mouse_state.send_data&0x1)//write data
					{
						am_set_gpio(MOUSE_DATA_IO,1);
					}
					else
					{
						am_set_gpio(MOUSE_DATA_IO,0);	
					}
					ps2mouse_state.send_bit_index+=1;
					ps2mouse_state.send_data =ps2mouse_state.send_data>>1;
					break;
				case 9:
					{
						am_set_gpio(MOUSE_DATA_IO,1);
						ps2mouse_state.send_bit_index++;
						am_get_gpio(MOUSE_DATA_IO);
						break;//ֹͣλ
					}
				case 10://ACK
					{
						if(0==am_get_gpio(MOUSE_DATA_IO))	
						{
							ps2mouse_state.receive_end=0;
							ps2mouse_state.send_bit_index=0;
							ps2mouse_state.send_data=0;
							ps2mouse_state.receive_bit_index=0;
							ps2mouse_state.receive_byte_index=0;
							ps2mouse_state.receive_byte_index2=0;
							ps2mouse_state.receive_data=0;
							ps2mouse_state.state=Receive_cmd_data;
						}
						else
						{
							ps2mouse_state.restart_mouse_flag=1;//��ʱ����λmouse
							ps2mouse_state.receive_data_timeout=1;
							ps2mouse_state.each_restart_times=1;
							ps2mouse_state.receive_byte_index=0;
							ps2mouse_state.receive_byte_index2=0;
							memset(MouseReceiveBuffer,0x00,sizeof(int)*BUFNUMMAX);
							ps2mouse_state.state=Other_state;
							//printk("send fail\n");
						}
						break;	
					}						
			}
		break;
		}
		case Receive_cmd_data://receive data
		{
		receive_cmd_data();
		if(ps2mouse_state.receive_end==1)//��1byet
		{
			ps2mouse_state.receive_end=0;
			switch(ps2mouse_state.receive_data)
			{
			case 0xaa://�ظ���OK
				{
					if(ps2mouse_state.init_mouse_index == 0)
					{		
						ps2mouse_state.init_mouse_index  = 0x01;
					}
					else
					{	
						if(ps2mouse_state.restart_mouse_flag==0)
						{
							ps2mouse_state.restart_mouse_flag=1;//��ʱ����λmouse
							ps2mouse_state.each_restart_times++;
						}
					}
					break;
				}
			case 0x00://ID				
				{
					if( ps2mouse_state.init_mouse_index==0x01 )
					{
						SendDatatoDevice(0xf3);//Set Sample Rate (first)
						ps2mouse_state.init_mouse_index = 0x2;
					}
					else
					{	
						if(ps2mouse_state.restart_mouse_flag==0)
						{
							ps2mouse_state.restart_mouse_flag=1;//��ʱ����λmouse
							ps2mouse_state.each_restart_times++;
						}
					}
					break;
				}
			case 0x03://3
				{
					if( ps2mouse_state.init_mouse_index==0x100 )//3
					{
						SendDatatoDevice(0xe8);//���÷ֱ���
						ps2mouse_state.init_mouse_index = 0x200;
					}
					else
					{	
						if(ps2mouse_state.restart_mouse_flag==0)
						{
							ps2mouse_state.restart_mouse_flag=1;//��ʱ����λmouse
							ps2mouse_state.each_restart_times++;
						}
					}
					break;							
				}
			case 0xfa://��������ɹ�
				{
					switch(ps2mouse_state.init_mouse_index )
					{
						case 0x0:
						{
							ps2mouse_state.init_mouse_index =0x0;
							break;
						}
						case 0x02:
						{
							SendDatatoDevice(0xc8);//Set Sample Rate 200
							ps2mouse_state.init_mouse_index = 0x04;
							break;
						}
						case 0x04:
						{
							SendDatatoDevice(0xf3);//Set Sample Rate (two)
							ps2mouse_state.init_mouse_index = 0x08;							
							break;
						}
						case 0x08:
						{
							SendDatatoDevice(0x64);//Set Sample Rate 100
							ps2mouse_state.init_mouse_index = 0x10;
							break;
						}
						case 0x10:
						{
							SendDatatoDevice(0xf3);//Set Sample Rate (there)
							ps2mouse_state.init_mouse_index = 0x20;
							break;
						}
						case 0x20:
						{
							SendDatatoDevice(0x50);//Set Sample Rate 80
							ps2mouse_state.init_mouse_index = 0x40;
							break;
						}
						case 0x40:
						{
							SendDatatoDevice(0xf2);//��ѯ����ID
							ps2mouse_state.init_mouse_index = 0x80;
							break;
						}
						case 0x80:
						{
							ps2mouse_state.init_mouse_index = 0x100;
							break;
						}
						case 0x200:
						{
							SendDatatoDevice(0x03);//���÷ֱ���
							ps2mouse_state.init_mouse_index = 0x400;
							break;
						}						
						case 0x400:
						{
							SendDatatoDevice(0xf3);//���ò�������
							ps2mouse_state.init_mouse_index = 0x800;							
							break;
						}		
						case 0x800:
						{
							//	SendDatatoDevice(40);//���ò����ʵ�ֵ ������Ϳ���
							SendDatatoDevice(20);
							ps2mouse_state.init_mouse_index = 0x1000;
							break;
						}
						case 0x1000:
						{
							SendDatatoDevice(0xf4);//ʹ��Streamģʽ�µ����ݰ����͡�
							ps2mouse_state.init_mouse_index = 0x2000;
							break;
						}
						case 0x2000:
						{
							ps2mouse_state.send_bit_index=0;
							ps2mouse_state.send_data=0;
							ps2mouse_state.receive_bit_index=0;
							ps2mouse_state.receive_byte_index=0;
							ps2mouse_state.receive_byte_index2=0;
							ps2mouse_state.receive_data=0;
							ps2mouse_state.init_mouse_index =0xfff;
							ps2mouse_state.state=Receive_normal_data;
							printk(">>>init mouse ok<<<\n");
							break;
						}
						default:
						{	
							if(ps2mouse_state.restart_mouse_flag==0)
							{
								ps2mouse_state.restart_mouse_flag=1;//��ʱ����λmouse
								ps2mouse_state.each_restart_times++;
							}
						}
					}
				 break;
			  }// switch(0xfa)
			 default:
			 {
			 	if(ps2mouse_state.restart_mouse_flag==0)
			 	{
					ps2mouse_state.restart_mouse_flag=1;//��ʱ����λmouse
					ps2mouse_state.each_restart_times++;
			 	}
			 }
		    }
		}
		 break;		
		}
		case Receive_normal_data://normal receive
		{
			receive_normal_data();//�����ݣ����ŵ�ps2mouse_state.receive_data��	
			if( ps2mouse_state.receive_byte_index2>=4 )//���ĸ�byte
			{
				if(ps2mouse_state.receive_data_timeout==0)
				{
					if(ps2mouse_state.receive_byte_index2==ps2mouse_state.receive_byte_index)//�м��޴�byte
					{
						int erro; 
						erro = MouseTimerCheck((int *)MouseReceiveBuffer);//����ĸ��ֽ�
						if(erro)//������
						{
							if(ps2mouse_state.open_timer1_flag==0)
							{
								ps2mouse_state.interrupt_switch=0;	
								ps2mouse_state.open_timer1_flag=1;
								mod_timer(&checkerr_timer,jiffies + ERR_CHECK_PERIOD);
							}
							//printk("byte\n");
						}
						else
						{
							sendmessage();	//send message
							//printk("sm\n");
						}	
					}
				}
				ps2mouse_state.receive_data_timeout=0xff;//��ʱ��������в�����
				ps2mouse_state.send_bit_index=0;
				ps2mouse_state.send_data=0;
				ps2mouse_state.receive_bit_index=0;
				ps2mouse_state.receive_byte_index=0;
				ps2mouse_state.receive_byte_index2=0;
				ps2mouse_state.receive_data=0;
				ps2mouse_state.state=Receive_normal_data;
				memset(MouseReceiveBuffer,0x00,sizeof(int)*BUFNUMMAX);
			}
		break;
		}
		case Other_state:
		{
			break;
		}
	}
errdata:	
	mouse_clearPending();
	return IRQ_HANDLED;
}
/**************************
*@��λmouse 
*@��⿪���ǳ�ʼ����mouse 
*@����Ƿ�һֱ����send_cmd ״̬
**************************/
static void timer_restart_mouse(void)
{
	static int times=0;
	static int send_cmd_index1=0;
	static int send_cmd_index2=0;
	static int receive_cmd_index1=0;
	static int receive_cmd_index2=0;
	static int other_state_index1=0;
	static int other_state_index2=0;
	static int interrupt_state_index1=0;
	static int interrupt_state_index2=0;
	static int interrupt_switch_index1=0;
	static int interrupt_switch_index2=0;

	times++;
	send_cmd_index1++;
	receive_cmd_index1++;
	other_state_index1++;
	interrupt_state_index1++;
	interrupt_switch_index1++;
	#if 1
	if(ps2mouse_state.first_init_mouse==1)
	{
		printk(">>>first restart mouse<<<\n");
		mouserest();
		ps2mouse_state.first_init_mouse=2;	
	}
	#endif
	#if 1
	if(1==ps2mouse_state.restart_mouse_flag)//��ʱ����λmouse
	{
		memset(MouseReceiveBuffer,0x00,sizeof(int)*BUFNUMMAX);
		ps2mouse_state.restart_mouse_flag=0;	
		if(ps2mouse_state.each_restart_times<resetmouse_maxtime)
		{
			mouserest();
		}
	}
	#endif
	#if 1
	//��⿪���ǳ�ʼ����mouse 
	//��������Ƿ���mouse(3s) ֻ����һ��	
	if(times==30)
	{
		if(ps2mouse_state.first_init_mouse==init_mouse_times)
		{
			times++;	
			ps2mouse_state.state=Receive_normal_data;		
			ps2mouse_state.send_bit_index=0;
			ps2mouse_state.send_data=0;
			ps2mouse_state.receive_bit_index=0;
			ps2mouse_state.receive_byte_index=0;
			ps2mouse_state.receive_byte_index2=0;
			ps2mouse_state.receive_data=0;
			ps2mouse_state.init_mouse_index =0;
			ps2mouse_state.receive_end=0;
			ps2mouse_state.each_restart_times=0;			
			ps2mouse_state.restart_mouse_flag=0;
			ps2mouse_state.first_init_mouse=init_mouse_times+1;
			printk(">>>no mosue<<<\n");			
		}
		if((ps2mouse_state.first_init_mouse<init_mouse_times)&(ps2mouse_state.init_mouse_index !=0xfff))
		{
			times=0;	
			ps2mouse_state.each_restart_times=0;
			ps2mouse_state.restart_mouse_flag=1;
			ps2mouse_state.first_init_mouse++;
			//printk(">>>Reset mouse times=%d <<<\n",ps2mouse_state.first_init_mouse);
		}
		if((ps2mouse_state.first_init_mouse<init_mouse_times)&(ps2mouse_state.init_mouse_index ==0xfff))
		{
			ps2mouse_state.first_init_mouse=init_mouse_times+1;
		}
	}
	#endif 
	//����һֱ��Send_cmd״̬����
	if((ps2mouse_state.state==Send_cmd)&(ps2mouse_state.first_init_mouse==end_init_mouse))
	{
		send_cmd_index2++;
		if(send_cmd_index2==1)//ͬ��
		{
			send_cmd_index1=1;
		}
		if(send_cmd_index1>=con_send_times)
		{
			if((send_cmd_index1==con_send_times)&(send_cmd_index2==con_send_times))//����N��״̬��Send_cmd
			{
				ps2mouse_state.state=Receive_normal_data;		
				ps2mouse_state.send_bit_index=0;
				ps2mouse_state.send_data=0;
				ps2mouse_state.receive_bit_index=0;
				ps2mouse_state.receive_byte_index=0;
				ps2mouse_state.receive_byte_index2=0;
				ps2mouse_state.receive_data=0;
				ps2mouse_state.init_mouse_index =0;
				ps2mouse_state.receive_end=0;
				ps2mouse_state.each_restart_times=0;
				ps2mouse_state.restart_mouse_flag=1;
				printk(">>>out send_cmd state<<<\n");
			}
			send_cmd_index2=0;
		}
	}
	#if 1
	if((ps2mouse_state.state==Receive_cmd_data)&(ps2mouse_state.first_init_mouse==end_init_mouse))
	{	
		receive_cmd_index2++;
		if(receive_cmd_index2==1)//ͬ��
		{
			receive_cmd_index1=1;
		}
		if(receive_cmd_index1>=con_cmd_times)
		{
			if((receive_cmd_index1==con_cmd_times)&(receive_cmd_index2==con_cmd_times))//����N��״̬��Receive_cmd_data
			{
				ps2mouse_state.state=Receive_normal_data;		
				ps2mouse_state.send_bit_index=0;
				ps2mouse_state.send_data=0;
				ps2mouse_state.receive_bit_index=0;
				ps2mouse_state.receive_byte_index=0;
				ps2mouse_state.receive_byte_index2=0;
				ps2mouse_state.receive_data=0;
				ps2mouse_state.init_mouse_index =0;
				ps2mouse_state.receive_end=0;
				ps2mouse_state.each_restart_times=0;
				ps2mouse_state.restart_mouse_flag=1;
				printk(">>>out receive_cmd_data state<<<\n");
			}
			receive_cmd_index2=0;
		}
	}	
	#endif
	#if 1
	if((ps2mouse_state.state==Other_state)&(ps2mouse_state.first_init_mouse==end_init_mouse))
	{	
		other_state_index2++;
		if(other_state_index2==1)//ͬ��
		{
			other_state_index1=1;
		}
		if(other_state_index1>=con_other_times)
		{
			if((other_state_index1==con_other_times)&(other_state_index2==con_other_times))//����N��״̬��other state
			{
				ps2mouse_state.state=Receive_normal_data;		
				ps2mouse_state.send_bit_index=0;
				ps2mouse_state.send_data=0;
				ps2mouse_state.receive_bit_index=0;
				ps2mouse_state.receive_byte_index=0;
				ps2mouse_state.receive_byte_index2=0;
				ps2mouse_state.receive_data=0;
				ps2mouse_state.init_mouse_index =0;
				ps2mouse_state.receive_end=0;
				ps2mouse_state.each_restart_times=0;
				ps2mouse_state.restart_mouse_flag=1;
				printk(">>>out other_state<<<\n");
			}
			other_state_index2=0;
		}
	}	
	#endif
	#if 1
	if(ps2mouse_state.interrupt_state_flag==0)//interrupt close
	{
		interrupt_state_index2++;
		if(interrupt_state_index2==1)
			interrupt_state_index1=1;
		if(interrupt_state_index1>=con_clinterrupt_times)
		{
			if((interrupt_state_index1==con_clinterrupt_times)&(interrupt_state_index2==con_clinterrupt_times))
			{
				 enableIRQ();//open interrupt
				 ps2mouse_state.each_restart_times=0;
				 ps2mouse_state.restart_mouse_flag=1;
				 printk(">>>open interrupt<<<\n");
			}
			interrupt_state_index2=0;
		}
	}
	#endif
	#if 1
	if( ps2mouse_state.interrupt_switch==0)
	{
		interrupt_switch_index2++;
		if(interrupt_switch_index2==1)
			interrupt_switch_index1=1;
		if(interrupt_switch_index1>=con_interrupt_switch)
		{
			if((interrupt_switch_index1==con_interrupt_switch)&(interrupt_switch_index2==con_interrupt_switch))
			{
				ps2mouse_state.interrupt_switch=1;
			}
			interrupt_switch_index2=0;
		}
	}
	#endif
	mod_timer(&mouse_timer,jiffies +25);//jiffies=4ms
}
/**************************
*@
*@
**************************/
static int init_mouse_timer(void)
{
	init_timer(&mouse_timer);			
	mouse_timer.function = (void*)timer_restart_mouse;
	mouse_timer.expires = jiffies +200;//jiffies=4ms
	add_timer(&mouse_timer);	
	return 0;
}
/**************************
*@�������ݴ���ʱ��������
*@
**************************/
static void errckeck_timer_isr(void)
{
	ps2mouse_state.interrupt_switch=1;//open switch
	ps2mouse_state.open_timer1_flag=0;//timer enter flag
	ps2mouse_state.send_bit_index=0;
	ps2mouse_state.send_data=0;
	ps2mouse_state.receive_bit_index=0;
	ps2mouse_state.receive_byte_index=0;
	ps2mouse_state.receive_byte_index2=0;
	ps2mouse_state.receive_data=0;
	ps2mouse_state.state=Receive_normal_data;
	memset(MouseReceiveBuffer,0x00,sizeof(int)*BUFNUMMAX);
//	printk("errckeck_time_isr\n");
}
/**************************
*@��ʼ���������ݴ���ʱ��
*@
**************************/
static int init_errcheck_timer(void)
{
	init_timer(&checkerr_timer);			
	checkerr_timer.function = (void*)errckeck_timer_isr;
	checkerr_timer.expires = jiffies +0xffff;
	add_timer(&checkerr_timer);	
	return 0;
}
/**************************
*@�������ݳ�ʱ������
*@
**************************/
static void check_timeout(void)
{
	ps2mouse_state.open_timer0_flag=0;
	if((ps2mouse_state.receive_data_timeout==0)&(ps2mouse_state.interrupt_switch==1))//��ʱ
	{
		ps2mouse_state.receive_data_timeout=1;//��־��ʱ
		ps2mouse_state.receive_bit_index=0;
		ps2mouse_state.receive_byte_index=0;
		ps2mouse_state.receive_byte_index2=0;
		ps2mouse_state.receive_data=0;
		memset(MouseReceiveBuffer,0x00,sizeof(int)*BUFNUMMAX);	
//		printk("check_timeout\n");
	}
	if(ps2mouse_state.receive_data_timeout==0xff)//����ʱ
	{
		ps2mouse_state.receive_data_timeout=0xaa;
//		printk("normal receive\n");
	}	
}
/**************************
*@��ʼ�����ճ�ʱ��ʱ��
*@
**************************/
static int init_timeout_timer(void)
{
	init_timer(&mouse_timeout);			
	mouse_timeout.function = (void*)check_timeout;
	mouse_timeout.expires = jiffies +3*HZ;
	add_timer(&mouse_timeout);	
	return 0;
}
/**************************
*@��λmouse 
*@
**************************/
static void mouserest(void)
{
#if CONFIG_AM_CHIP_ID == 1213
	// to be done
#else
	//IO�����³�ʼ��
	act_writel((gGPIO_Multi5&(~7<<28))|(4<<28),GPIO_MFCTL5); //set IO36 for MouseCLK (IO)
	act_writel((gGPIO_Multi6&(~3<<0)),GPIO_MFCTL6); //set IO40 for MoouseDAT (IO)
#endif
	am_set_gpio(MOUSE_CLK_IO,1);
	am_set_gpio(MOUSE_DATA_IO,1);
	ps2mouse_state.state=Send_cmd;
	ps2mouse_state.send_bit_index=0;
	ps2mouse_state.send_data=0;
	ps2mouse_state.receive_bit_index=0;
	ps2mouse_state.receive_byte_index=0;
	ps2mouse_state.receive_byte_index2=0;
	ps2mouse_state.receive_data=0;
	ps2mouse_state.init_mouse_index =0;
	ps2mouse_state.receive_end=0;
	ps2mouse_state.restart_mouse_flag=0;
	memset(MouseReceiveBuffer,0x00,sizeof(int)*BUFNUMMAX);
	SendDatatoDevice(0xff);//��λ(�ظ�0xfa 0xaa 0x00)
}
/**************************
*@ģ�����
*@��ʼ��PS/2��IO��
*@��λmouse 
**************************/
static int __init mouse_init(void)
{
	int error;

	printk(KERN_ERR "*********am_mouse_init************\n");
	am_mouse_dev = input_allocate_device();
	if (!am_mouse_dev)
	{
		printk(KERN_ERR "input_allocate_device() failed for mouse 1\n");
		return -ENOMEM;
	}
	am_mouse_dev->name = "AM mouse";
	am_mouse_dev->phys = "AM/input0";
	
	am_mouse_dev->id.bustype = BUS_HOST;
	am_mouse_dev->id.vendor = 0x0001;
	am_mouse_dev->id.product = 0x0001;
	am_mouse_dev->id.version = 0x0100;

	am_mouse_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REL);
	am_mouse_dev->relbit[0] = BIT_MASK(REL_X) | BIT_MASK(REL_Y) | BIT_MASK(REL_WHEEL) ;
	am_mouse_dev->keybit[BIT_WORD(BTN_LEFT)] = BIT_MASK(BTN_LEFT) |
		BIT_MASK(BTN_MIDDLE) | BIT_MASK(BTN_RIGHT);
	error = input_register_device(am_mouse_dev);
	if (error) {
		printk(KERN_ERR "input_allocate_device() failed for mouse 2\n");
		input_free_device(am_mouse_dev);
		return error;
	}
	init_mouse_io();
	ps2mouse_state.first_init_mouse=1;	    //�״γ�ʼ��mouse	
	ps2mouse_state.each_restart_times=0;//��¼��λ����
	ps2mouse_state.receive_data_timeout=0xff;//��¼�������ݳ�ʱ
	ps2mouse_state.interrupt_state_flag=0; //interrupt state flag is close
	ps2mouse_state.interrupt_switch=1;	//receive data 
	ps2mouse_state.open_timer0_flag=0;
	ps2mouse_state.open_timer1_flag=0;
	ps2mouse_state.state=Other_state;
	enableIRQ();
	init_mouse_timer();
	init_errcheck_timer();
	init_timeout_timer();
	return 0;
}
/**************************
*@
*@
**************************/
static void __exit mouse_exit(void)
{
	del_timer(&mouse_timer);
	del_timer(&checkerr_timer);
	del_timer(&mouse_timeout);
	free_irq(IRQ_EXT, mouse_interrupt);
	input_unregister_device(am_mouse_dev);	
}
module_init(mouse_init);
module_exit(mouse_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("luozhenghai");
MODULE_DESCRIPTION("ps/2 mouse driver for Actions-Micro");
